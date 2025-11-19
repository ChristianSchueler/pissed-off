#include <hx711.h>
#include <driver/gpio.h>
#include "load-cell.h"

hx711_t dev = {
    .dout = GPIO_NUM_11,
    .pd_sck = GPIO_NUM_11,
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
    printf("hx711: %d\n", data);
}