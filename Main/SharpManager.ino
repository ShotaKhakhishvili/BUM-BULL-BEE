#include "SharpManager.hpp"
#include "Defines.hpp"

#include <Arduino.h>
#include <SharpIR.h>
#include <math.h>

#ifndef MODEL_SHORT
#define MODEL_SHORT 20150   // GP2Y0A41SK0F : 2..15 cm
#endif

#ifndef MODEL_LONG
#define MODEL_LONG 1080     // GP2Y0A21YK0F : 10..80 cm
#endif

namespace
{
    SharpIR sharpForwardShort(IR_SML, MODEL_SHORT);
    SharpIR sharpForwardLong(IR_M, MODEL_LONG);

    static constexpr float SHORT_MIN_VALID = 2.0f;
    static constexpr float SHORT_MAX_VALID = 15.0f;

    static constexpr float LONG_MIN_VALID  = 10.0f;
    static constexpr float LONG_MAX_VALID  = 80.0f;

    static constexpr float BLEND_START_CM = 10.0f;
    static constexpr float BLEND_END_CM   = 15.0f;

    static constexpr float EMA_ALPHA = 0.60f;     // faster response for robosumo
    static constexpr float MAX_STEP_CM = 18.0f;   // max allowed change per update

    inline float clampf(float x, float lo, float hi)
    {
        if (x < lo) return lo;
        if (x > hi) return hi;
        return x;
    }

    inline void sort3(float& a, float& b, float& c)
    {
        if (a > b) { float t = a; a = b; b = t; }
        if (b > c) { float t = b; b = c; c = t; }
        if (a > b) { float t = a; a = b; b = t; }
    }

    float readMedian3(SharpIR& sensor)
    {
        float a = (float)sensor.getDistance();
        float b = (float)sensor.getDistance();
        float c = (float)sensor.getDistance();

        sort3(a, b, c);
        return b;
    }

    bool isShortValid(float d)
    {
        return d >= SHORT_MIN_VALID && d <= SHORT_MAX_VALID;
    }

    bool isLongValid(float d)
    {
        return d >= LONG_MIN_VALID && d <= LONG_MAX_VALID;
    }

    float shortConfidence(float d)
    {
        if (!isShortValid(d)) return 0.0f;

        // strongest near the middle of its useful range
        // 2..15, best roughly around 7..10
        float center = 8.0f;
        float spread = 7.0f;
        float c = 1.0f - fabsf(d - center) / spread;
        return clampf(c, 0.15f, 1.0f);
    }

    float longConfidence(float d)
    {
        if (!isLongValid(d)) return 0.0f;

        // weakest near 10, stronger deeper into its intended range
        float c = (d - LONG_MIN_VALID) / (LONG_MAX_VALID - LONG_MIN_VALID);
        return clampf(0.25f + 0.75f * c, 0.25f, 1.0f);
    }
}

void SharpManager::init()
{
}

ForwardDistanceResult SharpManager::readForward()
{
    ForwardDistanceResult r{};

    r.shortCm = readMedian3(sharpForwardShort);
    r.longCm  = readMedian3(sharpForwardLong);

    r.shortValid = isShortValid(r.shortCm);
    r.longValid  = isLongValid(r.longCm);

    r.tooClose = false;
    r.outOfRange = false;
    r.fusedCm = -1.0f;

    // Very close handling
    if (r.shortCm > 0.0f && r.shortCm < SHORT_MIN_VALID)
    {
        r.tooClose = true;
        r.fusedCm = 0.0f;
        return r;
    }

    // Both valid -> confidence-based fusion
    if (r.shortValid && r.longValid)
    {
        float ws = shortConfidence(r.shortCm);
        float wl = longConfidence(r.longCm);

        // Extra overlap bias so short dominates near 10cm, long near 15cm
        float t = clampf((r.shortCm - BLEND_START_CM) / (BLEND_END_CM - BLEND_START_CM), 0.0f, 1.0f);
        ws *= (1.0f - 0.5f * t);
        wl *= (0.5f + 0.5f * t);

        r.fusedCm = (ws * r.shortCm + wl * r.longCm) / (ws + wl);
        return r;
    }

    // Only short valid
    if (r.shortValid)
    {
        r.fusedCm = r.shortCm;
        return r;
    }

    // Only long valid
    if (r.longValid)
    {
        r.fusedCm = r.longCm;
        return r;
    }

    // No usable result
    r.outOfRange = true;
    r.fusedCm = -1.0f;
    return r;
}

float SharpManager::filterForwardDistance(float rawCm)
{
    static bool initialized = false;
    static float filtered = 0.0f;

    if (!initialized)
    {
        filtered = rawCm;
        initialized = true;
        return filtered;
    }

    // Jump clamp first
    float delta = rawCm - filtered;
    if (delta > MAX_STEP_CM) rawCm = filtered + MAX_STEP_CM;
    else if (delta < -MAX_STEP_CM) rawCm = filtered - MAX_STEP_CM;

    // Fast EMA for robosumo
    filtered = EMA_ALPHA * rawCm + (1.0f - EMA_ALPHA) * filtered;
    return filtered;
}

void SharpManager::printForwardResult(const ForwardDistanceResult& r, float filteredCm)
{
    Serial.print("S=");
    Serial.print(r.shortCm);
    Serial.print("  L=");
    Serial.print(r.longCm);
    Serial.print("  F=");

    if (r.outOfRange)
    {
        Serial.print("OUT");
    }
    else if (r.tooClose)
    {
        Serial.print("CLOSE");
    }
    else
    {
        Serial.print(r.fusedCm);
        Serial.print("  FF=");
        Serial.print(filteredCm);
    }

    Serial.print("  SV=");
    Serial.print(r.shortValid ? 1 : 0);
    Serial.print("  LV=");
    Serial.println(r.longValid ? 1 : 0);
}