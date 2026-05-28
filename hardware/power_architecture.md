# Power architecture

Reproduced from Table 32 in Appendix A.6 of the final report. Three rails plus a common ground bus (Section 4.4.1).

| Rail | Source | Loads |
|---|---|---|
| 3.3 V | ESP8266 onboard regulator | MPRLS, I2C pull-ups, level shifter LV reference, MT3608 input |
| 5 V | MT3608 boost converter | Level shifter HV reference, DRV8825 logic VDD |
| 24 V | Dedicated PSU | DRV8825 VMOT only |
| GND | Common bus | All component grounds |

## Notes

Without the shared ground bus, the I2C bus and the level-shifted motor control signals do not work reliably (Section 4.4.1).

Set the DRV8825 Vref potentiometer to about 70 percent of the rated phase current of 1.5 A before connecting the motor or the 24 V supply.

The Arduino-based build used in earlier iterations relied on the Arduino onboard regulator for the 5 V rail. That put the driver ground on the same rail as the microcontroller, where it picked up switching noise, and the bare 3.3 V signal sat right on the edge of the DRV8825 input-high threshold. The MT3608 boost converter and the level shifter were added to fix both problems on the ESP8266 build (Section 5.4, Iteration 4).
