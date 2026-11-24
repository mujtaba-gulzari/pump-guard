#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#define TRIG_GPIO GPIO_NUM_14
#define ECHO_GPIO GPIO_NUM_27
static const char *TAG = "hcsr04";

static volatile int64_t s_rise_us = 0;     // time the echo started (0 = none yet)
static volatile int64_t s_echo_us = 0;     // length of the last echo
static SemaphoreHandle_t s_done;           // the "doorbell" the ISR rings

// Runs automatically every time ECHO changes (rises or falls).
static void IRAM_ATTR echo_isr(void *arg)
{
    int64_t now = esp_timer_get_time();

    if (gpio_get_level(ECHO_GPIO)) {
        s_rise_us = now;                         // echo started: remember when
    } else if (s_rise_us != 0) {
        s_echo_us = now - s_rise_us;             // echo ended: work out how long
        s_rise_us = 0;
        BaseType_t woken = pdFALSE;
        xSemaphoreGiveFromISR(s_done, &woken);   // ring the doorbell
        portYIELD_FROM_ISR(woken);
    }
}

static void hcsr04_init(void)
{
    gpio_reset_pin(TRIG_GPIO);
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG_GPIO, 0);

    gpio_config_t echo_cfg = {
        .pin_bit_mask = 1ULL << ECHO_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,          // interrupt on rising AND falling
    };
    gpio_config(&echo_cfg);

    s_done = xSemaphoreCreateBinary();
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ECHO_GPIO, echo_isr, NULL);
}

static float measure_cm(void)
{
    xSemaphoreTake(s_done, 0);                   // clear any old doorbell ring
    s_rise_us = 0;

    gpio_set_level(TRIG_GPIO, 0);
    esp_rom_delay_us(2);
    gpio_set_level(TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_GPIO, 0);

    // Sleep until the ISR rings the doorbell, or give up after 60 ms.
    if (xSemaphoreTake(s_done, pdMS_TO_TICKS(60)) != pdTRUE) {
        return -1.0f;
    }

    int64_t echo_us = s_echo_us;
    if (echo_us < 300 || echo_us > 25000) {
        return -1.0f;
    }
    return echo_us / 58.0f;
}

void app_main(void)
{
    hcsr04_init();

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