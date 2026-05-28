# Bubble removal protocol

Reproduced from Section 5.2.2 of the final report. The protocol is run at the start of every session before any reading is taken.

## Procedure

Fluid is drawn through the circuit with the needle submerged, and the T-junction is gently tapped to release any trapped air.

## Why it matters

Trapped air was the dominant source of pressure drift on early builds. Before the protocol was in routine use, drift was about 5 mmHg in 30 seconds. With the bubble-removal step in place, drift falls to 0.5 to 1.0 mmHg over the same window on the final MPRLS0300YG build (Section 5.2.1, drift results). The PDS limit is 0.5 mmHg per minute on a continuous-monitoring window; the residual drift on this build sits above that headline figure but well inside the 3-second inject-pause-measure window the controller actually uses (see docs/known_limitations.md).

## Verification

The fluid circuit was pressurised by hand to about 70 mmHg with the stopcock open and the needle submerged. The stopcock was then closed and the sensor output watched for 30 seconds. No leakage was seen at any junction in any test session, and pressure held steady throughout each 30-second window. The protocol cleared every visible bubble, and once it was in routine use, no bubble-related pressure anomalies appeared. PBS and distilled water behaved the same way, and distilled water was kept on for the sealing tests because it is the more conservative choice. The PDS leak-tightness requirement is met.
