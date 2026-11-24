# ADR 001: median of 5 and a 300 to 25,000 us validity window

**Status:** draft (fill in the blanks)

## Context

The HC-SR04 occasionally returns a stray echo or no echo. A single bad reading must not move the level value or trigger the pump logic.

## Decision

- Take 5 samples and output the **median**.
- Reject any echo shorter than 300 us (about 5 cm) or longer than 25,000 us (about 4.3 m) before it reaches the filter.

## Alternatives considered

| Option | Why not chosen |
| --- | --- |
| Mean of N | One outlier shifts the result. |
| | |

## Consequences

- Targets closer than about 5 cm are not measured (acceptable: the tank never gets that full, to be confirmed in requirements).
- Output latency is 5 samples x measurement period.
