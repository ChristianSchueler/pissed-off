// (c) 2025, Christian Schüeler, hello@christianschueler.at

#ifndef LOAD_CELL_H
#define LOAD_CELL_H

void load_cell_init();
void load_cell_loop();
void load_cell_tare();
float load_cell_get_value_grams();
int32_t load_cell_get_value_raw();

#endif