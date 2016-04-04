#include <pebble.h>

#include "aplite.h"

#ifndef WEATHER_TEMPERATURE
#define WEATHER_TEMPERATURE 0
#endif

#ifndef UNIT_TEMPERATURE
#define UNIT_TEMPERATURE 1
#endif

int main(void) {	
	init();
	app_event_loop();
	deinit();
}
