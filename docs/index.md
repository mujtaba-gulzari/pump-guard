# Pump Guard: tank level sensing and dry-run protection on ESP32

> **Status:** work in progress. Sections marked *planned* are not built yet.

<!-- HERO: 45 s overview video goes here (YouTube iframe) -->

**Key results** (fill in with measured numbers only)

| Accuracy | Dry-run cut-off time | Automated tests |
| --- | --- | --- |
| ±1 cm, 10 to 60 cm (bench) | *planned* | *planned* |

## Abstract

<!-- 4 sentences, about 80 words: problem, approach, key result, what is validated. Write this last. -->

## 1. Problem and requirements

<!-- Why dry running damages pumps, who it affects. Then the table from docs/requirements.md (copy or link). -->

See [requirements](requirements.md).

## 2. System overview

<!-- Block diagram (SVG in docs/assets/), parts table with pins, power tree. -->

## 3. Physics and signal conditioning

### 3.1 Ranging and temperature compensation

```text
d = c(T) * t / 2,   c(T) = 331.3 + 0.606 * T   [m/s, T in deg C]
```

<!-- Error budget table, raw vs corrected plot. -->

### 3.2 Input conditioning

<!-- 1 k / 2.2 k echo divider: 5 V * 2.2 / 3.2 = 3.44 V. Include the wiring fault story. -->

### 3.3 Filtering and validity

<!-- Median of 5, plausibility window 300 to 25000 us, ok / timeout / rejected counters. -->

### 3.4 Dry-run detection (*planned*)

<!-- Current sensor + level, threshold-plus-timer rule. Model only if it beats the rule on data. -->

### 3.5 Footprint

<!-- Flash and RAM from `idf.py size`, loop time, current draw. -->

## 4. Firmware architecture

<!-- Tasks, ISR to task hand-off, pump state machine diagram. -->

## 5. Validation

### 5.1 Test setup and HIL rig (*planned*)

### 5.2 Test matrix and results

| ID | Requirement | Stimulus | Pass criterion | Measured | Method |
| --- | --- | --- | --- | --- | --- |
| T1 | R1 | Real target at 10, 20, 30, 60 cm | Error ≤ ±1 cm | | Bench |

### 5.3 Latency and accuracy plots

## 6. Limitations and future work

-
-

## 7. Reproduce it

```text
cd firmware
idf.py set-target esp32
idf.py -p COMx flash monitor
```

## 8. References

- HC-SR04 datasheet
- DHT22 / AM2302 datasheet
- ESP-IDF programming guide
