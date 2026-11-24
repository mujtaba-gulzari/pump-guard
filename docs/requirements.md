# Pump Guard: requirements

Status: **proposed values, written by Claude for Mujtaba to review.** Items marked *(confirm)* depend on your real tank and pump: replace them with your own numbers. Every other value is a design target that the tests in `docs/index.md` section 5 will prove or disprove, so change a target only if you change the test too.

Rules for a good requirement:

- One sentence, one measurable target, one way to test it.
- Write what the system must do, not how it does it.
- Every row must be referenced by at least one test in `docs/index.md` section 5.

Priority: **M** = must have, **S** = should have, **C** = could have.

| ID | Requirement (the system shall...) | Target / limit | How it is verified | Test ID | Priority |
| --- | --- | --- | --- | --- | --- |
| R1 | Measure the distance from the sensor to the water surface | Error within ±1 cm from 10 to 60 cm, at 20 to 30 °C | Bench with tape measure (done: about ±1 cm); HIL injects echoes of known length | T1, T2 | M |
| R2 | Detect pump dry running and switch the pump off | Relay opens within 2 s of the dry-run condition starting; no false trip in 24 h of normal running | HIL replays a current waveform that drops to the dry-run level; soak test | T7, T9 | M |
| R3 | Stay correct and keep running when sensor data is bad (no echo, spike, out of range) | No crash or reboot; one bad sample out of 5 changes the output by less than 0.5 cm; every bad sample is counted | Fault injection: unplug echo, inject spikes and out-of-window pulses | T3, T4, T5 | M |
| R4 | Correct the distance for air temperature | Corrected result differs from the uncorrected one by the formula's amount ±0.2 cm between 10 and 35 °C; falls back to the last good temperature if the DHT22 fails | Same echo length at two temperatures; unplug DHT22 | T6 | S |
| R5 | Default to pump OFF on reset, brown-out or firmware fault | Relay is open within 100 ms of boot and stays open until a valid reading exists *(confirm 100 ms with a scope or logic analyser)* | Reset and power-cycle the DUT during a run, watch the relay pin | T8 | M |
| R6 | Run unattended | 24 h continuous with 0 crashes and a timeout rate under 0.1 % | 24 h soak, CSV log of counters | T9 | S |

## Operating conditions

| Item | Value |
| --- | --- |
| Tank height / usable range | Sensor range used: 10 to 60 cm below the sensor. Real tank height: *(confirm: measure your tank)* |
| Supply voltage | 12 V SMPS, 5.00 V buck rail for the Feather and sensors, 3.3 V logic |
| Ambient temperature range | 10 to 40 °C design range (DHT22 works from −40 to 80 °C); only 20 to 30 °C is tested so far *(confirm what your bench reaches)* |
| Pump (model, rated current) | 12 V DC pump *(confirm: model and the current it draws with water and dry; measure with a meter before choosing the R2 threshold)* |
| Update rate of level measurement | About 2 Hz: 5 samples x 60 ms spacing + 200 ms delay per filtered reading |
| Pump switching | SRD-05VDC relay through a transistor driver, 12 V only (no mains) |

## Out of scope

- Mains-voltage pumps (needs a contactor and an electrician).
- Foam, waves or floating debris on the water surface: ultrasonic ranging can fail on them. Mentioned as a limitation.
- Wi-Fi, phone alerts and cloud logging.
- Measuring volume or flow; only level and dry-run protection.

## Open questions

- What are the real tank height and the pump's current when pumping water versus dry? These set the R2 threshold (plan: dry-run threshold at about 60 % of the wet current, decided from measurements).
- Do you want a manual override switch? (Not required now. Add as R7 if yes.)
- Is 100 ms enough for R5 on your relay driver? Measure it.
