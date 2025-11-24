// /* LEDC (LED Controller) basic example

//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */
// #include <stdio.h>
// #include "driver/ledc.h"
// #include "esp_err.h"
// #include "sdkconfig.h"
// #include "esp_pm.h"

// #define LEDC_TIMER              LEDC_TIMER_0
// #define LEDC_MODE               LEDC_LOW_SPEED_MODE
// #define LEDC_OUTPUT_IO          (2) // Define the output GPIO
// #define LEDC_CHANNEL            LEDC_CHANNEL_0
// #define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
// #define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
// #if CONFIG_PM_ENABLE
// #define LEDC_CLK_SRC            LEDC_USE_RC_FAST_CLK // choose a clock source that can maintain during light sleep
// #define LEDC_FREQUENCY          (400) // Frequency in Hertz. Set frequency at 400 Hz
// #else
// #define LEDC_CLK_SRC            LEDC_AUTO_CLK
// #define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz
// #endif

// /* Warning:
//  * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2 (rev < 1.2), ESP32P4 (rev < 3.0) targets,
//  * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
//  * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
//  */

// static void example_ledc_init(void)
// {
//     // Prepare and then apply the LEDC PWM timer configuration
//     ledc_timer_config_t ledc_timer = {
//         .speed_mode       = LEDC_MODE,
//         .duty_resolution  = LEDC_DUTY_RES,
//         .timer_num        = LEDC_TIMER,
//         .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
//         .clk_cfg          = LEDC_CLK_SRC,
//     };
//     ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

//     // Prepare and then apply the LEDC PWM channel configuration
//     ledc_channel_config_t ledc_channel = {
//         .speed_mode     = LEDC_MODE,
//         .channel        = LEDC_CHANNEL,
//         .timer_sel      = LEDC_TIMER,
//         .gpio_num       = LEDC_OUTPUT_IO,
//         .duty           = 0, // Set duty to 0%
//         .hpoint         = 0,
// #if CONFIG_PM_ENABLE
//         .sleep_mode     = LEDC_SLEEP_MODE_KEEP_ALIVE,
// #endif
//     };
//     ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
// }

// void app_main(void)
// {
// #if CONFIG_PM_ENABLE
//     esp_pm_config_t pm_config = {
//         .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
//         .min_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
// #if CONFIG_FREERTOS_USE_TICKLESS_IDLE
//         .light_sleep_enable = true
// #endif
//     };
//     ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
// #endif
//     // Set the LEDC peripheral configuration
//     example_ledc_init();
//     // Set duty to 50%
//     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
//     // Update duty to apply the new value
//     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
// }


// ---------------------------------

// /*
//  * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: Unlicense OR CC0-1.0
//  */
// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "led_strip.h"
// #include "esp_log.h"
// #include "esp_err.h"

// // Set to 1 to use DMA for driving the LED strip, 0 otherwise
// // Please note the RMT DMA feature is only available on chips e.g. ESP32-S3/P4
// #define LED_STRIP_USE_DMA  0

// #if LED_STRIP_USE_DMA
// // Numbers of the LED in the strip
// #define LED_STRIP_LED_COUNT 256
// #define LED_STRIP_MEMORY_BLOCK_WORDS 1024 // this determines the DMA block size
// #else
// // Numbers of the LED in the strip
// #define LED_STRIP_LED_COUNT 24
// #define LED_STRIP_MEMORY_BLOCK_WORDS 0 // let the driver choose a proper memory block size automatically
// #endif // LED_STRIP_USE_DMA

// // GPIO assignment
// #define LED_STRIP_GPIO_PIN  2

// // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
// #define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

// static const char *TAG = "example";

// led_strip_handle_t configure_led(void)
// {
//     // LED strip general initialization, according to your led board design
//     led_strip_config_t strip_config = {
//         .strip_gpio_num = LED_STRIP_GPIO_PIN, // The GPIO that connected to the LED strip's data line
//         .max_leds = LED_STRIP_LED_COUNT,      // The number of LEDs in the strip,
//         .led_model = LED_MODEL_WS2812,        // LED strip model
//         .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
//         .flags = {
//             .invert_out = false, // don't invert the output signal
//         }
//     };

//     // LED strip backend configuration: RMT
//     led_strip_rmt_config_t rmt_config = {
//         .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
//         .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
//         .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS, // the memory block size used by the RMT channel
//         .flags = {
//             .with_dma = LED_STRIP_USE_DMA,     // Using DMA can improve performance when driving more LEDs
//         }
//     };

//     // LED Strip object handle
//     led_strip_handle_t led_strip;
//     ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
//     ESP_LOGI(TAG, "Created LED strip object with RMT backend");
//     return led_strip;
// }

// void app_main(void)
// {
//     led_strip_handle_t led_strip = configure_led();
//     bool led_on_off = false;

//     ESP_LOGI(TAG, "Start blinking LED strip");
//     while (1) {
//         if (led_on_off) {
//             /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
//             for (int i = 0; i < LED_STRIP_LED_COUNT; i++) {
//                 ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 5, 5, 5));
//             }
//             /* Refresh the strip to send data */
//             ESP_ERROR_CHECK(led_strip_refresh(led_strip));
//             ESP_LOGI(TAG, "LED ON!");
//         } else {
//             /* Set all LED off to clear all pixels */
//             ESP_ERROR_CHECK(led_strip_clear(led_strip));
//             ESP_LOGI(TAG, "LED OFF!");
//         }

//         led_on_off = !led_on_off;
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }
