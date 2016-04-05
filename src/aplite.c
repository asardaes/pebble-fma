#include <pebble.h>

#include "common.h"
#include "aplite.h"

static Window *s_main_window;

static GFont s_font_time, s_font_date, s_font_jagged, s_font_temp;

static TextLayer *s_time_layer, *s_date_layer, *s_message_layer, *s_batt_layer, *s_temp_layer;

static BitmapLayer *s_bg_layer, *s_hands_layer, *s_sparks_layer, *s_rune_layer, *s_circle_layer, *s_charge_layer, *s_stone_layer, *s_stone_layer_2;
static GBitmap *s_bg_bitmap, *s_bgc_bitmap;
static GBitmap *s_hands_bitmap_0, *s_hands_bitmap_1, *s_hands_bitmap_2;
static GBitmap *s_sparks_bitmap_1, *s_sparks_bitmap_2, *s_sparks_bitmap_3, *s_sparks_bitmap_4;
static GBitmap *s_rune_bitmap, *s_circle_bitmap, *s_charge_bitmap, *s_stone_bitmap, *s_stone_bitmap_2;

static char date_buffer[16];
static char time_buffer[] = "00:00";
static char batt_buffer[] = "100";
static char temperature[5];

static int anim_index = 1;
static uint32_t anim_duration = 75;
static int anim_queue = 0;

static bool weather_flag = 1; // true initially
static bool t_unit = 0; // false (°C) by default

static bool dbg = 0;

/* ===================================================================================================================== */
//static void next_animation();
//static void update_time();

void debug() {
	static int test = -20;
	snprintf(temperature, sizeof(temperature), "%d", test);
	text_layer_set_text(s_temp_layer, temperature);
	test++;
	update_time();
	app_timer_register(500, debug, NULL);
	//next_animation();
}

/* ===================================================================================================================== */

void tap_handler_exit(void *data) {
	next_animation();
}

void tap_handler(AccelAxisType axis, int32_t direction) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Entering tap_handler");
	bitmap_layer_set_bitmap(s_bg_layer, s_bgc_bitmap);
	text_layer_set_text(s_message_layer, "Don't\nforget\n3.oct.11");
	
	if (anim_queue == 0) {
		app_timer_register(5000, tap_handler_exit, NULL);
		anim_queue++;
	}
}

void handle_bt(bool connected) {
	if (s_rune_bitmap)
		gbitmap_destroy(s_rune_bitmap);
	
	if (connected) {
		s_rune_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
		bitmap_layer_set_bitmap(s_rune_layer, s_rune_bitmap);
	} else {
		s_rune_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOBT);
		bitmap_layer_set_bitmap(s_rune_layer, s_rune_bitmap);
	}
}

void handle_battery(BatteryChargeState charge_state) {	
	if (charge_state.is_charging) {
		layer_set_hidden(bitmap_layer_get_layer(s_charge_layer), false);
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_charge_layer), true);
	}
	
	if (dbg)
		snprintf(batt_buffer, sizeof(batt_buffer), "100");
	else
		snprintf(batt_buffer, sizeof(batt_buffer), "%d", charge_state.charge_percent);		
	
	text_layer_set_text(s_batt_layer, batt_buffer);
}

/* ===================================================================================================================== */

void update_time() {
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
	
	if (dbg)
		text_layer_set_text(s_time_layer, "12:34");
	else {
		text_layer_set_text(s_time_layer, time_buffer);
		
		if (bluetooth_connection_service_peek() && weather_flag)
			get_weather();
	}
	
	// Date
	strftime(date_buffer, sizeof(date_buffer), "%b, %a %d", tick_time);
	text_layer_set_text(s_date_layer, date_buffer);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (units_changed & HOUR_UNIT) {
		// To mark that temperature should be updated
		weather_flag = 1;
		text_layer_set_text(s_temp_layer, "...");
	}
	
	if (anim_queue == 0) {
		anim_queue++;
		next_animation();
	}
}

/* ===================================================================================================================== */

void next_animation() {
	bool stop = 0;
	
	switch (anim_index) {
		case 1:
		bitmap_layer_set_bitmap(s_hands_layer, s_hands_bitmap_1);
		anim_index++;
		break;
		
		case 2:
		bitmap_layer_set_bitmap(s_hands_layer, s_hands_bitmap_2);
		text_layer_set_text(s_time_layer, "");
		text_layer_set_text(s_date_layer, "");
		text_layer_set_text(s_message_layer, "");
		bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
		anim_duration = 100;
		anim_index++;
		break;
		
		case 3:
		psleep(200);
		
		bitmap_layer_set_bitmap(s_sparks_layer, s_sparks_bitmap_1);
		layer_set_hidden(bitmap_layer_get_layer(s_sparks_layer), false);
		
		anim_index++;
		break;
		
		case 4:
		bitmap_layer_set_bitmap(s_sparks_layer, s_sparks_bitmap_2);
		
		anim_index++;
		break;
		
		case 5:
		bitmap_layer_set_bitmap(s_hands_layer, s_hands_bitmap_1);
		bitmap_layer_set_bitmap(s_sparks_layer, s_sparks_bitmap_3);
		
		anim_index++;
		break;
		
		case 6:
		bitmap_layer_set_bitmap(s_hands_layer, s_hands_bitmap_0);
		bitmap_layer_set_bitmap(s_sparks_layer, s_sparks_bitmap_4);
		
		anim_index++;
		break;
		
		default:
		layer_set_hidden(bitmap_layer_get_layer(s_sparks_layer), true);
		anim_index = 1;
		anim_duration = 75;
		
		if (!dbg)
			stop = 1;
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
			if (t_unit) {
				snprintf(temperature, sizeof(temperature), "%d",  ((int)t->value->int32) * 9/5 + 32 );
			} else {
				snprintf(temperature, sizeof(temperature), "%d", (int)t->value->int32);
			}
			text_layer_set_text(s_temp_layer, temperature);
			
			// Mark that weather shouldn't be updated any more
			weather_flag = 0; 
			break;
			
			case UNIT_TEMPERATURE:
			if (strcmp(t->value->cstring, "C") == 0) {
				persist_write_bool(UNIT_TEMPERATURE, false);
				t_unit = 0;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting temperature units to celsius");
			} else if (strcmp(t->value->cstring, "F") == 0) {
				persist_write_bool(UNIT_TEMPERATURE, true);
				t_unit = 1;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting temperature units to fahrenheit");
			}
			
			text_layer_set_text(s_temp_layer, "...");
			weather_flag = 1;
			get_weather();
			break;
			
			default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			break;
		}

    // Look for next item
    t = dict_read_next(iterator);
	}
}

/* ===================================================================================================================== */

void main_window_load(Window *window) {
	// Black background, 144 × 168 pixels, 176 PPI
	window_set_background_color(window, GColorBlack);
	
	// Assign GFont
	s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_30));
	s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_18));
	s_font_jagged = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JAGGED_24));
	s_font_temp = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEMP_14));
	
	// Background layer
	s_bg_layer = bitmap_layer_create(GRect(23, 5, 100, 100));
	bitmap_layer_set_compositing_mode(s_bg_layer, GCompOpAssign);
	s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	s_bgc_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_CIRCLE);
	bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bg_layer));
	
	// Hands layer
	s_hands_layer = bitmap_layer_create(GRect(0, 168-62, 144, 62));
	bitmap_layer_set_compositing_mode(s_hands_layer, GCompOpAssign);
	s_hands_bitmap_0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_0);
	bitmap_layer_set_bitmap(s_hands_layer, s_hands_bitmap_0);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_hands_layer));
	
	s_hands_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_1);
	s_hands_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_2);
	
	// Sparks layer
	s_sparks_layer = bitmap_layer_create(GRect(0, 168-62, 144, 62));
	
	bitmap_layer_set_compositing_mode(s_sparks_layer, GCompOpOr);
	
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_sparks_layer));
	
	s_sparks_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_1);
	s_sparks_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_2);
	s_sparks_bitmap_3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_3);
	s_sparks_bitmap_4 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPARKS_4);
	
	// Blood rune layer
	s_rune_layer = bitmap_layer_create(GRect(0, 0, 30, 30));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_rune_layer));
	
	// Battery layers
	s_circle_layer = bitmap_layer_create(GRect(144-30, 0, 30, 30));
	s_circle_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CIRCLE);
	bitmap_layer_set_bitmap(s_circle_layer, s_circle_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_circle_layer));
	
	s_batt_layer = text_layer_create(GRect(144-29, 4, 30, 30));
	text_layer_set_background_color(s_batt_layer, GColorClear);
	text_layer_set_text_color(s_batt_layer, GColorWhite);	
	text_layer_set_font(s_batt_layer, s_font_date);
	text_layer_set_text_alignment(s_batt_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batt_layer));
	
	s_charge_layer = bitmap_layer_create(GRect(144-30, 0, 30, 30));
	bitmap_layer_set_compositing_mode(s_charge_layer, GCompOpAssign);
	s_charge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE);
	bitmap_layer_set_bitmap(s_charge_layer, s_charge_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_charge_layer));
	
	// Time layer
	s_time_layer = text_layer_create(GRect(0, 168-62, 144, 62));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);	
	text_layer_set_font(s_time_layer, s_font_time);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	// Date layer
	s_date_layer = text_layer_create(GRect(0, 168-27, 144, 25));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_font(s_date_layer, s_font_date);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	
	// Message layer
	s_message_layer = text_layer_create(GRect(23, 10, 100, 100));
	text_layer_set_background_color(s_message_layer, GColorClear);
	text_layer_set_text_color(s_message_layer, GColorWhite);	
	text_layer_set_font(s_message_layer, s_font_jagged);
	text_layer_set_text_alignment(s_message_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_message_layer));
	
	// Temperature layers
	s_stone_layer = bitmap_layer_create(GRect(144-29, 83, 27, 19));
	s_stone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE);
	bitmap_layer_set_bitmap(s_stone_layer, s_stone_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer));
	
	s_temp_layer = text_layer_create(GRect(144-28, 87, 17, 30));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorBlack);	
	text_layer_set_font(s_temp_layer, s_font_temp);
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
	text_layer_set_text(s_temp_layer, "...");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
	
	// Second stone layer
	s_stone_layer_2 = bitmap_layer_create(GRect(3, 84, 26, 18));
	s_stone_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE2);
	bitmap_layer_set_bitmap(s_stone_layer_2, s_stone_bitmap_2);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer_2));
	
	// Get BT status when window loads
	bool bt = bluetooth_connection_service_peek();
	handle_bt(bt);
	
	// Get battery percentage when window loads
	BatteryChargeState batt = battery_state_service_peek();
	handle_battery(batt);
}

void main_window_unload(Window *window) {
	// Unload fonts
	fonts_unload_custom_font(s_font_time);
	fonts_unload_custom_font(s_font_date);
	fonts_unload_custom_font(s_font_jagged);
	fonts_unload_custom_font(s_font_temp);
	
	// Destroy bitmap layers
	bitmap_layer_destroy(s_bg_layer);
	bitmap_layer_destroy(s_hands_layer);
	bitmap_layer_destroy(s_sparks_layer);
	bitmap_layer_destroy(s_rune_layer);
	bitmap_layer_destroy(s_circle_layer);
	bitmap_layer_destroy(s_charge_layer);
	bitmap_layer_destroy(s_stone_layer);
	bitmap_layer_destroy(s_stone_layer_2);
	
	// Destroy GBitmaps
	gbitmap_destroy(s_bg_bitmap);
	gbitmap_destroy(s_bgc_bitmap);
	gbitmap_destroy(s_hands_bitmap_0);
	gbitmap_destroy(s_hands_bitmap_1);
	gbitmap_destroy(s_hands_bitmap_2);
	gbitmap_destroy(s_sparks_bitmap_1);
	gbitmap_destroy(s_sparks_bitmap_2);
	gbitmap_destroy(s_sparks_bitmap_3);
	gbitmap_destroy(s_sparks_bitmap_4);
	gbitmap_destroy(s_rune_bitmap);
	gbitmap_destroy(s_circle_bitmap);
	gbitmap_destroy(s_charge_bitmap);
	gbitmap_destroy(s_stone_bitmap);
	gbitmap_destroy(s_stone_bitmap_2);
	
	// Destroy text layers
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_message_layer);
	text_layer_destroy(s_batt_layer);
	text_layer_destroy(s_temp_layer);
}

/* ===================================================================================================================== */

void init() {
	if (persist_exists(UNIT_TEMPERATURE))
		t_unit = persist_read_bool(UNIT_TEMPERATURE);
	
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
	bluetooth_connection_service_subscribe(&handle_bt);
	
	// Register battery service
	battery_state_service_subscribe(&handle_battery);
	
	// Register with TickTimerService
	if (!dbg)
		tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
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
	bluetooth_connection_service_unsubscribe();
	battery_state_service_unsubscribe();
	window_destroy(s_main_window);
}
