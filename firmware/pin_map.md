# ESP8266 pin map

Reproduced from Table 33 in Appendix A.6 of the final report. This is the pin allocation for the final build (Section 4.4.1).

| ESP8266 pin | Component | Signal type | Notes |
|---|---|---|---|
| D2 (GPIO4) | MPRLS0300YG SDA | I2C data | 4.7 kohm pull-up to 3.3 V |
| D1 (GPIO5) | MPRLS0300YG SCL | I2C clock | 4.7 kohm pull-up to 3.3 V |
| D5 (GPIO14) | DRV8825 STEP (via level shifter) | Digital output | One pulse per microstep |
| D6 (GPIO12) | DRV8825 DIR (via level shifter) | Digital output | HIGH = forward |
| D7 (GPIO13) | DRV8825 EN (via level shifter) | Digital output | LOW = motor enabled |
| 3.3 V | MPRLS VCC, pull-ups, level shifter LV, MT3608 | Power | ESP8266 onboard regulator |
| GND | All components | Ground | Common ground bus |

## Microstepping

M0, M1 and M2 on the DRV8825 are wired HIGH to set 1/32 microstepping (Section 4.3.1). Each microstep delivers 0.358 microlitres.

## Vref

Set the DRV8825 Vref potentiometer to about 70 percent of the rated phase current of 1.5 A before connecting the motor or the 24 V supply (Section 4.4.1).
