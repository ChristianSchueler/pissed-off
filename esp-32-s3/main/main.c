#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include <driver/gpio.h>

#define PUMP_PIN GPIO_NUM_1

gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << PUMP_PIN),      // Select GPIO 1
    .mode = GPIO_MODE_OUTPUT,                // Set as output
    .pull_up_en = GPIO_PULLUP_DISABLE,       // Disable pull-up
    .pull_down_en = GPIO_PULLDOWN_DISABLE,   // Disable pull-down
    .intr_type = GPIO_INTR_DISABLE           // Disable interrupts
};

void app_main(void)
{
    gpio_config(&io_conf);
    
    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        int onoff = i % 2;
        printf("on/off: %d\n", onoff);
        gpio_set_level(PUMP_PIN, onoff);    // toggle GPIO pin high / low
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}