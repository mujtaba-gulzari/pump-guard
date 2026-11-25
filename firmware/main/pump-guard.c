#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "csv_log.h"   // prints the DATA, line that tools/log_capture.py saves as CSV
#include "dht.h"

#define TRIG_GPIO GPIO_NUM_14
#define ECHO_GPIO GPIO_NUM_27
#define DHT_GPIO GPIO_NUM_25
#define DHT_READ_EVERY_N_LOOPS 4     // loop runs every 500 ms, so 4 loops = 2 seconds between DHT reads
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

// Returns the echo length in microseconds, or -1 if no valid echo came back.
static int64_t measure_echo_us(void)
{
    xSemaphoreTake(s_done, 0);       // clear any old doorbell ring
    s_rise_us = 0;

    gpio_set_level(TRIG_GPIO, 0);
    esp_rom_delay_us(2);
    gpio_set_level(TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_GPIO, 0);

    if (xSemaphoreTake(s_done, pdMS_TO_TICKS(60)) != pdTRUE) {
        return -1;
    }

    int64_t echo_length_us = s_echo_us;
    if (echo_length_us < 300 || echo_length_us > 25000) {
        return -1;
    }
    return echo_length_us;
}

// Speed of sound in metres per second, for a temperature in degrees Celsius.
static float speed_of_sound_mps(float temperature_c)
{
    return 331.3f + 0.606f * temperature_c;
}

// Converts an echo time into centimetres.
// The sound travels there and back, so we divide by 2.
// Microseconds to seconds is ÷1,000,000; metres to centimetres is ×100.
static float echo_to_cm(int64_t echo_length_us, float speed_mps)
{
    return (float)echo_length_us * speed_mps * 100.0f / 2.0f / 1000000.0f;
}

#define SAMPLES_PER_READING 5

// Takes 5 readings and returns the median of the good ones.
static float measure_filtered(void)
{
    float good_readings[SAMPLES_PER_READING];   // the readings that worked
    int good_count = 0;                         // how many good readings we have so far

    for (int attempt = 0; attempt < SAMPLES_PER_READING; attempt++) {
        float distance_cm = measure_echo_us();
        if (distance_cm > 0) {
            good_readings[good_count] = distance_cm;   // store it in the next free slot
            good_count = good_count + 1;
        }
        vTaskDelay(pdMS_TO_TICKS(60));                 // let old echoes die out
    }

    if (good_count < 3) {
        return -1.0f;                                  // not enough good readings to trust
    }

    // Sort the good readings, smallest first. Think of sorting playing cards in your
    // hand: pick up the next card and slide it left past every card that is bigger.
    for (int next_card = 1; next_card < good_count; next_card++) {
        float card_in_hand = good_readings[next_card];
        int slot = next_card - 1;
        while (slot >= 0 && good_readings[slot] > card_in_hand) {
            good_readings[slot + 1] = good_readings[slot];   // move the bigger one right
            slot = slot - 1;
        }
        good_readings[slot + 1] = card_in_hand;              // drop the card in its place
    }

    int middle = good_count / 2;
    return good_readings[middle];                      // the median
}

void app_main(void)
{
    hcsr04_init();

    float current_temperature_c = 20.0f;   // used until the first DHT22 read works
    int loops_since_dht_read = DHT_READ_EVERY_N_LOOPS;   // read on the first loop

    while (1) {
        // Read the DHT22 every 2 seconds, not every loop.
        if (loops_since_dht_read >= DHT_READ_EVERY_N_LOOPS) {
            float humidity_percent = 0.0f;
            float new_temperature_c = 0.0f;
            esp_err_t dht_result = dht_read_float_data(
                DHT_TYPE_AM2301,          // the DHT22 uses the AM2301 protocol
                DHT_GPIO,
                &humidity_percent,
                &new_temperature_c);

            if (dht_result == ESP_OK
                && new_temperature_c > -20.0f && new_temperature_c < 60.0f
                && !(new_temperature_c == 0.0f && humidity_percent == 0.0f)) 
            {
                current_temperature_c = new_temperature_c;
            } else {
                ESP_LOGW(TAG, "DHT22 reading rejected (%s), still using %.1f C",
                esp_err_to_name(dht_result), current_temperature_c);
            }
            loops_since_dht_read = 0;
        }
        loops_since_dht_read++;

        int64_t echo_length_us = measure_echo_us();
        if (echo_length_us < 0) {
            ESP_LOGW(TAG, "no echo");
        } else {
            float raw_distance_cm = echo_to_cm(echo_length_us, 343.4f);   // fixed 20 °C speed
            float corrected_distance_cm = echo_to_cm(echo_length_us,
                                                     speed_of_sound_mps(current_temperature_c));
            ESP_LOGI(TAG, "raw %.1f cm | corrected %.1f cm | temp %.1f C",
                     raw_distance_cm, corrected_distance_cm, current_temperature_c);
                     // One CSV row per filtered reading. NAN / -1 = value not available yet
                     // (raw, corrected and temperature will be filled in on Day 4 with the DHT22).
                    //  float median_distance_cm = (cm < 0) ? NAN : cm;
                     // csv_log_measurement(-1, NAN, NAN, NAN, median_distance_cm,
                     //                     (uint32_t)s_ok, (uint32_t)s_timeout, (uint32_t)s_rejected);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

