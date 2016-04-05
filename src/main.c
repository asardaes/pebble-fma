#include <pebble.h>

#include "aplite.h"

int main(void) {	
	init();
	app_event_loop();
	deinit();
}
