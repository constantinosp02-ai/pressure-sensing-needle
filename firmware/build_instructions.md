# Firmware build instructions

TODO. The firmware source has not been added to this repository yet. When it is added under firmware/src/, fill in this file with:

First, the toolchain version used for the final build (Arduino IDE version, ESP8266 board package version) and any switch to PlatformIO.

Second, the exact library list and pinned versions, including the Honeywell MPRLS driver and any stepper or I2C helpers (cross-check against firmware/lib/ once populated).

Third, the upload procedure on the NodeMCU 1.0 board (port selection, flash size, upload speed).

Fourth, the calibration constants that must be set at build time: the zero-offset baseline window size (20 samples on power-up), the hydrostatic correction (5.71 mmHg at the as-built 80 mm sensor-to-tip height, scaling at 0.071 mmHg per millimetre), the validation window (10 samples), and the absolute trip thresholds (50 mmHg HOLD, 60 mmHg DONE).

The pin map is in firmware/pin_map.md. The state machine is in firmware/state_machine.md. The full description of the firmware is in Section 4.4 of the report. The clinician-facing CSV log format is described in Section 4.4.4.
