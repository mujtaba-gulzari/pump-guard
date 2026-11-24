#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#define TRIG_GPIO GPIO_NUM_14
#define ECHO_GPIO GPIO_NUM_27
static const char *TAG = "hcsr04";

// Returns distance in cm, or -1.0 if no echo came back.
static float measure_cm(void)
{
    // 1. Send a 10 microsecond pulse on TRIG
    gpio_set_level(TRIG_GPIO, 0);
    esp_rom_delay_us(2);
    gpio_set_level(TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_GPIO, 0);

    // 2. Wait for ECHO to go high (start of the echo), with a 30 ms timeout
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(ECHO_GPIO) == 0) {
        if (esp_timer_get_time() - start > 30000) {
            return -1.0f;
        }
    }
    int64_t rise = esp_timer_get_time();

    // 3. Wait for ECHO to go low (end of the echo), with a 30 ms timeout
    while (gpio_get_level(ECHO_GPIO) == 1) {
        if (esp_timer_get_time() - rise > 30000) {
            return -1.0f;
        }
    }
    int64_t echo_us = esp_timer_get_time() - rise;
    if (echo_us < 300) return -1.0f; // ignore very short pulses, which are probably noise

    // 4. Convert the pulse length to distance: about 58 us per cm round trip
    return echo_us / 58.0f;
}

void app_main(void)
{
    gpio_reset_pin(TRIG_GPIO);
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG_GPIO, 0);

    gpio_reset_pin(ECHO_GPIO);
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);

    while (1) {
        float cm = measure_cm();
        if (cm < 0) {
            ESP_LOGW(TAG, "no echo");
        } else {
            ESP_LOGI(TAG, "distance: %.1f cm", cm);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}