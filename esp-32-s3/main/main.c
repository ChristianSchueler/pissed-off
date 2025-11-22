// (c) 2025, Christian Schüeler, hello@christianschueler.at

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "driver/uart.h"
#include "peristaltic-pump.h"
#include "coin-acceptor.h"
#include "load-cell.h"

enum State {
  INSERT_COIN,
  DISPENSING,
  TAKE_CUP
};

// how much we intentionally slow down the main loop
#define MAIN_LOOP_DELAY_MS 100

void app_main(void)
{
    peristaltic_pump_init();
    coin_acceptor_init();
    load_cell_init();

    while (1) {

        peristaltic_pump_loop();
        coin_acceptor_loop();
        load_cell_loop();

        vTaskDelay(MAIN_LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
}