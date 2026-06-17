/*
 * =============================================
 *   ROBOSUMO BOT - Arduino Uno  (TEMP DEBUG SKETCH)
 * =============================================
 * Motor Driver: TB6612FNG
 *   PWMA  -> D3   (Right Motor PWM)
 *   AIN2  -> D4   (Right Motor Dir)
 *   AIN1  -> D12  (Right Motor Dir)
 *   BIN1  -> A2   (Left Motor Dir)
 *   BIN2  -> A1   (Left Motor Dir)
 *   PWMB  -> D9   (Left Motor PWM)
 *
 * Sensors:
 *   Sharp IR (20-80cm analog) -> A3
 *   Line sensor LEFT          -> D7 (0=white/line, 1=black/no line)
 *   Line sensor RIGHT         -> D8 (0=white/line, 1=black/no line)
 *   ToF sensor SDA            -> A4
 *   ToF sensor SCL            -> A5
 *
 * Required Libraries:
 *   - VL53L0X by Pololu (install via Library Manager)
 *
 * Serial monitor: 9600 baud
 * =============================================
 */

#include <Wire.h>
#include <VL53L0X.h>

// ── Motor Driver Pins ──────────────────────────────────────────────────────────
#define PWMA      3    // Right motor PWM
#define AIN2      4    // Right motor direction
#define AIN1      12   // Right motor direction
#define BIN1      A2   // Left motor direction
#define BIN2      A1   // Left motor direction
#define PWMB      9    // Left motor PWM

// ── Sensor Pins ───────────────────────────────────────────────────────────────
#define SHARP_PIN       A3   // Sharp IR analog input
#define LINE_LEFT_PIN   7    // Line sensor left  (0=white, 1=black)
#define LINE_RIGHT_PIN  8    // Line sensor right (0=white, 1=black)

// ── Tuning Constants ──────────────────────────────────────────────────────────
#define MOTOR_SPEED_FULL    255   // Full attack speed
#define MOTOR_SPEED_SEARCH  180   // Speed while searching
#define MOTOR_SPEED_REVERSE 220   // Reverse speed when line detected
#define MOTOR_SPEED_TURN    200   // Turn speed

#define SHARP_DETECT_CM     70    // Sharp sensor: enemy detected within X cm
#define TOF_DETECT_MM       500   // ToF sensor: enemy detected within X mm (500=50cm)
#define TOF_TIMEOUT_MM      2000  // ToF reading timeout sentinel value

#define REVERSE_MS          250   // How long to reverse when line detected (ms)
#define TURN_MS             300   // How long to turn after reversing (ms)

#define STARTUP_DELAY_MS    5000  // 5-second countdown before match start

#define DEBUG_INTERVAL_MS   300   // how often the verbose debug line prints

// ── State Machine ─────────────────────────────────────────────────────────────
enum RobotState {
  STATE_STARTUP,
  STATE_SEARCH,
  STATE_ATTACK,
  STATE_AVOID_LINE_LEFT,
  STATE_AVOID_LINE_RIGHT,
  STATE_AVOID_LINE_BOTH
};

RobotState currentState = STATE_STARTUP;

// ── Globals ───────────────────────────────────────────────────────────────────
VL53L0X tofSensor;
bool tofAvailable = false;
unsigned long stateTimer = 0;

// Last sensor reads, stashed by enemyDetected() so the debug dump can print
// them without re-reading the hardware.
uint16_t g_tofMM      = 0;
bool     g_tofTimeout = false;
int      g_sharpRaw   = 0;
float    g_sharpCm    = 0.0;
unsigned long lastDebug = 0;

// ── Forward Declarations ──────────────────────────────────────────────────────
void motorRight(int speed);
void motorLeft(int speed);
void driveForward(int speed);
void driveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);
void stopMotors();
bool enemyDetected();
void handleLineAvoidance(bool leftLine, bool rightLine);
const char* stateName(RobotState s);
void printDebug(bool leftLine, bool rightLine, bool enemy);

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(9600);

  // Motor driver pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Sensor pins
  pinMode(LINE_LEFT_PIN, INPUT);
  pinMode(LINE_RIGHT_PIN, INPUT);
  // A3 is analog input by default — no pinMode needed

  // Init ToF sensor over I2C
  Wire.begin();
  if (tofSensor.init()) {
    tofSensor.setTimeout(50);
    tofSensor.startContinuous(20);  // Read every 20 ms
    tofAvailable = true;
    Serial.println(F("[ToF] VL53L0X OK"));
  } else {
    Serial.println(F("[ToF] Not found - using Sharp IR only"));
  }

  stopMotors();

  // ── 5-second startup delay (rules-compliant) ──
  Serial.println(F("Starting in 5 seconds..."));
  for (int i = 5; i > 0; i--) {
    Serial.print(i);
    Serial.println(F("..."));
    delay(1000);
  }
  Serial.println(F("FIGHT!"));

  currentState = STATE_SEARCH;
}

// =============================================================================
//  MAIN LOOP
// =============================================================================
void loop() {
  // ── Read sensors ────────────────────────────────────────────────────────────
  bool leftLine  = (digitalRead(LINE_LEFT_PIN)  == LOW);  // LOW = white = edge!
  bool rightLine = (digitalRead(LINE_RIGHT_PIN) == LOW);

  bool enemy = enemyDetected();

  // ── DEBUG: verbose dump of everything we just read ───────────────────────────
  printDebug(leftLine, rightLine, enemy);

  // ── Line edge takes absolute priority ───────────────────────────────────────
  if (leftLine || rightLine) {
    handleLineAvoidance(leftLine, rightLine);
    return;
  }

  // ── State transitions ────────────────────────────────────────────────────────
  switch (currentState) {

    // ── Searching: slow rotation until enemy found ───────────────────────────
    case STATE_SEARCH:
      turnRight(MOTOR_SPEED_SEARCH);
      if (enemy) {
        currentState = STATE_ATTACK;
        Serial.println(F(">> ATTACK"));
      }
      break;

    // ── Attack: full speed toward enemy ─────────────────────────────────────
    case STATE_ATTACK:
      driveForward(MOTOR_SPEED_FULL);
      if (!enemy) {
        currentState = STATE_SEARCH;
        Serial.println(F(">> SEARCH"));
      }
      break;

    default:
      currentState = STATE_SEARCH;
      break;
  }
}

// =============================================================================
//  VERBOSE DEBUG DUMP  (added)
//  Throttled one-line dump of every sensor + state + motor intent.
// =============================================================================
void printDebug(bool leftLine, bool rightLine, bool enemy) {
  if (millis() - lastDebug < DEBUG_INTERVAL_MS) return;
  lastDebug = millis();

  Serial.print(F("[DBG] State="));
  Serial.print(stateName(currentState));

  Serial.print(F(" | Line L="));
  Serial.print(leftLine);
  Serial.print(F(" R="));
  Serial.print(rightLine);

  Serial.print(F(" | ToF="));
  if (!tofAvailable) {
    Serial.print(F("N/A"));
  } else if (g_tofTimeout) {
    Serial.print(F("TIMEOUT"));
  } else {
    Serial.print(g_tofMM);
    Serial.print(F("mm"));
  }

  Serial.print(F(" | Sharp raw="));
  Serial.print(g_sharpRaw);
  Serial.print(F(" ~"));
  Serial.print(g_sharpCm, 1);
  Serial.print(F("cm"));

  Serial.print(F(" | enemy="));
  Serial.print(enemy);

  Serial.print(F(" | motors="));
  if (leftLine || rightLine)             Serial.print(F("LINE-AVOID"));
  else if (currentState == STATE_ATTACK) Serial.print(F("FWD-FULL"));
  else                                   Serial.print(F("TURN-R-SEARCH"));

  Serial.println();
}

const char* stateName(RobotState s) {
  switch (s) {
    case STATE_STARTUP: return "STARTUP";
    case STATE_SEARCH:  return "SEARCH";
    case STATE_ATTACK:  return "ATTACK";
    default:            return "AVOID";
  }
}

// =============================================================================
//  LINE AVOIDANCE
//  Called when either line sensor triggers. Robot reverses away from the edge
//  then turns back into the ring.
// =============================================================================
void handleLineAvoidance(bool leftLine, bool rightLine) {
  Serial.print(F("[LINE] L="));
  Serial.print(leftLine);
  Serial.print(F(" R="));
  Serial.println(rightLine);

  // Step 1 – Reverse
  driveBackward(MOTOR_SPEED_REVERSE);
  delay(REVERSE_MS);

  // Step 2 – Turn back toward center
  if (leftLine && rightLine) {
    // Both sensors on white → completely over the edge, reverse + sharp turn
    driveBackward(MOTOR_SPEED_REVERSE);
    delay(100);
    turnRight(MOTOR_SPEED_TURN);
    delay(TURN_MS + 100);
  } else if (leftLine) {
    // Left sensor on white → edge is to the left → turn right
    turnRight(MOTOR_SPEED_TURN);
    delay(TURN_MS);
  } else {
    // Right sensor on white → edge is to the right → turn left
    turnLeft(MOTOR_SPEED_TURN);
    delay(TURN_MS);
  }

  stopMotors();
  currentState = STATE_SEARCH;
}

// =============================================================================
//  ENEMY DETECTION
//  Returns true if either sensor detects something within threshold.
//  Also stashes the raw reads into g_* globals for the debug dump.
// =============================================================================
bool enemyDetected() {
  bool detected = false;

  // ToF sensor (primary – more accurate)
  if (tofAvailable) {
    g_tofMM      = tofSensor.readRangeContinuousMillimeters();
    g_tofTimeout = tofSensor.timeoutOccurred();
    if (!g_tofTimeout && g_tofMM < TOF_DETECT_MM) {
      detected = true;
    }
  }

  // Sharp IR sensor (secondary / wider angle backup)
  g_sharpRaw = analogRead(SHARP_PIN);
  // Sharp GP2Y0A21 / similar: higher raw value = closer object
  // Full formula: distance_cm = 27.86 * pow(voltage, -1.15)
  float voltage = g_sharpRaw * (5.0 / 1023.0);
  g_sharpCm     = 27.86 * pow(voltage, -1.15);

  if (g_sharpCm > 0 && g_sharpCm < SHARP_DETECT_CM) {
    detected = true;
  }

  return detected;
}

// =============================================================================
//  MOTOR CONTROL — TB6612FNG
//
//  Right motor: AIN1/AIN2 + PWMA
//  Left  motor: BIN1/BIN2 + PWMB
//
//  Forward:  AIN1=HIGH, AIN2=LOW  |  BIN1=HIGH, BIN2=LOW
//  Backward: AIN1=LOW,  AIN2=HIGH |  BIN1=LOW,  BIN2=HIGH
//  Brake:    AIN1=HIGH, AIN2=HIGH |  BIN1=HIGH, BIN2=HIGH
//  Coast:    AIN1=LOW,  AIN2=LOW  |  BIN1=LOW,  BIN2=LOW
// =============================================================================

// speed: -255 (full back) to +255 (full forward)
void motorRight(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    speed = -speed;
  }
  analogWrite(PWMA, speed);
}

void motorLeft(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    speed = -speed;
  }
  analogWrite(PWMB, speed);
}

void driveForward(int speed) {
  motorRight(speed);
  motorLeft(speed);
}

void driveBackward(int speed) {
  motorRight(-speed);
  motorLeft(-speed);
}

void turnLeft(int speed) {
  motorRight(speed);
  motorLeft(-speed);
}

void turnRight(int speed) {
  motorRight(-speed);
  motorLeft(speed);
}

void stopMotors() {
  // Brake both motors
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 0);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, 0);
}
