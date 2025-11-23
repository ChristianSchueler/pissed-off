// (c) 2025, Christian Schüler, hello@christianschueler.at

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

#define COCKTAIL_DONATION_CENTS 300
#define CUP_WEIGHT_MIN_GRAMS 3
#define CUP_WEIGHT_MAX_GRAMS 20
#define COCKTAIL_SIZE_ML 100

enum State state = INSERT_COIN;
int drink_dispensed_ml;
float initial_weight_grams;

void app_main(void)
{
    peristaltic_pump_init();    // first, such that even if everythink else fails, the pump gets stopped
    coin_acceptor_init();
    load_cell_init();

    while (1) {
        peristaltic_pump_loop();
        coin_acceptor_loop();
        load_cell_loop();

        float weight = load_cell_get_last_load_grams();
        int coins = coin_acceptor_get_amount_cents();

        switch (state) {
          case INSERT_COIN:    
            if (coins >= COCKTAIL_DONATION_CENTS && 
                weight >= CUP_WEIGHT_MIN_GRAMS && weight <= CUP_WEIGHT_MAX_GRAMS) {
                  printf("pissed off: coins donated and cup placed -> dispensing");
                  drink_dispensed_ml = 0;
                  initial_weight_grams = weight;
                  set_peristaltic_pump_on();
                  state = DISPENSING;
                }
            break;
          
            // TODO implement TIMEOUT
          case DISPENSING:
            drink_dispensed_ml = (load_cell_get_last_load_grams() - initial_weight_grams);    // assuming ml == grams
            if (drink_dispensed_ml >= COCKTAIL_SIZE_ML) {
              printf("pissed off: cup filled -> take cup");
              set_peristaltic_pump_off();
              state = TAKE_CUP;
            }
            break;

          case TAKE_CUP:
            if (weight < CUP_WEIGHT_MIN_GRAMS) {
              printf("pissed off: cup taken -> insert coin");
              coin_acceptor_reset_amount();
              load_cell_tare();                       // reset load cell every time to account for drift
              state = INSERT_COIN;
            }
            break;

          default:
            break;
        }
        vTaskDelay(MAIN_LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
}