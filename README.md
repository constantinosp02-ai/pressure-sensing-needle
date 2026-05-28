# Pressure-Sensing Injection Needle, Handover README

MECH0073 MEng Capstone Group Design Project, 2025/2026.
Team: Constantinos Papacharalambous, Miguel Landa Pulgar, Álvaro Cachón Delgado, Vasilis Markides.
Supervisor: Dr Abigail Wilson. Module Tutor: Dr Andrea Grech La Rosa.

This README is the entry point to the project repository. The wider Handover Package, including the final report PDF, the Fusion 360 cloud project, the raw test data and the physical-items checklist, lives in the team drive. This README explains what the device does, how to set it up from cold, how to run the inject-pause-measure cycle, where the data lives, and what is left for the next team. Section references point back to the final report.

## What it does

The device is a closed-loop pressure-sensing injection needle for the treatment of ocular hypotony. A needle, a pressure sensor and a motor-driven syringe meet at a three-way stopcock that acts as a shared hydraulic junction. The microcontroller reads pressure from the sensor and drives the syringe motor. The stopcock also gives the clinician a manual way to stop flow to the eye if anything goes wrong, independently of the electronics. The full clinical motivation is in Section 2.1.

The control strategy is the inject-pause-measure cycle. The motor delivers a fixed 5 microlitre increment, the firmware pauses for 3 seconds to let flow transients settle, then averages 10 sensor readings to get the validated pause-state pressure. That validated value is the only number the controller trusts as true IOP. The cycle is described in full in Section 4.4.2.

## What is in the repository

Each top-level folder maps to one of the artefacts listed in Section 8.3 of the report.

```
pressure-sensing-needle/
  README.md              You are here
  CHANGELOG.md           Release notes (firmware tagging deferred to v1.0)
  CITATION.cff           Citation metadata for the project team
  LICENSE                MIT
  bom/                   Bill of Materials, project reference 547914 (Appendix B)
  cad/                   Fusion 360 model, STEP, STL, PDF render (Section 4.5.1, Figure 11)
  calibration/           Bench rig calibration, hydrostatic offset, drift logs, bubble removal protocol (Section 5.2)
  docs/                  Clinician workflow, known limitations, future work, contact
  drawings/              Engineering drawings (Appendix E)
  firmware/              ESP8266 source, pin map, state machine, build instructions (Section 4.4)
  hardware/              Schematic, wiring diagram, power architecture (Section 4.4.1, Tables 32 and 33)
  regulatory/            PDS compliance table, standards mapping (Appendices D and I)
  risk/                  Risk register, FMEA (Appendix A)
  test_results/          Subsystem and integration test CSVs, iteration log (Sections 5.2, 5.3, 5.4)
```

Several folders currently hold only a placeholder with a TODO pointing at the matching folder in the team-drive Handover Package. They will be populated as the team migrates the source code, schematics, BoM and test data into the repository ahead of the firmware v1.0 tag.

## Hardware setup

The prototype is split across two breadboards plus the mechanical syringe stand. The split keeps the noisy motor switching path away from the pressure sensor and microcontroller wiring (Section 4.4.1).

Mechanical assembly. The syringe stand is FDM printed in PLA. It has four vertical posts with diagonal cross-bracing between a top platform and a bottom platform. The 20 mL Terumo syringe sits in the central hole in the top platform with the Luer-lock tip pointing down. The NEMA 17 motor is bolted to the bottom platform with the Tr8x8 lead screw extending upward. A circular platform on the brass lead nut contacts the syringe plunger and drives it as the motor rotates. The three-way stopcock and the MPRLS0300YG sensor are both fixed to the bottom platform. The needle outlet sits about 80 mm above the sensor (Section 4.5.1).

Hydraulic path. Syringe tip to 4 mm ID silicone tubing to three-way stopcock to needle branch tubing to 25 G needle. All fluidic interfaces use ISO 80369-7 Luer-lock fittings (Section 9).

Electronics layout. The small breadboard hosts the ESP8266, the level shifter and the MT3608 boost converter. The large breadboard hosts the DRV8825, the 100 microfarad VMOT capacitor, the 24 V motor input and the sensor I2C connections (Section 4.4.1).

## Power architecture

Three rails plus a common ground bus (Section 4.4.1 and Appendix A.6).

The 3.3 V rail comes from the ESP8266 onboard regulator and powers the MPRLS sensor, the I2C pull-ups, the LV side of the level shifter, and the input to the MT3608 boost converter. The 5 V rail comes from the MT3608 and powers only the HV side of the level shifter and the DRV8825 logic VDD. The 24 V rail is a dedicated 2 A DC adapter that feeds only the DRV8825 VMOT input. The ground bus ties all component grounds together. Without that shared ground, the I2C bus and the level-shifted motor control signals do not work reliably.

Before powering anything on, set the DRV8825 Vref potentiometer to about 70% of the motor's rated phase current of 1.5 A. Do this before connecting the motor or the 24 V supply.

## Pin map

ESP8266 pin allocation from Table 33 (Appendix A.6).

```
D2 (GPIO4)   MPRLS0300YG SDA    I2C data, 4.7 kohm pull-up to 3.3 V
D1 (GPIO5)   MPRLS0300YG SCL    I2C clock, 4.7 kohm pull-up to 3.3 V
D5 (GPIO14)  DRV8825 STEP       Digital out, one pulse per microstep
D6 (GPIO12)  DRV8825 DIR        Digital out, HIGH is forward
D7 (GPIO13)  DRV8825 EN         Digital out, LOW enables motor coils
3.3 V        MPRLS VCC, pull-ups, level shifter LV, MT3608 in
GND          All components, common ground bus
```

Microstepping is set to 1/32 by wiring M0, M1 and M2 all HIGH on the DRV8825 (Section 4.3.1).

## Firmware overview

The firmware runs on the ESP8266 and implements the inject-pause-measure cycle as a linear state machine (Section 4.4.2).

BASELINE. On power-up, the firmware initialises the MPRLS sensor and the DRV8825, then acquires 20 consecutive pressure readings and stores their mean as the zero-offset baseline. The motor driver is disabled during this phase.

WAIT_START. The motor driver stays disabled. The firmware waits for the operator to send `s` over USB serial. When `s` arrives, the DRV8825 is enabled and the firmware transitions to PUSH.

PUSH. The motor advances the plunger by a fixed 5 microlitre increment, about 14 STEP pulses at 2 pulses per second at 1/32 microstepping. Each pulse delivers 0.358 microlitres. During flow, the sensor reads hub pressure (true IOP plus the Hagen-Poiseuille drop across the needle), so no control decisions are made from this phase.

SETTLING. The motor is off for 3 seconds while the flow transient dissipates.

VALIDATE. The firmware acquires 10 consecutive readings (about 55 ms at the sensor's 5.5 ms conversion time), applies the baseline and the hydrostatic correction, and computes the arithmetic mean. That mean is the validated pause-state pressure. Control logic: if it reaches 60 mmHg, the system goes straight to DONE as a safety cut-off. If it reaches 50 mmHg, it goes to HOLD. Otherwise it returns to PUSH for another increment.

HOLD. The motor stays off for 10 minutes while the firmware logs averaged readings. This captures the post-injection scleral collagen relaxation curve.

DONE. The firmware emits an end marker over serial and halts. The motor is disabled (EN HIGH). The system can only be restarted by resetting the ESP8266.

The firmware also enforces a maximum cumulative step count of about 560 steps, which caps total delivered volume at 200 microlitres as a secondary safeguard against runaway injection (Section 4.3.1).

## How to run a bench-rig cycle

First time setup. Print the syringe stand from the STL under cad/ (or from the Fusion 360 cloud project listed in cad/fusion_link.md). Wire the electronics according to firmware/pin_map.md and the wiring diagram under hardware/wiring/. Set the DRV8825 Vref before plugging the motor in. Set M0, M1, M2 all HIGH for 1/32 microstepping.

Each run. Load the 20 mL syringe with the test fluid. Open all stopcock ports. Prime the fluid path and remove bubbles (this matters, see drift discussion below). Set the needle outlet so it is open to reference pressure at the same vertical height as the sensor during baseline. Connect the ESP8266 to the host computer over USB. Open a serial monitor at 115200 baud.

Power on or reset the ESP8266 so the BASELINE phase runs. Wait for the firmware to report WAIT_START. Insert the needle using standard ophthalmic technique. Send `s` over the serial monitor. The system delivers a 5 microlitre increment, pauses 3 seconds, averages 10 readings, then either pushes again, holds at 50 mmHg, or stops at 60 mmHg.

The firmware streams a CSV log over the same serial link. Each row reports time in milliseconds, corrected pressure in mmHg, cumulative step count, and current phase (Section 4.4.4). Save the CSV for each run under test_results/subsystem/ for bench-rig data, or test_results/integration/ for full closed-loop or pig-eye workflow runs.

## Hydrostatic offset, how to re-zero

The sensor sits below the needle tip. With the tip 80 mm above the sensor, the sensor reads 5.71 mmHg higher than the true IOP. The firmware subtracts a fixed offset of 5.71 mmHg from every gauge reading before the controller or the display use it (Section 4.2.3).

The offset scales linearly with height at 0.071 mmHg per millimetre. If you change the assembly so the sensor to needle height shifts, measure the new height, recalculate the offset, and update the firmware constant. A height uncertainty of plus or minus 5 mm gives an offset uncertainty of about plus or minus 0.36 mmHg, which is inside the BFSL margin, so a careful millimetre rule is enough. Record the measured height and the calculated offset under calibration/ so the next team can verify it.

## Calibration, how it was done and how to re-run

Bench rig. A bottle filled to four heights (150, 225, 270 and 300 mm) gives reference pressures from rho g h. Run a sweep first in PBS, then in distilled water. PBS and water gave the same readings within the noise floor (Section 5.2.1).

Accuracy. Largest error across the four test points was 1 mmHg at the 270 mm point, inside the plus or minus 2 mmHg PDS threshold.

Drift. Hold the sensor at each fixed reference for 30 seconds. The PDS limit is 0.5 mmHg per minute. The final build settles at 0.5 to 1.0 mmHg over 30 seconds. The single biggest improvement came from the bubble removal step in the priming protocol. Without it, drift was about 5 mmHg in 30 seconds.

Bubble removal protocol (Section 5.2.2). Fluid is drawn through the circuit with the needle submerged, and the T-junction is gently tapped to release any trapped air. The protocol is run at the start of every session before any reading is taken. With it in routine use, no bubble-related pressure anomalies appeared.

CSV outputs. The serial log from each run is the calibration record. Save with the rig configuration in the filename (fluid, fill height, sensor height).

## Known limitations

What has been tested (Section 6.2.2). Pressure accuracy against a hydrostatic column from 0 to 22 mmHg (max 1 mmHg error), hydrostatic offset at three sensor heights, fluid circuit leak and trapped-air handling, motor and syringe delivered volume, flow rate and plunger backdrive, the closed-loop cycle end-to-end on the bench rig including the over-pressure trips, and the full workflow on an ex-vivo pig eye.

What still needs to be tested (Section 6.2.2 and Section 10, Tier 1 and Tier 2). First, the bench rig cannot generate more than 22 mmHg, so calibration above this point needs a controlled syringe-press rig. Second, the device needs validation on cadaveric and in-vivo hypotonous tissue under a formal clinical protocol. Third, IEC 60601-1 environmental and EMC type-testing is outstanding. Fourth, a clinician-in-the-loop usability trial against the integrated single-handpiece build is still to be run.

Scope items set aside (Section 6.2.3). The single-use lockout token, the EMR integration, battery operation, and the full Class IIa regulatory documentation are all out of scope for this prototype. The follow-on tier and the rationale for each are in Section 10.

Over-pressure trip behaviour (Section 6.2.1). The 50 and 60 mmHg trips are anchored at fixed absolute values, not at a relative offset from the running baseline, so the trip thresholds do not shift with any drift in the baseline. The baseline is also re-referenced at every pause. The residual risk is confined to long unpaused holds, which fall outside the intended clinical use of the device. If lifetime drift testing in Section 10, Tier 2 shows it is needed, a periodic in-firmware re-zero against atmospheric pressure during stopcock-closed pauses will be added in a new firmware update.

## Future work

Tier 1, before the Summer Session (Section 10). Full setpoint confirmation at 50 mmHg on the higher-pressure syringe-press rig. Integrated single-handpiece build. Electronic single-use lockout token on the sensor cable. Firmware tagged at v1.0 in the public Git repository with the README, calibration data and risk assessment cross-checked against the Section 8.3 list.

Tier 2, clinical and regulatory pathway (Section 10). ISO 10993 cytotoxicity and sensitisation testing on the wetted fluid path. Sterilisation validation and post-sterilisation calibration shift measurement. Packaging integrity, accelerated ageing, drop testing, and a longer pressure soak test for sensor drift. IEC 60601-1 and IEC 60601-1-2 electrical safety and EMC. IEC 62304 firmware classification and verification. Design history file, clinical evaluation report and ISO 14971 risk file. Cadaveric testing and in-vivo validation with Moorfields.

Tier 3, follow-on student projects (Section 10). In-line or near-tip pressure transducer to replace the hub-mounted MPRLS. Clinician-facing interface with patient-specific pressure profiles. Battery-capable power architecture. Control algorithm extension beyond fixed thresholds, using the pressure-time curve to decide when to pause, resume or stop.

## Where to find things

Final report PDF: not committed to this repository, since it is the Moodle submission. The PDF lives in the team drive copy of the Handover Package, under 10_Final_report. Section numbers used throughout this README and the docs in this repository refer to that PDF.
Pin map: Table 33 in Appendix A.6 of the report, also reproduced above.
BoM, project reference 547914: Appendix B of the report and bom/.
FMEA: Table 29 in Appendix A.3.
PDS compliance: Appendix D.
Standards mapping (ISO 10993, IEC 62304, IEC 60601-1, UK MDR 2002, UK GDPR): Table 40 in Appendix I.
Engineering drawings: Appendix E and drawings/.

## Contact

Continuation channel for future students. The team email alias and the GitHub issue tracker are listed in docs/contact.md. Add yours before the Summer Session so the next team can reach you.

## Data and privacy

No patient-identifiable data is stored on the device or in this repository. Clinical context came from observation at Moorfields Eye Hospital and from discussions with Dr Orsine Murta Dias. Sponsor-supplied parts are listed in the Sponsorship and Test Consumables sub-assembly of the BoM (Appendix B). Before pushing to a public repository, scan the firmware history for WiFi credentials, API keys and personal email addresses.
