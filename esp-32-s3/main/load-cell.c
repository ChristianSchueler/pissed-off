// (c) 2025, Christian Schüeler, hello@christianschueler.at
//
// - load cell using HV711 chip
// - measure the raw data for a known weight and fill in SCALE_FACTOR

#include <hx711.h>
#include <driver/gpio.h>
#include "load-cell.h"
#include "esp_log.h"

#define LOAD_CELL_DT_GPIO_PIN GPIO_NUM_4              // green wire: pin for incoming data signal from hx711
#define LOAD_CELL_SCK_GPIO_PIN GPIO_NUM_5             // yellow wire: pin for outgoing clock signal to hx711

#define WAIT_READ_TIMEOUT_MS 500
#define READ_AVERAGE_SAMPLES 5
#define LOAD_CELL_INVALID_VALUE -1000000

#define SCALE_FACTOR 567927/500                       // hx711 raw reading when loaded with 500 g

int32_t load_cell_offset;
float load_cell_scale;

float load_cell_last_load;

hx711_t dev = {
    .dout = LOAD_CELL_DT_GPIO_PIN,
    .pd_sck = LOAD_CELL_SCK_GPIO_PIN,
    .gain = HX711_GAIN_A_64
};

void load_cell_init() {

    ESP_ERROR_CHECK(hx711_init(&dev));
    
    load_cell_offset = 0;
    load_cell_scale = SCALE_FACTOR;
    load_cell_last_load = LOAD_CELL_INVALID_VALUE;

    load_cell_tare();
}

void load_cell_loop() {
    
    load_cell_last_load = load_cell_get_value_grams();
    printf("load cell: weight: %.1f g\n", load_cell_last_load);
}

void load_cell_tare() {
    load_cell_offset = load_cell_get_value_raw();
}

int32_t load_cell_get_value_raw() {

    // wait until chip is ready with timeout
    esp_err_t r = hx711_wait(&dev, WAIT_READ_TIMEOUT_MS);
    if (r != ESP_OK)
    {
        ESP_LOGE("hx711", "Device not found: %d (%s)\n", r, esp_err_to_name(r));
        return LOAD_CELL_INVALID_VALUE;
    }

    int32_t data;
    r = hx711_read_average(&dev, READ_AVERAGE_SAMPLES, &data);
    if (r != ESP_OK)
    {
        ESP_LOGE("hx711", "Could not read data: %d (%s)\n", r, esp_err_to_name(r));
        return LOAD_CELL_INVALID_VALUE;
    }

    ESP_LOGI("load cell: hx711", "Raw data: %", data);     // PRIi32?
    printf("load cell: hx711 raw data %ld\n", data);

    return data;
}

float load_cell_get_value_grams() {
    
    int32_t data = load_cell_get_value_raw();

    // account for offset and scale factor
    int32_t offsetData = data-load_cell_offset;
    float scaleData = offsetData / load_cell_scale;

    return scaleData;
}