# ADR 001: median of 5 and a 300 to 25,000 us validity window

**Status:** accepted (proposed by Claude, reviewed by Mujtaba)

## Context

The HC-SR04 occasionally returns a stray echo or no echo. A single bad reading must not move the level value or trigger the pump logic.

## Decision

- Take 5 samples and output the **median**.
- Reject any echo shorter than 300 us (about 5 cm) or longer than 25,000 us (about 4.3 m) before it reaches the filter.

## Alternatives considered

| Option | Why not chosen |
| --- | --- |
| Mean of N | One outlier shifts the result. |
| Single reading, no filter | A stray echo moves the level and can false-trigger the pump logic (R3). |
| Moving average of 5 | One outlier still pulls the average; a median ignores it. Keep in mind for smoothing later. |
| Median of 3 | Cheaper, but tolerates only 1 bad sample out of 3; 5 tolerates 2 out of 5 for 2 more samples. |

## Rationale for the limits

- 300 us is about 5 cm, near the HC-SR04's minimum range; shorter echoes are ringing of the transducer, not water.
- 25,000 us is about 4.3 m, beyond the sensor's 4 m rated range.
- A reading is trusted only if at least 3 of 5 samples are valid; otherwise the output is 'no valid reading' (counted, never guessed).

## Consequences

- Targets closer than about 5 cm are not measured (acceptable: the tank is not filled to within 5 cm of the sensor; see requirements, operating conditions).
- Output latency is 5 samples x 60 ms + loop delay, about 0.5 s per filtered reading. This is well inside the 2 s dry-run budget (R2).
