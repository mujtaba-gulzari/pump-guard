# Pump Guard: requirements

Fill in the empty cells. Rules for a good requirement:

- One sentence, one measurable target, one way to test it.
- Write what the system must do, not how it does it.
- Every row here must be referenced by at least one test in `docs/index.md` section 6.

Priority: **M** = must have, **S** = should have, **C** = could have.

| ID | Requirement (the system shall...) | Target / limit | How it is verified | Test ID | Priority |
| --- | --- | --- | --- | --- | --- |
| R1 | Measure tank water level distance | Error within ±1 cm from 10 to 60 cm | Bench, tape measure + HIL echo injection | T1, T2 | M |
| R2 | Detect pump dry running | | | T7 | M |
| R3 | Survive bad sensor data (no echo, spikes, out of range) | | | T3, T4, T5 | M |
| R4 | Correct for air temperature | | | T6 | S |
| R5 | Default to pump OFF on reset or power loss | | | T8 | M |
| R6 | Run unattended | | | T9 | S |

(R1 is pre-filled from your measured bench result. Everything else is yours to decide.)

## Operating conditions

| Item | Value |
| --- | --- |
| Tank height / usable range | |
| Supply voltage | 12 V SMPS, 5.00 V buck rail |
| Ambient temperature range | |
| Pump (model, rated current) | |
| Update rate of level measurement | 2 Hz (500 ms loop) |

## Out of scope

List what Pump Guard deliberately does not do (for example: mains-voltage pumps, foam detection, remote alerts).

-
-

## Open questions

-
