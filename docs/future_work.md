# Future work

Reproduced from Section 10 of the final report. Remaining work is grouped into three tiers by horizon and dependency.

## Tier 1: before the Mech Eng Summer Session

Four items close out the prototype as it stands.

First, full setpoint confirmation at 50 mmHg will be carried out on the higher-pressure syringe-press rig referenced from Sections 5.2.4 and 8.1, replacing the 18 mmHg surrogate trip used in the bench tests.

Second, an integrated single-handpiece build will be assembled to address the limitations set out in Section 6.2.1, moving operation from two-handed bench-top to one-handed clinical form-factor.

Third, an electronic single-use lockout token will be prototyped on the sensor cable, closing the auto-disable gap noted in Section 8.3.

Fourth, the firmware will be tagged at v1.0 in the public Git repository and the README, calibration data and risk assessment cross-checked against the handover-package list in Section 8.3.

## Tier 2: clinical and regulatory pathway

Building on Section 8.2, the next stage toward a Class IIa submission would involve four workstreams.

First, the wetted fluid path would require ISO 10993 cytotoxicity and sensitisation testing, together with sterilisation validation using the most appropriate route for the disposable components. Any post-sterilisation calibration shift or material degradation would need to be measured.

Second, packaging integrity, accelerated ageing and drop testing would be required to support the shelf life requirement, while longer duration pressure soak testing would quantify sensor drift over extended use.

Third, the controller would require electrical safety and EMC testing under IEC 60601-1 and IEC 60601-1-2, while the firmware would need classification, documentation and verification under IEC 62304.

Fourth, the design history file, clinical evaluation report and ISO 14971 risk management file would need to be developed, using the FMEA in Table 29 as the starting point. Cadaveric testing and later in-vivo validation with Moorfields Eye Hospital would follow once the technical file and ethics pathway are sufficiently developed. Approval of the Class IIa technical file would be the main gating step before commercial translation.

## Tier 3: research and follow on student projects

Four extensions are suitable for continuation MEng projects.

First, the hub mounted MPRLS could be replaced with an in line or near tip pressure transducer to reduce hydrostatic correction error, improve response time and allow pressure measurement closer to the actual intraocular site.

Second, a clinician facing interface could be developed to display pressure, delivered volume and phase state more clearly, with optional patient specific pressure profiles for future clinical studies.

Third, the power architecture could be redesigned for battery operation, replacing the wall tethered 24 V supply used in the current prototype.

Fourth, the control algorithm could be extended beyond fixed pressure thresholds to investigate tissue relaxation behaviour, using the pressure time curve to decide when to pause, resume or stop injection.
