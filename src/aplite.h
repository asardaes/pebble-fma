#include <pebble.h>

/* ===================================================================================================================== */

void debug();

/* ===================================================================================================================== */

void tap_handler_exit(void *data);

void tap_handler(AccelAxisType axis, int32_t direction);

void handle_bt(bool connected);

void handle_battery(BatteryChargeState charge_state);

/* ===================================================================================================================== */

void update_time();

void tick_handler(struct tm *tick_time, TimeUnits units_changed);

/* ===================================================================================================================== */

void next_animation();

/* ===================================================================================================================== */

void inbox_received_callback(DictionaryIterator *iterator, void *context);

/* ===================================================================================================================== */

void main_window_load(Window *window);

void main_window_unload(Window *window);

/* ===================================================================================================================== */

void init();

void deinit();
