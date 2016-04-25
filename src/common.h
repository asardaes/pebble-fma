#pragma once

#include <pebble.h>

#ifndef WEATHER_TEMPERATURE
#define WEATHER_TEMPERATURE 0
#endif

#ifndef UNIT_TEMPERATURE
#define UNIT_TEMPERATURE 1
#endif

void handle_battery(BatteryChargeState charge_state); // used by main_window_load
void handle_bt(bool connected); // used by main_window_load

void main_window_load(Window *window);
void main_window_unload(Window *window);

void init();
void deinit();

