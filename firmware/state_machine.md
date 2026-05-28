# Firmware state machine

The firmware runs on the ESP8266 and implements the inject-pause-measure cycle as a linear state machine. The phases follow the operational states described in Section 4.1 and are driven by validated pressure readings taken during pause phases. The state transition diagram is Figure 10 in the report. The full description is in Section 4.4.2.

## BASELINE

On power-up, the ESP8266 initialises the MPRLS sensor and the DRV8825 driver. Before any fluid is delivered, the firmware acquires 20 consecutive pressure readings and computes their mean as the zero-offset baseline. This baseline is subtracted from all subsequent readings to remove any residual sensor or system offset before the hydrostatic correction (Section 4.2.3) is applied. The motor driver is disabled during this phase.

## WAIT_START

After baseline acquisition, the firmware keeps the motor driver disabled and waits for an operator start command over USB serial. This prevents actuation from beginning before the clinician has inserted the needle and confirmed that the system is ready. When the operator sends `s`, the firmware enables the DRV8825 and transitions to PUSH.

## PUSH

The motor advances the syringe plunger by a fixed number of STEP pulses at 2 pulses per second at 1/32 microstepping (Section 4.3.2). Each pulse delivers 0.358 microlitres (Section 4.3.1). Each PUSH phase delivers a fixed 5 microlitre increment, equivalent to approximately 14 STEP pulses. This specific increment balances measurement frequency against practical cycle time. During active flow, the sensor reading is hub pressure rather than true IOP because the Hagen-Poiseuille pressure drop across the needle is present (Section 4.1.2). No control decisions are made from hub pressure readings during this phase. Once the fixed increment has been delivered, the firmware stops the motor and transitions to SETTLING.

## SETTLING

The motor is off for 3 seconds so that the flow-induced pressure transient can dissipate and the fluid column between the sensor and needle tip can approach hydrostatic equilibrium. No pressure readings are used for validation during this period.

## VALIDATE

The firmware acquires 10 consecutive pressure readings (about 55 ms at the sensor's 5.5 ms conversion time), applies the baseline and hydrostatic corrections, and computes their arithmetic mean as the validated pause-state pressure. This averaged value is the only measurement treated as true IOP by the control system. The validated pressure determines the next transition. First, if it reaches 60 mmHg, the motor remains off and the system transitions directly to DONE as a safety cut-off. Second, if it reaches 50 mmHg, the system transitions to HOLD. Third, if it remains below 50 mmHg, the system returns to PUSH for another fixed increment.

## HOLD

After the final validated measurement reaches the 50 mmHg target, the motor remains off and the system continues to monitor pressure for 10 minutes, logging averaged readings throughout. This phase captures the post-injection pressure response as the scleral collagen relaxes (Section 2.2.2), providing the continuous data record that is central to the clinical value of the device.

## DONE

The system emits an end marker over serial and halts. The motor is disabled (EN set HIGH). The system can only be restarted by resetting the ESP8266.

## Measurement discipline

Valid IOP measurements are taken only in the VALIDATE phase, after the motor has stopped and the settling delay has elapsed. All control decisions (whether to continue injecting, hold or stop) are made exclusively from validated pause-state pressure, never from hub pressure during active flow.

## Step-count safeguard

The firmware also enforces a maximum cumulative step count of about 560 steps, which caps total delivered volume at 200 microlitres as a secondary safeguard against runaway injection (Section 4.3.1).
