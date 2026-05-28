/* =====================================================================
   Syringe pressure-control, fixed increment + validate version
   ---------------------------------------------------------------------
   Pins, NodeMCU / Wemos D1 Mini, ESP8266:
     D5 / GPIO14  -> STEP  DRV8825, via level shifter
     D6 / GPIO12  -> DIR   DRV8825, via level shifter
     D7 / GPIO13  -> EN    DRV8825, active-low, via level shifter
     D2 / GPIO4   -> SDA   MPRLS
     D1 / GPIO5   -> SCL   MPRLS
   ===================================================================== */

#include <ESP8266WiFi.h>
#include <Wire.h>

#define DIR_PIN     12
#define STEP_PIN    14
#define EN_PIN      13

#define MPRLS_ADDR       0x18
#define MPRLS_OUTPUT_MIN 419430.0
#define MPRLS_OUTPUT_MAX 3774874.0
#define P_MIN_MMHG       0.0
#define P_MAX_MMHG       300.0

const float SYRINGE_BORE_MM       = 19.1f;
const float SYRINGE_AREA_MM2      = 3.14159265f * (SYRINGE_BORE_MM / 2.0f) * (SYRINGE_BORE_MM / 2.0f);
const float LEAD_MM_PER_REV       = 8.0f;
const float FULL_STEPS_PER_REV    = 200.0f;
const float MICROSTEPS            = 32.0f;
const float VOLUME_PER_STEP_UL    = SYRINGE_AREA_MM2 * (LEAD_MM_PER_REV / (FULL_STEPS_PER_REV * MICROSTEPS));

const float    TARGET_MMHG             = 50.0f;
const float    SAFETY_CUTOFF_MMHG      = 60.0f;
const float    HYDROSTATIC_OFFSET_MMHG = -5.71f;
const uint32_t HOLD_MS                 = 10UL * 60UL * 1000UL;
const uint32_t SETTLE_MS               = 3000;
const uint8_t  N_AVG                   = 10;
const uint8_t  N_BASELINE              = 20;
const uint8_t  MAX_BAD_PRESSURE_READS  = 3;
const uint32_t STEP_HALF_PERIOD_US     = 250000;
const uint32_t MPRLS_CONV_US           = 5500;
const bool     PUSH_DIR_HIGH           = true;
const float    PUSH_INCREMENT_UL       = 5.0f;
const uint32_t PUSH_INCREMENT_STEPS    = (uint32_t)(PUSH_INCREMENT_UL / VOLUME_PER_STEP_UL + 0.5f);
const float    MAX_PUSH_VOLUME_UL      = 200.0f;
const uint32_t MAX_PUSH_STEPS          = (uint32_t)(MAX_PUSH_VOLUME_UL / VOLUME_PER_STEP_UL);

enum Phase {
  PHASE_WAIT_START,
  PHASE_PUSH,
  PHASE_SETTLING,
  PHASE_VALIDATE,
  PHASE_HOLD,
  PHASE_DONE
};

Phase phase = PHASE_WAIT_START;
uint32_t pushStartSteps = 0;
uint32_t lastStepEdgeUs = 0;
bool     stepLevel      = false;
uint32_t totalSteps     = 0;

enum MprlsState { MPRLS_IDLE, MPRLS_WAITING };
MprlsState mprlsState     = MPRLS_IDLE;
uint32_t   mprlsTriggerUs = 0;

uint32_t settleStartMs = 0;
uint32_t holdStartMs   = 0;
float    avgAccum = 0.0f;
uint8_t  avgCount = 0;
float pressureZeroMmHg = 0.0f;
uint8_t badPressureReads = 0;

bool mprlsTrigger() {
  Wire.beginTransmission(MPRLS_ADDR);
  Wire.write(0xAA); Wire.write(0x00); Wire.write(0x00);
  uint8_t err = Wire.endTransmission();
  if (err != 0) return false;
  mprlsTriggerUs = micros();
  return true;
}

float mprlsReadRaw() {
  if (Wire.requestFrom((uint8_t)MPRLS_ADDR, (uint8_t)4) != 4) return NAN;
  uint8_t status = Wire.read();
  uint32_t raw   = ((uint32_t)Wire.read() << 16)
                 | ((uint32_t)Wire.read() << 8)
                 |  (uint32_t)Wire.read();
  if (status & 0x25) return NAN;
  return ((float)raw - MPRLS_OUTPUT_MIN) * (P_MAX_MMHG - P_MIN_MMHG)
         / (MPRLS_OUTPUT_MAX - MPRLS_OUTPUT_MIN) + P_MIN_MMHG;
}

float correctedPressure(float rawPressureMmHg) {
  return rawPressureMmHg - pressureZeroMmHg + HYDROSTATIC_OFFSET_MMHG;
}

float readPressureBlockingRaw() {
  if (!mprlsTrigger()) return NAN;
  delayMicroseconds(MPRLS_CONV_US);
  return mprlsReadRaw();
}

float acquireBaseline(uint8_t n) {
  float sum = 0.0f; uint8_t count = 0;
  while (count < n) {
    float p = readPressureBlockingRaw();
    if (!isnan(p)) { sum += p; count++; }
    yield();
  }
  return sum / (float)count;
}

void emitRow(float p_mmHg, const char* phaseStr) {
  Serial.print(millis());        Serial.print(',');
  Serial.print(p_mmHg, 3);       Serial.print(',');
  Serial.print(totalSteps);      Serial.print(',');
  Serial.println(phaseStr);
}

void stopMotor() {
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(EN_PIN, HIGH);
}

void sensorFaultStop() {
  stopMotor();
  phase = PHASE_DONE;
  Serial.println("# SENSOR_FAULT_STOP");
  Serial.println("# END_CSV");
}

void notePressureFault() {
  badPressureReads++;
  if (badPressureReads >= MAX_BAD_PRESSURE_READS) {
    sensorFaultStop();
  }
}

void enableMotor() {
  digitalWrite(STEP_PIN, LOW);
  stepLevel = false;
  lastStepEdgeUs = micros();
  digitalWrite(EN_PIN, LOW);
}

void serviceStartCommand() {
  if (phase != PHASE_WAIT_START) return;
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 's' || c == 'S') {
      pushStartSteps = totalSteps;
      enableMotor();
      phase = PHASE_PUSH;
      Serial.println("# START_COMMAND_RECEIVED");
    }
  }
}

void serviceStepper() {
  if (phase != PHASE_PUSH) return;
  uint32_t now = micros();
  if (now - lastStepEdgeUs >= STEP_HALF_PERIOD_US) {
    stepLevel = !stepLevel;
    digitalWrite(STEP_PIN, stepLevel ? HIGH : LOW);
    lastStepEdgeUs = now;
    if (stepLevel) {
      totalSteps++;
      if (totalSteps >= MAX_PUSH_STEPS) {
        stopMotor(); phase = PHASE_DONE;
        Serial.println("# MAX_PUSH_STEPS_REACHED");
        Serial.println("# END_CSV"); return;
      }
      if ((totalSteps - pushStartSteps) >= PUSH_INCREMENT_STEPS) {
        stopMotor(); phase = PHASE_SETTLING;
        settleStartMs = millis(); return;
      }
    }
  }
}

void servicePressure() {
  if (mprlsState == MPRLS_IDLE) {
    if (mprlsTrigger()) {
      mprlsState = MPRLS_WAITING;
    } else {
      notePressureFault();
    }
    return;
  }
  if ((micros() - mprlsTriggerUs) < MPRLS_CONV_US) return;
  float raw = mprlsReadRaw();
  mprlsState = MPRLS_IDLE;
  if (isnan(raw)) {
    notePressureFault();
    return;
  }
  badPressureReads = 0;
  float p = correctedPressure(raw);

  if (phase == PHASE_PUSH) return;

  if (phase == PHASE_SETTLING) {
    if ((millis() - settleStartMs) >= SETTLE_MS) {
      phase = PHASE_VALIDATE; avgAccum = 0.0f; avgCount = 0;
    }
    return;
  }

  if (phase == PHASE_VALIDATE) {
    avgAccum += p; avgCount++;
    if (avgCount >= N_AVG) {
      float validated = avgAccum / (float)N_AVG;
      emitRow(validated, "VALIDATED");
      avgAccum = 0.0f; avgCount = 0;
      if (validated >= SAFETY_CUTOFF_MMHG) {
        emitRow(validated, "SAFETY_VALIDATED");
        phase = PHASE_DONE;
        Serial.println("# SAFETY_CUTOFF_REACHED");
        Serial.println("# END_CSV"); return;
      }
      if (validated >= TARGET_MMHG) {
        emitRow(validated, "TARGET_REACHED");
        Serial.println("# TARGET_50_MMHG_REACHED");
        phase = PHASE_HOLD; holdStartMs = millis(); return;
      }
      pushStartSteps = totalSteps;
      enableMotor(); phase = PHASE_PUSH;
    }
    return;
  }

  if (phase == PHASE_HOLD) {
    avgAccum += p; avgCount++;
    if (avgCount >= N_AVG) {
      float averaged = avgAccum / (float)N_AVG;
      emitRow(averaged, "HOLD");
      avgAccum = 0.0f; avgCount = 0;
    }
    if ((millis() - holdStartMs) >= HOLD_MS) {
      phase = PHASE_DONE; Serial.println("# END_CSV");
    }
    return;
  }
}

void setup() {
  Serial.begin(115200); delay(50);
  WiFi.mode(WIFI_OFF); WiFi.forceSleepBegin(); delay(1);
  pinMode(DIR_PIN, OUTPUT); pinMode(STEP_PIN, OUTPUT); pinMode(EN_PIN, OUTPUT);
  digitalWrite(DIR_PIN, PUSH_DIR_HIGH ? HIGH : LOW);
  digitalWrite(STEP_PIN, LOW); digitalWrite(EN_PIN, HIGH);
  Wire.begin(4, 5); delay(10);
  pressureZeroMmHg = acquireBaseline(N_BASELINE);
  Serial.println(); Serial.println("# BEGIN_CSV");
  Serial.print("# baseline_zero_mmHg="); Serial.println(pressureZeroMmHg, 3);
  Serial.print("# hydrostatic_offset_mmHg="); Serial.println(HYDROSTATIC_OFFSET_MMHG, 3);
  Serial.print("# syringe_bore_mm="); Serial.println(SYRINGE_BORE_MM, 2);
  Serial.print("# volume_per_step_uL="); Serial.println(VOLUME_PER_STEP_UL, 4);
  Serial.print("# push_increment_uL="); Serial.println(PUSH_INCREMENT_UL, 2);
  Serial.print("# push_increment_steps="); Serial.println(PUSH_INCREMENT_STEPS);
  Serial.print("# target_mmHg="); Serial.println(TARGET_MMHG, 1);
  Serial.print("# safety_cutoff_mmHg="); Serial.println(SAFETY_CUTOFF_MMHG, 1);
  Serial.print("# max_bad_pressure_reads="); Serial.println(MAX_BAD_PRESSURE_READS);
  Serial.print("# max_push_steps="); Serial.println(MAX_PUSH_STEPS);
  Serial.println("time_ms,pressure_mmHg,total_steps,phase");
  stopMotor(); phase = PHASE_WAIT_START;
  Serial.println("# WAITING_FOR_START_COMMAND");
  Serial.println("# Type s to start automated injection cycle");
}

void loop() {
  serviceStartCommand();
  if (phase != PHASE_DONE && phase != PHASE_WAIT_START) {
    serviceStepper(); servicePressure();
  } else if (phase == PHASE_DONE) {
    stopMotor(); delay(1000);
  }
  yield();
}
