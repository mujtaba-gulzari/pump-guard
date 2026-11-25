// Prints one machine-readable measurement line that tools/log_capture.py saves to CSV.
// Format (one line, comma separated, empty field = value not available):
//   DATA,uptime_ms,echo_us,raw_cm,corrected_cm,temp_c,median_cm,ok_count,timeout_count,rejected_count
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "esp_timer.h"

// Prints a float with 2 decimals, or nothing if it is NaN (value unavailable).
static inline void csv_print_float(float value)
{
    if (!isnan(value)) {
        printf("%.2f", value);
    }
}

static inline void csv_log_measurement(int64_t echo_us,
                                       float raw_cm,
                                       float corrected_cm,
                                       float temperature_c,
                                       float median_cm,
                                       uint32_t ok_count,
                                       uint32_t timeout_count,
                                       uint32_t rejected_count)
{
    printf("DATA,%lld,", (long long)(esp_timer_get_time() / 1000));
    if (echo_us >= 0) {
        printf("%lld", (long long)echo_us);
    }
    printf(",");
    csv_print_float(raw_cm);       printf(",");
    csv_print_float(corrected_cm); printf(",");
    csv_print_float(temperature_c);printf(",");
    csv_print_float(median_cm);    printf(",");
    printf("%lu,%lu,%lu\n", (unsigned long)ok_count,
           (unsigned long)timeout_count, (unsigned long)rejected_count);
}
