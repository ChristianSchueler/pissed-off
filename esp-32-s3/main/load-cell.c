#include <hx711.h>
#include <driver/gpio.h>
#include "load-cell.h"
#include "esp_log.h"

#define LOAD_CELL_DT_GPIO_PIN GPIO_NUM_4              // green wire: pin for incoming data signal from hx711
#define LOAD_CELL_SCK_GPIO_PIN GPIO_NUM_5             // yellow wire: pin for outgoing clock signal to hx711

hx711_t dev = {
    .dout = LOAD_CELL_DT_GPIO_PIN,
    .pd_sck = LOAD_CELL_SCK_GPIO_PIN,
    .gain = HX711_GAIN_A_64
};

void load_cell_init() {

    ESP_ERROR_CHECK(hx711_init(&dev));
}

void load_cell_loop() {
    
    esp_err_t r = hx711_wait(&dev, 500);
    if (r != ESP_OK)
    {
        ESP_LOGE("hx711", "Device not found: %d (%s)\n", r, esp_err_to_name(r));
        return;
    }

    int32_t data;
    r = hx711_read_average(&dev, 5, &data);     // sample 5 inputs for average. TODO: config value
    if (r != ESP_OK)
    {
        ESP_LOGE("hx711", "Could not read data: %d (%s)\n", r, esp_err_to_name(r));
        return;
    }

    ESP_LOGI("hx711", "Raw data: %", data);     // PRIi32?
    printf("hx711: %ld\n", data);
}