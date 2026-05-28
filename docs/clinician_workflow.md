# Clinician workflow

The intended prototype workflow proceeds in four grouped phases (Section 4.5.2 of the final report).

## Prep

Load the 20 mL syringe with injection fluid and set the stopcock to the all-ports-open position.

## Prime and baseline

With the fluid circuit primed and the needle outlet open to reference pressure at the same vertical height as the sensor, power on or reset the ESP8266 so the BASELINE phase is acquired correctly. The firmware then enters WAIT_START and holds the motor disabled until the operator sends the serial start command.

## Deliver

Insert the needle using standard ophthalmic technique and send `s` over the serial monitor to begin the automated cycle. The system delivers a fixed volume increment, pauses for 3 seconds, averages 10 pressure readings to obtain the validated pause-state pressure, and repeats while validated pressure stays below 50 mmHg.

## Hold and finish

At 50 mmHg the system enters HOLD and monitors pressure for 10 minutes without further injection. At 60 mmHg the system stops as a safety cutoff. The clinician removes the needle after completion.

## Notes on the current build

The bench prototype is operated two-handed. One hand stabilises the eye or bottle, the other presses start. True one-handed operation requires the integrated single-handpiece geometry listed in Section 10, Tier 1. The bubble-removal protocol in calibration/bubble_removal_protocol.md is run at the start of every session before any reading is taken.
