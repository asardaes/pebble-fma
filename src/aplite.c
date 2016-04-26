#ifdef PBL_PLATFORM_APLITE

#include <pebble.h>
#include "common.h"

static GFont s_font_time, s_font_date, s_font_jagged, s_font_temp;

static BitmapLayer *s_stone_layer, *s_stone_layer_2;

static GBitmap *s_stone_bitmap, *s_stone_bitmap_2;

TextLayer *time_layer, *date_layer, *message_layer, *batt_layer, *temp_layer;

BitmapLayer *bg_layer, *hands_layer, *sparks_layer, *rune_layer, *charge_layer;

GBitmap *bg_bitmap;
GBitmap *hands_bitmap_0, *hands_bitmap_1, *hands_bitmap_2;
GBitmap *rune_bitmap, *charge_bitmap;

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
	bg_layer = bitmap_layer_create(GRect(23, 5, 100, 100));
	bitmap_layer_set_compositing_mode(bg_layer, GCompOpAssign);
	bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	bitmap_layer_set_bitmap(bg_layer, bg_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bg_layer));
	
	// Hands layer
	hands_layer = bitmap_layer_create(GRect(0, 168-62, 144, 62));
	bitmap_layer_set_compositing_mode(hands_layer, GCompOpAssign);
	hands_bitmap_0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_0);
	bitmap_layer_set_bitmap(hands_layer, hands_bitmap_0);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(hands_layer));
	
	hands_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_1);
	hands_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_2);
	
	// Sparks layer
	sparks_layer = bitmap_layer_create(GRect(0, 168-62, 144, 62));	
	bitmap_layer_set_compositing_mode(sparks_layer, GCompOpOr);	
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(sparks_layer));
	
	// Blood rune layer
	rune_layer = bitmap_layer_create(GRect(0, 0, 30, 30));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(rune_layer));
	
	// Battery layers
	charge_layer = bitmap_layer_create(GRect(144-30, 0, 30, 30));
	bitmap_layer_set_compositing_mode(charge_layer, GCompOpAssign);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(charge_layer));
	
	batt_layer = text_layer_create(GRect(144-29, 4, 30, 30));
	text_layer_set_background_color(batt_layer, GColorClear);
	text_layer_set_text_color(batt_layer, GColorWhite);	
	text_layer_set_font(batt_layer, s_font_date);
	text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(batt_layer));
	
	// Time layer
	time_layer = text_layer_create(GRect(0, 168-38, 144, 38));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorWhite);	
	text_layer_set_font(time_layer, s_font_time);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
	
	// Date layer
	date_layer = text_layer_create(GRect(0, 168-60, 142, 25));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_font(date_layer, s_font_date);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	
	// Message layer
	message_layer = text_layer_create(GRect(23, 11, 100, 100));
	text_layer_set_background_color(message_layer, GColorClear);
	text_layer_set_text_color(message_layer, GColorWhite);	
	text_layer_set_font(message_layer, s_font_jagged);
	text_layer_set_text_alignment(message_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(message_layer));
	
	// Temperature layers
	s_stone_layer = bitmap_layer_create(GRect(144-29, 83, 27, 19));
	s_stone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE);
	bitmap_layer_set_bitmap(s_stone_layer, s_stone_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer));
	
	temp_layer = text_layer_create(GRect(144-28, 87, 17, 30));
	text_layer_set_background_color(temp_layer, GColorClear);
	text_layer_set_text_color(temp_layer, GColorBlack);	
	text_layer_set_font(temp_layer, s_font_temp);
	text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
	text_layer_set_text(temp_layer, "...");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));
	
	// Second stone layer
	s_stone_layer_2 = bitmap_layer_create(GRect(3, 84, 26, 18));
	s_stone_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE2);
	bitmap_layer_set_bitmap(s_stone_layer_2, s_stone_bitmap_2);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer_2));
	
	// Get BT status when window loads
	handle_bt(connection_service_peek_pebble_app_connection());
	
	// Get battery percentage when window loads
	handle_battery(battery_state_service_peek());
}

void main_window_unload(Window *window) {
	// Unload fonts
	fonts_unload_custom_font(s_font_time);
	fonts_unload_custom_font(s_font_date);
	fonts_unload_custom_font(s_font_jagged);
	fonts_unload_custom_font(s_font_temp);
	
	// Destroy bitmap layers
	bitmap_layer_destroy(bg_layer);
	bitmap_layer_destroy(hands_layer);
	bitmap_layer_destroy(sparks_layer);
	bitmap_layer_destroy(rune_layer);
	bitmap_layer_destroy(charge_layer);
	bitmap_layer_destroy(s_stone_layer);
	bitmap_layer_destroy(s_stone_layer_2);
	
	// Destroy GBitmaps
	gbitmap_destroy(bg_bitmap);
	gbitmap_destroy(hands_bitmap_0);
	gbitmap_destroy(hands_bitmap_1);
	gbitmap_destroy(hands_bitmap_2);
	gbitmap_destroy(rune_bitmap);
	gbitmap_destroy(charge_bitmap);
	gbitmap_destroy(s_stone_bitmap);
	gbitmap_destroy(s_stone_bitmap_2);
	
	// Destroy text layers
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(message_layer);
	text_layer_destroy(batt_layer);
	text_layer_destroy(temp_layer);
}

#endif
