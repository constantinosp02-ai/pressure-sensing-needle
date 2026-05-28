# Firmware build instructions

The firmware source lives in `src/syringe_pressure_control_code.ino`. It is a single-file Arduino sketch that targets the ESP8266 (NodeMCU 1.0, ESP-12E module). The state machine is described in `state_machine.md` and the pin allocation is in `pin_map.md`.

## Toolchain

Arduino IDE 2.x with the ESP8266 board package. Install via Boards Manager by adding the package URL `https://arduino.esp8266.com/stable/package_esp8266com_index.json` under Preferences, then installing "esp8266 by ESP8266 Community". The sketch has been built and flashed against board package version 3.1.2.

## Dependencies

Only the Arduino core libraries are used. No external libraries to install.

- `ESP8266WiFi.h`: bundled with the ESP8266 core. WiFi is explicitly disabled at boot to keep the radio quiet during pressure measurement.
- `Wire.h`: bundled with the Arduino core, used for I2C to the MPRLS0300YG at address 0x18.

## Upload procedure

1. Connect the ESP8266 to the host computer over USB. The board exposes a CP2102 or CH340 USB-to-serial bridge depending on the manufacturer.
2. Select Tools, Board, NodeMCU 1.0 (ESP-12E Module).
3. Select Tools, Port, and pick the USB serial port the board is on.
4. Leave Upload Speed at 115200 baud, Flash Size at 4MB (FS:2MB OTA:~1019KB), CPU Frequency at 80 MHz, Flash Mode at DIO.
5. Open `src/syringe_pressure_control_code.ino` and click Upload.
6. After upload, open the Serial Monitor at 115200 baud to see the CSV stream described in Section 4.4.4.

## Calibration constants set at build time

The constants below live at the top of the .ino file. Verify them before flashing a new build, since they encode the as-built physical geometry.

- `HYDROSTATIC_OFFSET_MMHG = -5.71f`: matches the as-built 80 mm sensor-to-tip height (Section 4.2.3). Scales at 0.071 mmHg per millimetre. If the sensor-to-tip height changes, recalculate and update this constant.
- `SYRINGE_BORE_MM = 19.1f`: internal diameter of the 20 mL Terumo Luer-lock syringe.
- `LEAD_MM_PER_REV = 8.0f`, `FULL_STEPS_PER_REV = 200.0f`, `MICROSTEPS = 32.0f`: Tr8x8 lead screw with NEMA 17 motor at 1/32 microstepping.
- `PUSH_INCREMENT_UL = 5.0f`: fixed inject increment per PUSH phase. The code rounds this to an integer step count (14 steps at the as-built geometry).
- `TARGET_MMHG = 50.0f`, `SAFETY_CUTOFF_MMHG = 60.0f`: absolute trip thresholds. These are anchored to absolute pressure values, not to the running baseline (Section 6.2.1).
- `N_BASELINE = 20`, `N_AVG = 10`: baseline window on power-up and validation window during pause-state measurement.
- `SETTLE_MS = 3000`, `HOLD_MS = 600000`: 3 second settling delay and 10 minute HOLD phase.
- `MAX_PUSH_VOLUME_UL = 200.0f`: hard cap on cumulative delivered volume as a runaway safeguard (Section 4.3.1).

## CSV log format

The firmware streams a header block prefixed with `#` lines, then a CSV header `time_ms,pressure_mmHg,total_steps,phase`, then rows during VALIDATE and HOLD. End-of-run markers are `# END_CSV` and one of `# TARGET_50_MMHG_REACHED`, `# SAFETY_CUTOFF_REACHED`, `# SENSOR_FAULT_STOP` or `# MAX_PUSH_STEPS_REACHED`. Save the full serial capture per run under `test_results/`.
