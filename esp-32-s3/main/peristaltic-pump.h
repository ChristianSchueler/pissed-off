// (c) 2025, Christian Schüler, hello@christianschueler.at

#ifndef PERISTALTIC_PUMP_H
#define PERISTALTIC_PUMP_H

void peristaltic_pump_init();
void peristaltic_pump_loop();

void set_peristaltic_pump_on();
void set_peristaltic_pump_off();
bool get_peristaltic_pump_state();

#endif