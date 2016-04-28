#include <pebble.h>
#include "common.h"

extern Layer *date_layer_b, *batt_layer_b, *time_layer_b; // basalt

extern TextLayer *date_layer, *batt_layer, *time_layer; // aplite
extern TextLayer *message_layer, *temp_layer;

extern BitmapLayer *bg_layer, *hands_layer, *sparks_layer, *rune_layer, *charge_layer;

extern GBitmap *bg_bitmap;
extern GBitmap *hands_bitmap_0, *hands_bitmap_1, *hands_bitmap_2;
extern GBitmap *rune_bitmap, *charge_bitmap;

extern uint32_t anim_duration;

static Window *s_main_window;

static GBitmap *s_sparks_bitmap_1, *s_sparks_bitmap_2;

static char temperature[5];

static int anim_index = 1;
static int anim_queue = 0;

static bool weather_flag = true; // true initially
static bool temp_unit = false; // false (°C) by default
static bool show_hands = true;

static bool dbg = false;

static void next_animation();

char date_buffer[16];
char batt_buffer[] = "100";

char time_buffer[] = "00:00";

/* ===================================================================================================================== */

static void update_time();

void debug() {	
	static int test = 7;
	snprintf(temperature, sizeof(temperature), "%d", test);
	text_layer_set_text(temp_layer, temperature);
	//test++;
	update_time();
	//app_timer_register(500, debug, NULL);
	next_animation();
}

/* ===================================================================================================================== */

static char *translate_error(AppMessageResult result) {
  	switch (result) {
		case APP_MSG_OK: return "APP_MSG_OK";
		case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
		case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
		case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
		case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
		case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
		case APP_MSG_BUSY: return "APP_MSG_BUSY";
		case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
		case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
		case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
		case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
		case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
		case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
		case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
		default: return "UNKNOWN ERROR";
  }
}

static void get_weather() {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Getting new weather info.");

	// Begin dictionary
	DictionaryIterator *iter;
	
	AppMessageResult res = app_message_outbox_begin(&iter);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMsgBegin: %s", translate_error(res));

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "NULL iter.");
		dict_write_end(iter);
		return;
	}

	// Add a key-value pair
	dict_write_uint8(iter, 0, 0);
	dict_write_end(iter);

	// Send the message!
	res = app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMsgSend: %s", translate_error(res));
}

/* ===================================================================================================================== */

static void tap_handler_exit(void *data) {
	if (bg_bitmap) gbitmap_destroy(bg_bitmap);
	bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	bitmap_layer_set_bitmap(bg_layer, bg_bitmap);
	text_layer_set_text(message_layer, "");
	
	next_animation();
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	if (anim_queue == 0) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Entering tap_handler");
	
		if (bg_bitmap) gbitmap_destroy(bg_bitmap);
	
		bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_CIRCLE);
		bitmap_layer_set_bitmap(bg_layer, bg_bitmap);
	
		text_layer_set_text(message_layer, "Don't\nforget\n3.oct.11");
	
		app_timer_register(5000, tap_handler_exit, NULL);
		anim_queue++;
	}
}

/* ===================================================================================================================== */

void handle_bt(bool connected) {
	if (rune_bitmap) gbitmap_destroy(rune_bitmap);
	
	if (connected) {
		rune_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
	} else {
		rune_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOBT);
	}
	
	bitmap_layer_set_bitmap(rune_layer, rune_bitmap);
}

void handle_battery(BatteryChargeState charge_state) {
	if (charge_bitmap) gbitmap_destroy(charge_bitmap);
	
	if (charge_state.is_charging) {
		charge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE);
		snprintf(batt_buffer, sizeof(batt_buffer), "%s", "");
		
	} else {
		charge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CIRCLE);
		snprintf(batt_buffer, sizeof(batt_buffer), "%d", charge_state.charge_percent);
	}

	PBL_IF_COLOR_ELSE(layer_mark_dirty(batt_layer_b), text_layer_set_text(batt_layer, batt_buffer));
	
	bitmap_layer_set_bitmap(charge_layer, charge_bitmap);
}

/* ===================================================================================================================== */

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	// Write the current hours and minutes into the buffer
	if(clock_is_24h_style() == true) {
		// Use 24 hour format
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	} else {
		// Use 12 hour format
		strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
	}
	
	if (dbg) {
		snprintf(time_buffer, sizeof(time_buffer), "%s", "12:34");
	} else if (connection_service_peek_pebble_app_connection() && weather_flag) {
		get_weather();
	}

	PBL_IF_COLOR_ELSE(layer_mark_dirty(time_layer_b), text_layer_set_text(time_layer, time_buffer));
	
	// Date
	strftime(date_buffer, sizeof(date_buffer), "%b, %a %d", tick_time);

	PBL_IF_COLOR_ELSE(layer_mark_dirty(date_layer_b), text_layer_set_text(date_layer, date_buffer));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (units_changed & HOUR_UNIT) {
		// To mark that temperature should be updated
		weather_flag = 1;
		text_layer_set_text(temp_layer, "...");
	}
	
	if (anim_queue == 0) {
		anim_queue++;
		next_animation();
	}
}

/* ===================================================================================================================== */

static void next_animation() {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Animation %d", anim_index);
	bool stop = false;
	
	switch (anim_index) {
		case 0:
			layer_set_hidden(bitmap_layer_get_layer(hands_layer), false);
			anim_index++;
			anim_duration += 500;
		break;

		case 1:
			if (anim_duration > 500) anim_duration -= 500;
			bitmap_layer_set_bitmap(hands_layer, hands_bitmap_1);
			anim_index++;
		break;
		
		case 2:
			s_sparks_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_1);
			bitmap_layer_set_bitmap(hands_layer, hands_bitmap_2);

			snprintf(date_buffer, sizeof(date_buffer), "%s", "");
			snprintf(time_buffer, sizeof(time_buffer), "%s", "");

			PBL_IF_COLOR_ELSE(layer_mark_dirty(date_layer_b),
				layer_mark_dirty(text_layer_get_layer(date_layer)));
			PBL_IF_COLOR_ELSE(layer_mark_dirty(time_layer_b),
				layer_mark_dirty(text_layer_get_layer(time_layer)));

			anim_duration += 25;
			anim_index++;
		break;
		
		case 3:
			psleep(PBL_IF_COLOR_ELSE(175, 200));

			bitmap_layer_set_bitmap(sparks_layer, s_sparks_bitmap_1);
			layer_set_hidden(bitmap_layer_get_layer(sparks_layer), false);

			s_sparks_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_2);

			anim_index++;
		break;
		
		case 4:
			bitmap_layer_set_bitmap(sparks_layer, s_sparks_bitmap_2);

			gbitmap_destroy(s_sparks_bitmap_1);
			s_sparks_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_3);

			anim_index++;
		break;
		
		case 5:
			bitmap_layer_set_bitmap(sparks_layer, s_sparks_bitmap_1);
			bitmap_layer_set_bitmap(hands_layer, hands_bitmap_1);

			gbitmap_destroy(s_sparks_bitmap_2);
			s_sparks_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_4);

			anim_index++;
		break;
		
		case 6:
			bitmap_layer_set_bitmap(sparks_layer, s_sparks_bitmap_2);
			bitmap_layer_set_bitmap(hands_layer, hands_bitmap_0);

			gbitmap_destroy(s_sparks_bitmap_1);

			anim_index++;
		break;

		case 7:
			layer_set_hidden(bitmap_layer_get_layer(sparks_layer), true);
			gbitmap_destroy(s_sparks_bitmap_2);

			anim_duration -= 25;

			if (show_hands) {
				anim_index = 1;

				if (!dbg)
					stop = true;
				else
					update_time();

			} else {
				anim_index++;
				anim_duration += 500;
				update_time();
			}
		break;
		
		default:
			layer_set_hidden(bitmap_layer_get_layer(hands_layer), true);
			anim_index = 0;
			anim_duration -= 500;

			if (!dbg)
				stop = true;
			else
				update_time();
		break;
	}
	
	if (!stop) {
		if (dbg)
			app_timer_register(5000, next_animation, NULL);
		else
			app_timer_register(anim_duration, next_animation, NULL);
	} else {
		update_time();
		anim_queue = 0;
	}
}

/* ===================================================================================================================== */

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Read first item
	Tuple *t = dict_read_first(iterator);
	
	// For all items
	while(t != NULL) {
		// Which key was received?
		switch(t->key) {
			case WEATHER_TEMPERATURE:
				// true = Fahrenheit
				if (temp_unit) {
					snprintf(temperature, sizeof(temperature), "%d",  ((int)t->value->int32) * 9/5 + 32 );
				} else {
					snprintf(temperature, sizeof(temperature), "%d", (int)t->value->int32);
				}
				text_layer_set_text(temp_layer, temperature);
			
				// Mark that weather shouldn't be updated any more
				weather_flag = 0; 
			break;
			
			case UNIT_TEMPERATURE:
				if (strcmp(t->value->cstring, "C") == 0) {
					persist_write_bool(UNIT_TEMPERATURE, false);
					temp_unit = 0;
					APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting temperature units to celsius");
				} else if (strcmp(t->value->cstring, "F") == 0) {
					persist_write_bool(UNIT_TEMPERATURE, true);
					temp_unit = 1;
					APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting temperature units to fahrenheit");
				}
			
				text_layer_set_text(temp_layer, "...");
				weather_flag = 1;
				get_weather();
			break;

			case HANDS:
				if (strcmp(t->value->cstring, "yes") == 0) {
					show_hands = true;
					layer_set_hidden(bitmap_layer_get_layer(hands_layer), false);
					persist_write_bool(HANDS, true);
					if (anim_queue == 0) anim_index = 1;

				} else {
					show_hands = false;
					layer_set_hidden(bitmap_layer_get_layer(hands_layer), true);
					persist_write_bool(HANDS, false);
					if (anim_queue == 0) anim_index = 0;
				}
			break;
			
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			break;
		}

		// Look for next item
		t = dict_read_next(iterator);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!: %s", translate_error(reason));
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!: %s", translate_error(reason));
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/* ===================================================================================================================== */

void init() {
	if (persist_exists(UNIT_TEMPERATURE))
		temp_unit = persist_read_bool(UNIT_TEMPERATURE);

	if (persist_exists(HANDS))
		show_hands = persist_read_bool(HANDS);
	
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	// Register bluetooth service
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = handle_bt,
		NULL
	});
	
	// Register battery service
	battery_state_service_subscribe(handle_battery);
	
	// Register with TickTimerService
	if (!dbg) tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Subscribe to the accelerometer tap service
	accel_tap_service_subscribe(tap_handler);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Open AppMessage
	AppMessageResult res = app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMsgOpen: %s", translate_error(res));
	
	if (dbg)
		debug();
	else {
		anim_queue++;
		next_animation();
	}
}

void deinit() {
	tick_timer_service_unsubscribe();
	accel_tap_service_unsubscribe();
	connection_service_unsubscribe();
	battery_state_service_unsubscribe();
	window_destroy(s_main_window);
}

