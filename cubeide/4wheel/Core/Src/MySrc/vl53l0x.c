#include "MySrc/vl53l0x.h"

#include "i2c.h"            /* hi2c2 from CubeMX */
#include "MySrc/platform_adapter.h"   /* Platform_Millis() */

/*
 * Compact register-level VL53L0X driver, ported from the Pololu/ST reduced API.
 *
 * Only the pieces needed to boot the sensor and run continuous back-to-back
 * ranging on hi2c2 are kept. The tuning-register block and the timeout/macro
 * period math are reproduced verbatim from ST's reduced API so that range
 * accuracy matches the reference driver.
 *
 * I2C: 7-bit address 0x29 -> HAL 8-bit 0x52. All register access goes through
 * HAL_I2C_Mem_Read/Write on hi2c2.
 *
 * XSHUT: for a single sensor left at the default address, XSHUT is assumed tied
 * high in hardware. If a second device sharing the bus is ever added, drive an
 * XSHUT GPIO low/high here before Init() to assign a fresh address.
 */

#define VL53L0X_I2C_ADDR_8BIT   (0x29U << 1)   /* 0x52 */
#define VL53L0X_I2C_TIMEOUT_MS  10U

/* --- Register map (subset) ------------------------------------------------ */
#define SYSRANGE_START                              0x00
#define SYSTEM_SEQUENCE_CONFIG                       0x01
#define SYSTEM_INTERMEASUREMENT_PERIOD               0x04
#define SYSTEM_INTERRUPT_CONFIG_GPIO                 0x0A
#define SYSTEM_INTERRUPT_CLEAR                        0x0B
#define RESULT_INTERRUPT_STATUS                       0x13
#define RESULT_RANGE_STATUS                           0x14
#define MSRC_CONFIG_CONTROL                           0x60
#define FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT   0x44
#define SYSTEM_SEQUENCE_CONFIG_REG                    0x01
#define DYNAMIC_SPAD_REF_EN_START_OFFSET             0x4F
#define DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD          0x4E
#define GLOBAL_CONFIG_REF_EN_START_SELECT            0xB6
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_0             0xB0
#define MSRC_CONFIG_TIMEOUT_MACROP                    0x46
#define FINAL_RANGE_CONFIG_VALID_PHASE_LOW           0x47
#define FINAL_RANGE_CONFIG_VALID_PHASE_HIGH          0x48
#define PRE_RANGE_CONFIG_VCSEL_PERIOD                 0x50
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI           0x51
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO           0x52
#define PRE_RANGE_CONFIG_VALID_PHASE_LOW             0x56
#define PRE_RANGE_CONFIG_VALID_PHASE_HIGH            0x57
#define PRE_RANGE_CONFIG_MIN_SNR                      0x27
#define FINAL_RANGE_CONFIG_VCSEL_PERIOD              0x70
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI         0x71
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO         0x72
#define GLOBAL_CONFIG_VCSEL_WIDTH                     0x32
#define IDENTIFICATION_MODEL_ID                       0xC0
#define VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV            0x89
#define ALGO_PHASECAL_LIM                             0x30
#define ALGO_PHASECAL_CONFIG_TIMEOUT                  0x30

/* VCSEL period type for sequence-step timeout math */
typedef enum
{
    VcselPeriodPreRange,
    VcselPeriodFinalRange
} VcselPeriodType;

/* Sequence-step enable flags decoded from SYSTEM_SEQUENCE_CONFIG */
typedef struct
{
    bool tcc;
    bool msrc;
    bool dss;
    bool pre_range;
    bool final_range;
} SequenceStepEnables;

typedef struct
{
    uint16_t pre_range_vcsel_period_pclks;
    uint16_t final_range_vcsel_period_pclks;
    uint16_t msrc_dss_tcc_mclks;
    uint16_t pre_range_mclks;
    uint16_t final_range_mclks;
    uint32_t msrc_dss_tcc_us;
    uint32_t pre_range_us;
    uint32_t final_range_us;
} SequenceStepTimeouts;

/* ------------------------------------------------------------------ */
/*  Low-level I2C helpers                                              */
/* ------------------------------------------------------------------ */
static bool writeReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, &value, 1U,
                             VL53L0X_I2C_TIMEOUT_MS) == HAL_OK;
}

static bool writeReg16Bit(uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(value >> 8);
    buf[1] = (uint8_t)(value & 0xFF);
    return HAL_I2C_Mem_Write(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, buf, 2U,
                             VL53L0X_I2C_TIMEOUT_MS) == HAL_OK;
}

static uint8_t readReg(uint8_t reg)
{
    uint8_t value = 0U;
    (void)HAL_I2C_Mem_Read(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                           I2C_MEMADD_SIZE_8BIT, &value, 1U,
                           VL53L0X_I2C_TIMEOUT_MS);
    return value;
}

static uint16_t readReg16Bit(uint8_t reg)
{
    uint8_t buf[2] = {0U, 0U};
    (void)HAL_I2C_Mem_Read(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                           I2C_MEMADD_SIZE_8BIT, buf, 2U,
                           VL53L0X_I2C_TIMEOUT_MS);
    return (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
}

static bool writeMulti(uint8_t reg, const uint8_t *src, uint16_t count)
{
    return HAL_I2C_Mem_Write(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)src, count,
                             VL53L0X_I2C_TIMEOUT_MS) == HAL_OK;
}

static bool readMulti(uint8_t reg, uint8_t *dst, uint16_t count)
{
    return HAL_I2C_Mem_Read(&hi2c2, VL53L0X_I2C_ADDR_8BIT, reg,
                            I2C_MEMADD_SIZE_8BIT, dst, count,
                            VL53L0X_I2C_TIMEOUT_MS) == HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  Timeout / macro-period math (verbatim from ST reduced API)        */
/* ------------------------------------------------------------------ */
#define decodeVcselPeriod(reg_val)   (((reg_val) + 1) << 1)
#define encodeVcselPeriod(period)    (((period) >> 1) - 1)
#define calcMacroPeriod(vcsel_pclks) ((((uint32_t)2304 * (vcsel_pclks) * 1655) + 500) / 1000)

/* Decode sequence-step timeout from register value to MCLKs */
static uint16_t decodeTimeout(uint16_t reg_val)
{
    /* format: (LSByte * 2^MSByte) + 1 */
    return (uint16_t)(((reg_val & 0x00FF) << (uint16_t)((reg_val & 0xFF00) >> 8)) + 1);
}

/* Encode sequence-step timeout from MCLKs to register value */
static uint16_t encodeTimeout(uint32_t timeout_mclks)
{
    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;

    if (timeout_mclks > 0)
    {
        ls_byte = timeout_mclks - 1;
        while ((ls_byte & 0xFFFFFF00U) > 0)
        {
            ls_byte >>= 1;
            ms_byte++;
        }
        return (uint16_t)((ms_byte << 8) | (ls_byte & 0xFF));
    }
    return 0;
}

static uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks,
                                           uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return (((uint32_t)timeout_period_mclks * macro_period_ns) + 500) / 1000;
}

static uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us,
                                           uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

/* ------------------------------------------------------------------ */
/*  Sequence-step helpers                                             */
/* ------------------------------------------------------------------ */
static uint8_t getVcselPulsePeriod(VcselPeriodType type)
{
    if (type == VcselPeriodPreRange)
    {
        return (uint8_t)decodeVcselPeriod(readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD));
    }
    else if (type == VcselPeriodFinalRange)
    {
        return (uint8_t)decodeVcselPeriod(readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD));
    }
    return 255U;
}

static void getSequenceStepEnables(SequenceStepEnables *enables)
{
    uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG);
    enables->tcc         = (sequence_config >> 4) & 0x1;
    enables->dss         = (sequence_config >> 3) & 0x1;
    enables->msrc        = (sequence_config >> 2) & 0x1;
    enables->pre_range   = (sequence_config >> 6) & 0x1;
    enables->final_range = (sequence_config >> 7) & 0x1;
}

static void getSequenceStepTimeouts(const SequenceStepEnables *enables,
                                    SequenceStepTimeouts *timeouts)
{
    timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

    timeouts->msrc_dss_tcc_mclks = readReg(MSRC_CONFIG_TIMEOUT_MACROP) + 1;
    timeouts->msrc_dss_tcc_us =
        timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    timeouts->pre_range_mclks =
        decodeTimeout(readReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    timeouts->pre_range_us =
        timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

    timeouts->final_range_mclks =
        decodeTimeout(readReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

    if (enables->pre_range)
    {
        timeouts->final_range_mclks -= timeouts->pre_range_mclks;
    }

    timeouts->final_range_us =
        timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                                   timeouts->final_range_vcsel_period_pclks);
}

/* ------------------------------------------------------------------ */
/*  Measurement timing budget                                         */
/* ------------------------------------------------------------------ */
static const uint16_t StartOverhead    = 1910;
static const uint16_t EndOverhead       = 960;
static const uint16_t MsrcOverhead      = 660;
static const uint16_t TccOverhead       = 590;
static const uint16_t DssOverhead       = 690;
static const uint16_t PreRangeOverhead  = 660;
static const uint16_t FinalRangeOverhead = 550;

static uint32_t getMeasurementTimingBudget(void)
{
    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;
    uint32_t budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc)  { budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead); }
    if (enables.dss)  { budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead); }
    else if (enables.msrc) { budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead); }
    if (enables.pre_range) { budget_us += (timeouts.pre_range_us + PreRangeOverhead); }
    if (enables.final_range) { budget_us += (timeouts.final_range_us + FinalRangeOverhead); }

    return budget_us;
}

static bool setMeasurementTimingBudget(Vl53l0x *self, uint32_t budget_us)
{
    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;
    uint32_t used_budget_us;
    uint32_t final_range_timeout_us;
    const uint32_t MinTimingBudget = 20000;

    if (budget_us < MinTimingBudget) { return false; }

    used_budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc)  { used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead); }
    if (enables.dss)  { used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead); }
    else if (enables.msrc) { used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead); }
    if (enables.pre_range) { used_budget_us += (timeouts.pre_range_us + PreRangeOverhead); }

    if (enables.final_range)
    {
        used_budget_us += FinalRangeOverhead;

        /* The final range timeout takes whatever budget remains. */
        if (used_budget_us > budget_us) { return false; }

        final_range_timeout_us = budget_us - used_budget_us;

        uint32_t final_range_timeout_mclks =
            timeoutMicrosecondsToMclks(final_range_timeout_us,
                                       timeouts.final_range_vcsel_period_pclks);

        if (enables.pre_range)
        {
            final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                      encodeTimeout(final_range_timeout_mclks));

        self->measurement_timing_budget_us = budget_us;
    }

    return true;
}

static bool setSignalRateLimit(float limit_Mcps)
{
    if (limit_Mcps < 0 || limit_Mcps > 511.99f) { return false; }
    /* Q9.7 fixed point format (9 integer bits, 7 fractional bits) */
    writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                  (uint16_t)(limit_Mcps * (1 << 7)));
    return true;
}

/* ------------------------------------------------------------------ */
/*  SPAD info                                                         */
/* ------------------------------------------------------------------ */
static bool getSpadInfo(uint8_t *count, bool *type_is_aperture)
{
    uint8_t tmp;

    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);

    writeReg(0xFF, 0x06);
    writeReg(0x83, readReg(0x83) | 0x04);
    writeReg(0xFF, 0x07);
    writeReg(0x81, 0x01);

    writeReg(0x80, 0x01);

    writeReg(0x94, 0x6b);
    writeReg(0x83, 0x00);

    {
        uint32_t start = Platform_Millis();
        while (readReg(0x83) == 0x00)
        {
            if ((Platform_Millis() - start) > 100U) { return false; }
        }
    }

    writeReg(0x83, 0x01);
    tmp = readReg(0x92);

    *count = tmp & 0x7f;
    *type_is_aperture = (tmp >> 7) & 0x01;

    writeReg(0x81, 0x00);
    writeReg(0xFF, 0x06);
    writeReg(0x83, readReg(0x83) & ~0x04);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x01);

    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    return true;
}

/* ------------------------------------------------------------------ */
/*  Reference calibration                                            */
/* ------------------------------------------------------------------ */
static bool performSingleRefCalibration(uint8_t vhv_init_byte)
{
    uint32_t start;

    writeReg(SYSRANGE_START, 0x01 | vhv_init_byte);

    start = Platform_Millis();
    while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        if ((Platform_Millis() - start) > 100U) { return false; }
    }

    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    writeReg(SYSRANGE_START, 0x00);

    return true;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                       */
/* ------------------------------------------------------------------ */
bool Vl53l0x_Init(Vl53l0x *self)
{
    uint8_t spad_count = 0U;
    bool spad_type_is_aperture = false;
    uint8_t ref_spad_map[6];
    uint8_t first_spad_to_enable;
    uint8_t spads_enabled;
    int i;

    if (self == 0) { return false; }

    self->address = VL53L0X_I2C_ADDR_8BIT;
    self->stop_variable = 0U;
    self->last_read = 0U;
    self->last_distance_mm = 0U;
    self->initialized = false;
    self->valid = false;

    /* Check the sensor is alive and is actually a VL53L0X. */
    if (readReg(IDENTIFICATION_MODEL_ID) != 0xEE)
    {
        return false;
    }

    /* --- DataInit: set I2C to 2.8V mode --- */
    writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
             readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01);

    /* Set I2C standard mode */
    writeReg(0x88, 0x00);

    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    self->stop_variable = readReg(0x91);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    /* Disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks */
    writeReg(MSRC_CONFIG_CONTROL, readReg(MSRC_CONFIG_CONTROL) | 0x12);

    /* Set final range signal rate limit to 0.25 MCPS (million counts/sec) */
    setSignalRateLimit(0.25f);

    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);

    /* --- StaticInit: get SPAD info --- */
    if (!getSpadInfo(&spad_count, &spad_type_is_aperture))
    {
        return false;
    }

    readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    writeReg(0xFF, 0x01);
    writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    writeReg(0xFF, 0x00);
    writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    first_spad_to_enable = spad_type_is_aperture ? 12 : 0;
    spads_enabled = 0;

    for (i = 0; i < 48; i++)
    {
        if (i < first_spad_to_enable || spads_enabled == spad_count)
        {
            ref_spad_map[i / 8] &= ~(1 << (i % 8));
        }
        else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
        {
            spads_enabled++;
        }
    }

    writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    /* --- Default tuning settings (verbatim from ST reduced API) --- */
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0xFF, 0x00);
    writeReg(0x09, 0x00);
    writeReg(0x10, 0x00);
    writeReg(0x11, 0x00);
    writeReg(0x24, 0x01);
    writeReg(0x25, 0xFF);
    writeReg(0x75, 0x00);
    writeReg(0xFF, 0x01);
    writeReg(0x4E, 0x2C);
    writeReg(0x48, 0x00);
    writeReg(0x30, 0x20);
    writeReg(0xFF, 0x00);
    writeReg(0x30, 0x09);
    writeReg(0x54, 0x00);
    writeReg(0x31, 0x04);
    writeReg(0x32, 0x03);
    writeReg(0x40, 0x83);
    writeReg(0x46, 0x25);
    writeReg(0x60, 0x00);
    writeReg(0x27, 0x00);
    writeReg(0x50, 0x06);
    writeReg(0x51, 0x00);
    writeReg(0x52, 0x96);
    writeReg(0x56, 0x08);
    writeReg(0x57, 0x30);
    writeReg(0x61, 0x00);
    writeReg(0x62, 0x00);
    writeReg(0x64, 0x00);
    writeReg(0x65, 0x00);
    writeReg(0x66, 0xA0);
    writeReg(0xFF, 0x01);
    writeReg(0x22, 0x32);
    writeReg(0x47, 0x14);
    writeReg(0x49, 0xFF);
    writeReg(0x4A, 0x00);
    writeReg(0xFF, 0x00);
    writeReg(0x7A, 0x0A);
    writeReg(0x7B, 0x00);
    writeReg(0x78, 0x21);
    writeReg(0xFF, 0x01);
    writeReg(0x23, 0x34);
    writeReg(0x42, 0x00);
    writeReg(0x44, 0xFF);
    writeReg(0x45, 0x26);
    writeReg(0x46, 0x05);
    writeReg(0x40, 0x40);
    writeReg(0x0E, 0x06);
    writeReg(0x20, 0x1A);
    writeReg(0x43, 0x40);
    writeReg(0xFF, 0x00);
    writeReg(0x34, 0x03);
    writeReg(0x35, 0x44);
    writeReg(0xFF, 0x01);
    writeReg(0x31, 0x04);
    writeReg(0x4B, 0x09);
    writeReg(0x4C, 0x05);
    writeReg(0x4D, 0x04);
    writeReg(0xFF, 0x00);
    writeReg(0x44, 0x00);
    writeReg(0x45, 0x20);
    writeReg(0x47, 0x08);
    writeReg(0x48, 0x28);
    writeReg(0x67, 0x00);
    writeReg(0x70, 0x04);
    writeReg(0x71, 0x01);
    writeReg(0x72, 0xFE);
    writeReg(0x76, 0x00);
    writeReg(0x77, 0x00);
    writeReg(0xFF, 0x01);
    writeReg(0x0D, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x01);
    writeReg(0x01, 0xF8);
    writeReg(0xFF, 0x01);
    writeReg(0x8E, 0x01);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    /* --- Configure interrupt: new-sample-ready, active low --- */
    writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    writeReg(0x84, readReg(0x84) & ~0x10);   /* GPIO active low */
    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

    /* Read and store the current timing budget */
    self->measurement_timing_budget_us = getMeasurementTimingBudget();

    /* Disable MSRC and TCC by default (ST reduced API default) */
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);

    /* Recalculate timing budget after the sequence config change */
    setMeasurementTimingBudget(self, self->measurement_timing_budget_us);

    /* --- VHV / phase reference calibration --- */
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!performSingleRefCalibration(0x40)) { return false; }

    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!performSingleRefCalibration(0x00)) { return false; }

    /* Restore the full sequence config */
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);

    /* --- Start continuous back-to-back ranging --- */
    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, self->stop_variable);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    writeReg(SYSRANGE_START, 0x02);   /* continuous, back-to-back */

    self->initialized = true;
    self->last_read = Platform_Millis();
    return true;
}

void Vl53l0x_Update(Vl53l0x *self)
{
    uint16_t range_mm;

    if (self == 0) { return; }
    if (!self->initialized) { return; }

    /* Non-blocking: bail out until the sensor flags a fresh sample. */
    if ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        return;
    }

    /* Range is at RESULT_RANGE_STATUS + 10 (0x14 + 10). */
    range_mm = readReg16Bit(RESULT_RANGE_STATUS + 10);

    /* Clear the interrupt so the next sample can be flagged. */
    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

    self->last_read = Platform_Millis();

    /* 8190/8191 are the sensor's "no target / out of range" sentinels. */
    if (range_mm >= 8190U || range_mm == 0U)
    {
        self->valid = false;
    }
    else
    {
        self->last_distance_mm = range_mm;
        self->valid = true;
    }
}

double Vl53l0x_GetDistanceCm(const Vl53l0x *self)
{
    if (self == 0) { return 0.0; }
    return (double)self->last_distance_mm / 10.0;
}

uint16_t Vl53l0x_GetDistanceMm(const Vl53l0x *self)
{
    if (self == 0) { return 0U; }
    return self->last_distance_mm;
}

bool Vl53l0x_IsValid(const Vl53l0x *self)
{
    if (self == 0) { return false; }
    return self->valid;
}
