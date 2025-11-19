#include <stdio.h>
#include <driver/gpio.h>
#include "driver/uart.h"
#include "coin-acceptor.h"

#define COIN_ACCEPTOR_UART_NUM UART_NUM_1
#define COIN_ACCEPTOR_RX_PIN GPIO_NUM_18               // white wire
#define COIN_ACCEPTOR_INHIBIT_GPIO_PIN GPIO_NUM_17     // blue wire
#define BUF_SIZE 256

int coin_acceptor_initialized = 0;              // initialized true/false
int coin_acceptor_amount_cents = 0;             // how much money has been inserted yet
uint8_t coin_acceptor_read_buffer[BUF_SIZE];    // serial RX read buffer, gets filled with data from UART

uart_config_t coin_acceptor_uart_config = {
    .baud_rate = 4800,                          // DG600F coin acceptor default baud rate: 4800
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT
};

// TODO: 
// - set GPIO for enable/disable ("inhibit")
// - set UART
void coin_acceptor_init() {

    ESP_ERROR_CHECK(uart_driver_install(COIN_ACCEPTOR_UART_NUM, BUF_SIZE, 0, 0, NULL, 0));      // not using interrupts
    ESP_ERROR_CHECK(uart_param_config(COIN_ACCEPTOR_UART_NUM, &coin_acceptor_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(COIN_ACCEPTOR_UART_NUM, UART_PIN_NO_CHANGE, COIN_ACCEPTOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    coin_acceptor_initialized = 1;
}

// TODO:
// - read from buffer of data avalaible and parse and update amount
void coin_acceptor_loop() {

    if (!coin_acceptor_initialized) return;

    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(COIN_ACCEPTOR_UART_NUM, (size_t*)&length));
    if (length > 0) {
        length = uart_read_bytes(COIN_ACCEPTOR_UART_NUM, coin_acceptor_read_buffer, length, 100);
        printf("%d bytes received: 0x ", length);
        for (int i=0; i<length; i++) { 
            printf("%x", coin_acceptor_read_buffer[i]); 
            if (i < length-1) printf(" ");

            int coin = coin_acceptor_read_buffer[i];
            if (coin == 1 || coin == 2) {
                coin_acceptor_amount_cents += coin*100;
            }
        }
        printf("\n");
        printf("current amount: %d,%02d EUR\n", coin_acceptor_amount_cents / 100, coin_acceptor_amount_cents % 100);
    }
}

void coin_acceptor_reset_amount() {
    coin_acceptor_amount_cents = 0;
}

int coin_acceptor_get_amount_cents() { 
    return coin_acceptor_amount_cents;
}

void coin_acceptor_enable() {

}

void coin_acceptor_disable() {

}