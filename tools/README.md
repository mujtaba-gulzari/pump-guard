# tools/

## log_capture.py: save serial logs to files

```
pip install pyserial
python tools/log_capture.py --port COM3 --label ranging-20cm
```

Creates `data/<date>_<label>/` with `<label>.log` (raw serial text) and `<label>.csv` (parsed). Close `idf.py monitor` first: only one program can use the port.

### Why this CSV format

- **Long format, one row per measurement:** loads straight into pandas, Excel and plotting tools.
- **Raw and processed side by side** (`echo_us`, `raw_cm`, `corrected_cm`, `median_cm`) so any plot can be regenerated and the compensation can be audited later.
- **`uptime_ms` from the ESP32 plus `pc_time_utc` from the PC:** the first gives exact spacing between samples, the second ties a run to the real clock.
- **Counters (`ok`, `timeout`, `rejected`) in every row:** a drop in quality shows up in the data, not just on the screen.
- **Plain text, comma separated, units in the names, empty cell = not available:** no custom parser needed.
- **The `.log` file is never edited:** it is the evidence; the CSV can always be rebuilt from it.

| Column | Unit | Meaning |
| --- | --- | --- |
| pc_time_utc | ISO 8601 | PC clock when the line arrived |
| uptime_ms | ms | ESP32 time since boot |
| echo_us | us | Echo pulse length, empty if no valid echo |
| raw_cm | cm | Distance at fixed 20 C speed of sound |
| corrected_cm | cm | Distance with temperature compensation |
| temp_c | C | DHT22 temperature, empty until Day 4 is wired |
| median_cm | cm | Output of the 5-sample median filter, empty if rejected |
| ok_count / timeout_count / rejected_count | count | Running totals since boot |

### Using it in firmware

`firmware/main/csv_log.h` is already in the project. In `pump-guard.c`:

```c
#include "csv_log.h"
...
// once per measurement, NAN where a value does not exist yet:
csv_log_measurement(echo_length_us, raw_distance_cm, corrected_distance_cm,
                    current_temperature_c, median_distance_cm,
                    ok_count, timeout_count, rejected_count);
```

Keep the existing `ESP_LOGI` lines too: they go to the `.log` file only.
