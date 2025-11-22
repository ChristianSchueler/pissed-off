// (c) 2025, Christian Schüeler, hello@christianschueler.at

#include <driver/gpio.h>
#include "peristaltic-pump.h"
#include "coin-acceptor.h"

#define PUMP_PIN GPIO_NUM_1

gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << PUMP_PIN),      // Select GPIO 1
    .mode = GPIO_MODE_OUTPUT,                // Set as output
    .pull_up_en = GPIO_PULLUP_DISABLE,       // Disable pull-up
    .pull_down_en = GPIO_PULLDOWN_DISABLE,   // Disable pull-down
    .intr_type = GPIO_INTR_DISABLE           // Disable interrupts
};

void peristaltic_pump_init() {
    
    gpio_config(&io_conf);
    gpio_set_level(PUMP_PIN, 0);
}

int onoff = 0;

void peristaltic_pump_loop() {
    
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
}
