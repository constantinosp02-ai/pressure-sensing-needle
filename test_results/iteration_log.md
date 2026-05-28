# Iteration log

TODO. The iteration log lives in Section 5.4 of the final report. Reproduce or reference it here once the test CSVs in ./subsystem/ and ./integration/ have been added. The five iterations on the final build are:

First, air entrapment in the fluid circuit. Resolved by the bubble-removal protocol (Section 5.2.2). Drift fell from about 5 mmHg over 30 seconds to 0.5 to 1.0 mmHg over the same window.

Second, sensor procurement. Three intermediate candidates were ruled out before the Honeywell MPRLS0300YG was confirmed against the plus or minus 2 mmHg accuracy target (Section 5.2.1).

Third, fluid upgrade from distilled water to PBS as the primary test fluid. Sensor accuracy was unchanged within measurement noise.

Fourth, microcontroller architecture moved from an Arduino-based build to the ESP8266, with the MT3608 boost converter for the 5 V rail and a level shifter on the STEP, DIR and EN lines. The ESP8266 build logged zero missed steps and zero I2C errors across 25 consecutive inject-pause-measure cycles.

Fifth, closed-loop and integration validation. Tests 10 and 11 in the risk assessment confirmed an inject-pause-measure cycle of 3.05 seconds on average, well below the 4 second target. No design changes were required at this iteration.
