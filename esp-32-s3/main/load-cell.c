// (c) 2025, Christian Schüler, hello@christianschueler.at
//
// - load cell using HV711 chip
// - measure the raw data for a known weight and fill in SCALE_FACTOR

#include <hx711.h>
#include <driver/gpio.h>
#include "load-cell.h"
#include "esp_log.h"

//#define DEBUG_PRINTF 1

#define LOAD_CELL_DT_GPIO_PIN GPIO_NUM_4              // green wire: pin for incoming data signal from hx711
#define LOAD_CELL_SCK_GPIO_PIN GPIO_NUM_5             // yellow wire: pin for outgoing clock signal to hx711

#define WAIT_READ_TIMEOUT_MS 500
#define READ_AVERAGE_SAMPLES 5
#define LOAD_CELL_INVALID_VALUE -1000000

// 70623
#define SCALE_FACTOR 500. / (531848 - 74670)              // m / (x1 - x0), x0 = hx711 raw reading when loaded with 0 g, x1 = raw reading with 500 g

// internal use for calibration
int32_t load_cell_offset;
float load_cell_scale;

// last measured load
float load_cell_last_load_grams;

hx711_t dev = {
    .dout = LOAD_CELL_DT_GPIO_PIN,
    .pd_sck = LOAD_CELL_SCK_GPIO_PIN,
    .gain = HX711_GAIN_A_64
};

void load_cell_init() {

    printf("load cell: initializing...\n");
    ESP_ERROR_CHECK(hx711_init(&dev));
    
    load_cell_offset = 0;
    load_cell_scale = SCALE_FACTOR;
    load_cell_last_load_grams = LOAD_CELL_INVALID_VALUE;

    // reset offset to current value (make sure load cell has no extra weight!)
    load_cell_tare();

    printf("load cell: initialized.\n");
}

void load_cell_loop() {
    
    load_cell_last_load_grams = load_cell_read_value_grams();
    #ifdef DEBUG_PRINTF
    printf("load cell: weight: %.1f g\n", load_cell_last_load_grams);
    #endif
}

void load_cell_tare() {
    printf("load cell: tare\n");
    
    load_cell_offset = load_cell_read_value_raw();
    // re-calculate scale due to different offset (and assume the 500 g weight measurement stays the same, which it doesn't, but hey)
    load_cell_scale = 500. / (531848 - load_cell_offset);

    printf("load cell: load_cell_offset=%ld\n", load_cell_offset);
    printf("load cell: load_cell_scale=%f\n", load_cell_scale);
}

int32_t load_cell_read_value_raw() {

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

    #ifdef DEBUG_PRINTF
    //ESP_LOGI("load cell: hx711", "Raw data: %", data);     // PRIi32?
    printf("DEBUG - load cell: hx711 raw data %ld\n", data);
    #endif

    return data;
}

float load_cell_read_value_grams() {
    
    int32_t data = load_cell_read_value_raw();

    // account for offset and scale factor
    // basically: m / (x1 - x0) * (x - x0)
    int32_t offsetData = data - load_cell_offset;         // account for load cell offset (= measurement with 0 g load)
    float scaleData = offsetData * load_cell_scale;       // scale factor (depends on two measurements)

    return scaleData;
}

float load_cell_get_last_load_grams() {
    return load_cell_last_load_grams;
}