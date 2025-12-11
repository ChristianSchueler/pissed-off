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

#define VERSION 1.0
//#define DEBUG_PRINTF 1

enum State {
  INSERT_COIN,          // donate coins
  DISPENSING,           // dispense drink
  TAKE_CUP              // waiting to take cup
};

#define MAIN_LOOP_DELAY_MS 100                // how much we intentionally slow down the main loop -> 100 ms by default
#define COCKTAIL_MIN_DONATION_SMALL_CENTS 200 // donate at least eur 2 for small cup
#define COCKTAIL_MIN_DONATION_LARGE_CENTS 300 // donate at least eur 3 for larger cup
#define COCKTAIL_SIZE_SMALL_ML 100            // small drink is 10 ml
#define COCKTAIL_SIZE_LARGE_ML 180            // large drink is 18 ml, as in original recipe
#define CUP_WEIGHT_MIN_GRAMS 3                // minimum empty cup weight, used to identify placement of a cup
#define CUP_WEIGHT_MAX_GRAMS 20               // maximum empty cup weight
#define DISPENSING_TIMEOUT_MS 1500*60         // after dispensing time out we stop; most probably empty supply
#define MIN_FILL_DURATION_MS 1000             // how long we fill the cup at least, even if someone removed the cup (necessary to ignore load scale measures bouncing)
#define MIN_DISPENSING_DELAY_MS 1000*2        // start 2 s after (!) placing the cup to make sure shaky hands do get splashed (trust me with this one)

enum State state = INSERT_COIN;               // simple state based application with 3 states

float initial_weight_grams;                   // remember weight when starting to fill the cup. most probably very small
int targetDrinkSize_ml;                       // either small or large amount, will be set in INSERT_COIN state

// computation of data types for times: microsecs: 86.400.000.000 per 24 hours
// long int: max.: 2.147.483.647 -> overflow after about 1h 40s! 
// -> we need 64 bit, i.e. int64_t

uint64_t dispensing_started_time_ms;          // when we started to dispense; used for timeout
int64_t cup_placed_time_ms = -1;              // when we placed the cup; used to wait a bit until the hands are away

void app_main(void)
{
    printf("Pissed Off - Mechatronic Interactive Cocktail Machine, (c) 2025 Christian Schüler, hello@christianschueler.at\n");
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

        #ifdef DEBUG_PRINTF
        printf("DEBUG - state: %d\n", state);
        #endif

        switch (state) {
          // -------------------------------------------------------------------------------------------------------------------------------------------
          case INSERT_COIN:
            int cup_placed = (weight >= CUP_WEIGHT_MIN_GRAMS && weight <= CUP_WEIGHT_MAX_GRAMS);    // true when cup is present

            // remember when we placed the cup (or -1 when no cup present)
            if (cup_placed) {   // if cup has been placed just now
              if (cup_placed_time_ms < 0) {
                cup_placed_time_ms = esp_timer_get_time()/1000;
                printf("pissed off: empty cup has been placed\n");
                printf("pissed off: empty cup weight: %1.1f g\n", weight);
              }
            }
            else {     // if cup has been lifted just now
              if (cup_placed_time_ms > 0) {
                cup_placed_time_ms = -1;
                printf("pissed off: empty cup has been taken\n");
              }
            }

            // compute how long the cup has been present (or -1 when not present)
            long int duration_cup_present_ms;
            if (cup_placed && cup_placed_time_ms >= 0) {
              duration_cup_present_ms = (esp_timer_get_time() - cup_placed_time_ms*1000)/1000;
              #ifdef DEBUG_PRINTF
              printf("pissed off: cup present duration: %ld ms\n", duration_cup_present_ms);
              #endif
            }
            else duration_cup_present_ms = -1;

            // coins donated, cup present for long enough? -> go!
            if (coins >= COCKTAIL_MIN_DONATION_SMALL_CENTS && 
                cup_placed &&
                duration_cup_present_ms >= MIN_DISPENSING_DELAY_MS) {   // if enough donation and cup is present and at least some time has passed since placing the cup
                  printf("pissed off: coins donated: %d\n", coins);
                  
                  // decide drink size depending on donation
                  if (coins >= COCKTAIL_MIN_DONATION_LARGE_CENTS) targetDrinkSize_ml = COCKTAIL_SIZE_LARGE_ML;
                  else targetDrinkSize_ml = COCKTAIL_SIZE_SMALL_ML;

                  printf("pissed off: drink size selected: %d ml\n", targetDrinkSize_ml);
                  printf("pissed off: coins donated and cup placed -> dispensing\n");

                  initial_weight_grams = weight;
                  dispensing_started_time_ms = esp_timer_get_time()/1000;
                  set_peristaltic_pump_on();
                  
                  state = DISPENSING;
                }
            break;

          // -------------------------------------------------------------------------------------------------------------------------------------------
          case DISPENSING:
            int drink_dispensed_ml = weight - initial_weight_grams;    // assuming ml == grams
            uint64_t duration_ms = (esp_timer_get_time() - dispensing_started_time_ms*1000)/1000;
            int tookCup = (weight < CUP_WEIGHT_MIN_GRAMS && duration_ms >= MIN_FILL_DURATION_MS);

            #ifdef DEBUG_PRINTF
            printf("DEBUG - drink_dispensed_ml: %d\n", drink_dispensed_ml);
            printf("DEBUG - duration_ms: %lld\n", duration_ms);
            #endif

            // if we either have reached drink size or timeout -> stop
            if (drink_dispensed_ml >= targetDrinkSize_ml || duration_ms > DISPENSING_TIMEOUT_MS) {    // cup is filled or timeout (e.g. ran out of booze)
              if (duration_ms > DISPENSING_TIMEOUT_MS) printf("pissed off: ERROR ran out of supply\n");
              else printf("pissed off: cup filled -> take cup\n");
              set_peristaltic_pump_off();
              state = TAKE_CUP;
            }
            else if (tookCup) {   // someone removed the cup mid-dispensing ot it "fell" down -> stop!
              printf("pissed off: ERROR cup removed -> insert coin\n");
              set_peristaltic_pump_off();
              cup_placed_time_ms = -1;
              state = INSERT_COIN;
            }
            break;

          // -------------------------------------------------------------------------------------------------------------------------------------------
          case TAKE_CUP:
            if (weight < CUP_WEIGHT_MIN_GRAMS) {
              printf("pissed off: cup taken -> insert coin\n");
              coin_acceptor_reset_amount();
              cup_placed_time_ms = -1;
              load_cell_tare();                       // reset load cell every time to account for drift
              state = INSERT_COIN;
            }
            break;
        }

        vTaskDelay(MAIN_LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
}