// (c) 2025, Christian Schüler, hello@christianschueler.at

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "driver/uart.h"
#include "peristaltic-pump.h"
#include "coin-acceptor.h"
#include "load-cell.h"
#include <esp_timer.h>

#define VERSION 0.8

enum State {
  INSERT_COIN,
  DISPENSING,
  TAKE_CUP
};

#define MAIN_LOOP_DELAY_MS 100                // how much we intentionally slow down the main loop
#define COCKTAIL_DONATION_CENTS 300           // how much to donate at least
#define COCKTAIL_SIZE_ML 100                  // cocktail size for donation
#define CUP_WEIGHT_MIN_GRAMS 3                // minimum empty cup weight, used to identify placement of a cup
#define CUP_WEIGHT_MAX_GRAMS 20               // maximum empty cup weight
#define DISPENSING_TIMEOUT_MS 1000*20         // after dispensing time out we stop; most probably empty supply

enum State state = INSERT_COIN;

float initial_weight_grams;                   // remember weight when starting to fill the cup. most probably very small
uint64_t dispensing_started_time_ms;          // when we started to dispense; used for timeout

void app_main(void)
{
    printf("Pissed Off - Mechatronic Interactive Cocktail Maschine, (c) 2025 Christian Schüler, hello@christianschueler.at\n");
    printf("Version: %1.1f\n", VERSION);

    printf("application: starting initialization...\n");
    peristaltic_pump_init();    // first, such that even if everythink else fails, the pump gets stopped
    coin_acceptor_init();
    load_cell_init();

    printf("application: initialized.\n");

    printf("starting main loop, tick duration: %d ms.\n", MAIN_LOOP_DELAY_MS);
    printf("pissed off: insert coin\n");

    while (1) {
        peristaltic_pump_loop();
        coin_acceptor_loop();
        load_cell_loop();

        // keep track of donation and weight
        float weight = load_cell_get_last_load_grams();
        int coins = coin_acceptor_get_amount_cents();

        switch (state) {
          case INSERT_COIN:    
            if (coins >= COCKTAIL_DONATION_CENTS && 
                weight >= CUP_WEIGHT_MIN_GRAMS && weight <= CUP_WEIGHT_MAX_GRAMS) {
                  printf("pissed off: coins donated and cup placed -> dispensing\n");
                  initial_weight_grams = weight;
                  dispensing_started_time_ms = esp_timer_get_time();
                  set_peristaltic_pump_on();
                  state = DISPENSING;
                }
            break;

          case DISPENSING:
            int drink_dispensed_ml = load_cell_get_last_load_grams() - initial_weight_grams;    // assuming ml == grams
            uint64_t duration_ms = esp_timer_get_time() - dispensing_started_time_ms;
            if (drink_dispensed_ml >= COCKTAIL_SIZE_ML || duration_ms > DISPENSING_TIMEOUT_MS) {
              if (duration_ms > DISPENSING_TIMEOUT_MS) printf("pissed off: ERROR ran out of supply\n");
              else printf("pissed off: cup filled -> take cup\n");
              set_peristaltic_pump_off();
              state = TAKE_CUP;
            }
            break;

          case TAKE_CUP:
            if (weight < CUP_WEIGHT_MIN_GRAMS) {
              printf("pissed off: cup taken -> insert coin\n");
              coin_acceptor_reset_amount();
              load_cell_tare();                       // reset load cell every time to account for drift
              state = INSERT_COIN;
            }
            break;
        }

        vTaskDelay(MAIN_LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
}