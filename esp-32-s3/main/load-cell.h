// (c) 2025, Christian Schüler, hello@christianschueler.at

#ifndef LOAD_CELL_H
#define LOAD_CELL_H

void load_cell_init();
void load_cell_loop();

int32_t load_cell_read_value_raw();
void load_cell_tare();
float load_cell_read_value_grams();

float load_cell_get_last_load_grams();

#endif