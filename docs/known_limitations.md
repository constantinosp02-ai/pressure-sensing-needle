# Known limitations

Reproduced from Section 6.2 of the final report.

## Design limitations (Section 6.2.1)

### Sensor placement

The pressure sensor sits 80 mm away and is connected to the tip through a fluid line. The issue is that during assembly the column height cannot be set perfectly. A 5 mm error in the assumed height feeds through to a 0.36 mmHg error in the correction. This still fits inside the 1.25 mmHg error budget, but it takes a meaningful slice of it. The two options are to measure the column height very accurately during assembly, or to redesign the device so the sensor sits directly at the needle tip and no correction is needed. The latter is the in-line micro-cannula transducer route described in Section 10, Tier 3.

### Measurement latency

The device works in cycles of inject, pause, measure. Each cycle takes about three seconds. This is acceptable for the intended procedures, which last from tens of seconds to several minutes. It does mean, however, that the device cannot be used for cases that need continuous high-bandwidth pressure tracking. A sensor placed at the needle tip would allow continuous measurement and remove this limit. This is also covered by the in-line transducer extension in Section 10, Tier 3.

### Sterilisation

Any medical device must be sterilised before clinical use. The three standard methods are autoclave, ethylene oxide gas, and gamma radiation. The materials used in the final device were chosen to be compatible with all three, but no sterilisation cycle has yet been run on a built unit. Heat and radiation can shift sensor calibration, and this effect has not been characterised. An alternative route, used in practice for many non-disposable theatre devices, is to keep the unit itself non-sterile and bring it into theatre inside a sterile drape, with only the disposable fluid path and needle inside the sterile field. This avoids the need to qualify a full sterilisation cycle on the unit, but adds a per-procedure consumable (the drape) and constrains the device form factor. Biocompatibility testing under ISO 10993 is also outstanding. Both are scheduled in Section 10, Tier 2.

### Pressure drift

On the final build the sensor drifted by 0.5 to 1.0 mmHg over a 30 second static hold, which works out to roughly 1 to 2 mmHg per minute. The PDS specifies a maximum drift of 0.5 mmHg per minute, so against a continuous-monitoring reading the device fails this specification. In practice this does not affect clinical performance, for three reasons. First, the device never measures pressure continuously. It uses an inject-pause-measure cycle in which each pause lasts only 3 seconds. Within a window of this length the accumulated drift is below 0.1 mmHg, which is well inside the plus or minus 2 mmHg accuracy budget. The baseline is also re-referenced at every pause, so drift cannot build up between cycles. Second, each measurement is the average of ten samples taken over 50 ms, which smooths out short-term noise in the reading. Third, the over-pressure trips are anchored at fixed absolute values of 50 and 60 mmHg, not at a relative offset from the running baseline. The trip thresholds therefore do not shift with any drift in the baseline. The residual risk is confined to long unpaused holds, which fall outside the intended clinical use of the device. The 200-hour soak test in Section 10, Tier 2 will measure lifetime drift, and if needed a periodic in-firmware re-zero against atmospheric pressure during stopcock-closed pauses will be added in a new firmware update. The PDS row will also be re-stated at the next revision so it reflects the inject-pause-measure measurement window rather than continuous monitoring.

## Testing limitations (Section 6.2.2)

### What has been tested

The current build has been validated for what a TRL (Technology Readiness Level) 4 proof-of-concept needs to cover, which means component and breadboard validation in a laboratory setting. Pressure accuracy was checked against a hydrostatic water column over the 0 to 22 mmHg range, with a maximum error of 1 mmHg (Section 5.2.1). The hydrostatic offset correction was also verified at three different sensor heights (Section 5.2.1). The fluid circuit was checked for leaks and for how well it handles trapped air (Section 5.2.2). The motor and syringe chain was checked for delivered volume, flow rate, and how easily the plunger could be pushed back by pressure in the line (Section 5.2.3). The full closed-loop control cycle was run end-to-end on the bench rig, including the over-pressure trips. Finally, the whole workflow was run on an ex-vivo pig eye to confirm it works in a clinical-style setup (Section 5.3). Clinical context for this workflow came from observing injection procedures at Moorfields Eye Hospital and from discussions with Dr Orsine Murta Dias.

### What still needs to be tested

Four items are flagged as the natural handover to a follow-on team. First, the bench rig used here cannot generate more than 22 mmHg, so calibration above this point needs a controlled syringe-press rig (Section 10, Tier 1). Second, the device needs to be validated on cadaveric and in-vivo hypotonous tissue under a formal clinical protocol (Section 10, Tier 2). Third, the controller needs IEC 60601-1 environmental and EMC type-testing, covering the operating-room temperature range and electromagnetic interference from nearby electrosurgical equipment (Section 10, Tier 2). Fourth, a clinician-in-the-loop usability trial, including the one-handed operation requirement, needs to be run against the integrated handpiece build (Section 10, Tiers 1 and 2).

## Scope limitations (Section 6.2.3)

Four PDS items were deliberately set aside in agreement with the supervisor and the clinical partner, because they belong to product development rather than to the engineering proof-of-concept that this project was scoped to deliver.

### Single-use lockout

The fluid path is supported as a single-use part by the modular split between the disposable wetted assembly and the reusable electronics module (Section 8.3). What was not built is a tamper-evident lockout that would stop the disposable from being reused. That electronic single-use lockout is included in the v1.1 firmware work that needs to be done in Section 10, Tier 1.

### EMR integration

The device was not connected to hospital information systems or electronic medical records. The PDS row on data handling was met by the more conservative route of simply not storing any patient-identifiable data, rather than by building a compliant export interface. A clinician-facing UI and the EMR hooks are scoped as a continuation MEng project in Section 10, Tier 3.

### Battery operation

The device currently runs on a wall-tethered 24 V supply for the motor and a 5 V supply for the logic. A battery-capable version would need the power architecture to be redesigned around the voltage range of the DRV8825 motor driver. This is identified as follow-on student work in Section 10, Tier 3.

### Regulatory documentation

Full regulatory submission documentation for MHRA Class IIa marking was outside the scope of this project. What the report does provide (Section 8.2) is a standards-mapping that identifies which clauses each subsystem will need to satisfy at a future clinical translation. The full design history file and the Class IIa submission itself are the central items of Section 10, Tier 2.

These are scope boundaries rather than design failures, and they form the natural starting point for the future-work plan in Section 10.
