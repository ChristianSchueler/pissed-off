#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_mac.h"
#include <driver/gpio.h>
#include "driver/uart.h"
//#include "esp_err.h"
#include "coin-acceptor.h"
#include "load-cell.h"

#define PUMP_PIN GPIO_NUM_1

gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << PUMP_PIN),      // Select GPIO 1
    .mode = GPIO_MODE_OUTPUT,                // Set as output
    .pull_up_en = GPIO_PULLUP_DISABLE,       // Disable pull-up
    .pull_down_en = GPIO_PULLDOWN_DISABLE,   // Disable pull-down
    .intr_type = GPIO_INTR_DISABLE           // Disable interrupts
};

// uart_config_t uart_config = {
//     .baud_rate = 4800,                       // DG600F coin acceptor default baud rate: 4800
//     .data_bits = UART_DATA_8_BITS,
//     .parity    = UART_PARITY_DISABLE,
//     .stop_bits = UART_STOP_BITS_1,
//     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//     .rx_flow_ctrl_thresh = 0,
//     .source_clk = UART_SCLK_DEFAULT,
//     .flags = {},
// };
// int intr_alloc_flags = 0;                    // interrupt allocation flags

// minumum buffer size: UART_HW_FIFO_LEN(UART_NUM_1) -> 128
// #define BUF_SIZE (1024)

// #define COIN_ACCEPTOR_UART_NUM UART_NUM_1

// uint8_t data[BUF_SIZE];

void app_main(void)
{
    gpio_config(&io_conf);
    gpio_set_level(PUMP_PIN, 0);

    // ESP_ERROR_CHECK(uart_driver_install(COIN_ACCEPTOR_UART_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    // ESP_ERROR_CHECK(uart_param_config(COIN_ACCEPTOR_UART_NUM, &uart_config));
    // ESP_ERROR_CHECK(uart_set_pin(COIN_ACCEPTOR_UART_NUM, GPIO_NUM_17, GPIO_NUM_18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    coin_acceptor_init();
    load_cell_init();

    int onoff = 0;
    while (1) {
        coin_acceptor_loop();
        load_cell_loop();

        if (coin_acceptor_get_amount_cents() >= 300) {
            printf("Paid.\n");
            onoff = !onoff;
            printf("on/off: %d\n", onoff);
            gpio_set_level(PUMP_PIN, onoff);    // toggle GPIO pin high / low
            coin_acceptor_reset_amount();
        }
        //printf("[%d] Hello world!\n", i);
        //int onoff = i % 2;
        //printf("on/off: %d\n", onoff);
        //gpio_set_level(PUMP_PIN, onoff);    // toggle GPIO pin high / low
        //i++;

        // int length = 0;
        // ESP_ERROR_CHECK(uart_get_buffered_data_len(COIN_ACCEPTOR_UART_NUM, (size_t*)&length));
        // if (length > 0) {
        //     length = uart_read_bytes(COIN_ACCEPTOR_UART_NUM, data, length, 100);
        //     printf("%d bytes received: 0x ", length);
        //     for (int i=0; i<length; i++) { 
        //         printf("%x", data[i]); 
        //         if (i < length-1) printf(" ");
        //     }
        //     printf("\n");
        // }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}