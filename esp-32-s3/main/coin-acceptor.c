// (c) 2025, Christian Schüler, hello@christianschueler.at
//
// - assumes coin acceptor DG600F
// - dip switches: 1 - OFF, 2 - OFF, 3 - ON (RS232), 4 - OFF
// - coins programmed to their "value as 10 Cents", e.g. 1 EUR = number "10" programmed
// - accepts EUR 2, EUR 1, 50 Cents, 20 Cents, 10 Cents (hardcoded)

#include <stdio.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include "driver/uart.h"
#include "coin-acceptor.h"

#define COIN_ACCEPTOR_UART_NUM UART_NUM_1
#define COIN_ACCEPTOR_RX_PIN GPIO_NUM_18               // white wire
#define COIN_ACCEPTOR_INHIBIT_GPIO_PIN GPIO_NUM_17     // blue wire
#define BUF_SIZE 256

bool coin_acceptor_initialized = false;                // true wen ready to be used
int coin_acceptor_amount_cents = 0;                    // how much money has been inserted yet
long coin_acceptor_amount_since_start_cents = 0;       // overall sum of money inserted since start
bool coin_acceptor_enabled = true;                     // by default enabled with dip switch 4 - OFF

uint8_t coin_acceptor_read_buffer[BUF_SIZE];    // serial RX read buffer, gets filled with data from UART

// UART for async serial communication from coin acceptor to esp32
uart_config_t coin_acceptor_uart_config = {
    .baud_rate = 4800,                          // DG600F coin acceptor default baud rate: 4800
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT
};

// GPIO to enable/disable accepting coins
gpio_config_t coin_acceptor_io_conf = {
    .pin_bit_mask = (1ULL << COIN_ACCEPTOR_INHIBIT_GPIO_PIN),
    .mode = GPIO_MODE_OUTPUT,                   // Set as output
    .pull_up_en = GPIO_PULLUP_DISABLE,          // Disable pull-up
    .pull_down_en = GPIO_PULLDOWN_DISABLE,      // Disable pull-down
    .intr_type = GPIO_INTR_DISABLE              // Disable interrupts
};

void coin_acceptor_init() {

    printf("coin acceptor: initializing...");
    ESP_ERROR_CHECK(uart_driver_install(COIN_ACCEPTOR_UART_NUM, BUF_SIZE, 0, 0, NULL, 0));      // not using interrupts
    ESP_ERROR_CHECK(uart_param_config(COIN_ACCEPTOR_UART_NUM, &coin_acceptor_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(COIN_ACCEPTOR_UART_NUM, UART_PIN_NO_CHANGE, COIN_ACCEPTOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    gpio_config(&coin_acceptor_io_conf);
    gpio_set_level(COIN_ACCEPTOR_INHIBIT_GPIO_PIN, coin_acceptor_enabled);

    coin_acceptor_initialized = true;
    printf("coin acceptor: initialized.");
}

void coin_acceptor_loop() {

    if (!coin_acceptor_initialized) return;

    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(COIN_ACCEPTOR_UART_NUM, (size_t*)&length));
    if (length > 0) {
        length = uart_read_bytes(COIN_ACCEPTOR_UART_NUM, coin_acceptor_read_buffer, length, 100);
        printf("coin acceptor: %d bytes received: 0x ", length);
        for (int i=0; i<length; i++) { 
            printf("%x", coin_acceptor_read_buffer[i]); 
            if (i < length-1) printf(" ");

            int coin = coin_acceptor_read_buffer[i];
            // hard-coded list of available coins: 2 EUR, 1 EUR, 50 Cents, 20 Cents, 10 Cents
            // coins are programmed as "amount of 10 cents", e.g. 1 EUR is programmed as "10", 10 Cents as "1"
            // this is because of upper program limit of 100 
            if (coin == 20 || coin == 10 || coin == 5 || coin == 2 || coin == 1) {
                coin_acceptor_amount_cents += coin*10;
                coin_acceptor_amount_since_start_cents += coin*10; 
            }
        }
        printf("\n");
        printf("coin acceptor: current amount: %d,%02d EUR\n", coin_acceptor_amount_cents / 100, coin_acceptor_amount_cents % 100);
        printf("coin acceptor: amount since start: %ld,%02ld EUR\n", coin_acceptor_amount_since_start_cents / 100, coin_acceptor_amount_since_start_cents % 100);
    }
}

void coin_acceptor_reset_amount() {
    coin_acceptor_amount_cents = 0;
}

int coin_acceptor_get_amount_cents() { 
    return coin_acceptor_amount_cents;
}

void coin_acceptor_reset_amount_since_start() {
    coin_acceptor_amount_since_start_cents = 0;
}

int coin_acceptor_get_amount_since_start_cents() { 
    return coin_acceptor_amount_since_start_cents;
}

void coin_acceptor_enable() {
    printf("coin acceptor: enabled");
    coin_acceptor_enabled = true;
    gpio_set_level(COIN_ACCEPTOR_INHIBIT_GPIO_PIN, coin_acceptor_enabled);
}

void coin_acceptor_disable() {
    printf("coin acceptor: disabled");
    coin_acceptor_enabled = false;
    gpio_set_level(COIN_ACCEPTOR_INHIBIT_GPIO_PIN, coin_acceptor_enabled);
}

bool coin_acceptor_get_enabled() {
    return coin_acceptor_enabled;
}