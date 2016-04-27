#ifdef PBL_PLATFORM_BASALT

#include <pebble.h>
#include "common.h"

extern char date_buffer[16];
extern char batt_buffer[4];
extern char time_buffer[6];

static GFont s_font_time, s_font_date, s_font_jagged, s_font_temp;

static BitmapLayer *s_stone_layer, *s_stone_layer_2;

static GBitmap *s_stone_bitmap, *s_stone_bitmap_2;

Layer *date_layer_b, *batt_layer_b, *time_layer_b;

TextLayer *message_layer, *temp_layer;

BitmapLayer *bg_layer, *hands_layer, *sparks_layer, *rune_layer, *charge_layer;

GBitmap *bg_bitmap;
GBitmap *hands_bitmap_0, *hands_bitmap_1, *hands_bitmap_2;
GBitmap *rune_bitmap, *charge_bitmap;

uint32_t anim_duration = 100;

/* ===================================================================================================================== */

static void date_border(Layer *this_layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);

	int x0 = 3;
	int y0 = 168 - 60;
	int x1 = 144;
	int y1 = 25;

	for (int i = -1; i < 2; i = i + 2) {
		for (int j = -1; j < 2; j = j + 2) {
			graphics_draw_text(ctx, date_buffer, s_font_date, GRect(x0 + i, y0 + j, x1, y1),
				GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		}
	}

	graphics_context_set_text_color(ctx, GColorLightGray);
	graphics_draw_text(ctx, date_buffer, s_font_date, GRect(x0, y0, x1, y1), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void time_border(Layer *this_layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);

	int x0 = 1;
	int y0 = 168 - 39;
	int x1 = 143;
	int y1 = 38;

	for (int i = -1; i < 2; i = i + 2) {
		for (int j = -1; j < 2; j = j + 2) {
			graphics_draw_text(ctx, time_buffer, s_font_time, GRect(x0 + i, y0 + j, x1, y1),
				GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		}
	}

	graphics_context_set_text_color(ctx, GColorDarkCandyAppleRed);
	graphics_draw_text(ctx, time_buffer, s_font_time, GRect(x0, y0, x1, y1), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void batt_border(Layer *this_layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);

	int x0 = 144 - 29;
	int y0 = 4;
	int x1 = 30;
	int y1 = 30;

	for (int i = -1; i < 2; i = i + 2) {
		for (int j = -1; j < 2; j = j + 2) {
			graphics_draw_text(ctx, batt_buffer, s_font_date, GRect(x0 + i, y0 + j, x1, y1),
				GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		}
	}

	graphics_context_set_text_color(ctx, GColorLightGray);
	graphics_draw_text(ctx, batt_buffer, s_font_date, GRect(x0, y0, x1, y1), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

/* ===================================================================================================================== */

void main_window_load(Window *window) {
	// Black background, 144 × 168 pixels, 176 PPI
	window_set_background_color(window, GColorWindsorTan);
	
	// Assign GFont
	s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_30));
	s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_18));
	s_font_jagged = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JAGGED_22));
	s_font_temp = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEMP_14));
	
	// Background layer
	bg_layer = bitmap_layer_create(GRect(23, 5, 100, 100));
	bitmap_layer_set_compositing_mode(bg_layer, GCompOpSet);
	bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	bitmap_layer_set_bitmap(bg_layer, bg_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bg_layer));
	
	// Hands layer
	hands_layer = bitmap_layer_create(GRect(0, 168-65, 144, 65));
	bitmap_layer_set_compositing_mode(hands_layer, GCompOpSet);
	hands_bitmap_0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_0);
	bitmap_layer_set_bitmap(hands_layer, hands_bitmap_0);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(hands_layer));
	
	hands_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_1);
	hands_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ARMS_2);
	
	// Blood rune layer
	rune_layer = bitmap_layer_create(GRect(0, 0, 30, 31));
	bitmap_layer_set_compositing_mode(rune_layer, GCompOpSet);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(rune_layer));
	
	// Battery layers
	charge_layer = bitmap_layer_create(GRect(144-30, 0, 30, 30));
	bitmap_layer_set_compositing_mode(charge_layer, GCompOpSet);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(charge_layer));
	
	batt_layer_b = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(batt_layer_b, batt_border);
	layer_add_child(window_get_root_layer(window), batt_layer_b);
	
	// Time layer
	time_layer_b = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(time_layer_b, time_border);
	layer_add_child(window_get_root_layer(window), time_layer_b);
	
	// Date layer
	date_layer_b = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(date_layer_b, date_border);
	layer_add_child(window_get_root_layer(window), date_layer_b);
	
	// Message layer
	message_layer = text_layer_create(GRect(23, 16, 100, 100));
	text_layer_set_background_color(message_layer, GColorClear);
	text_layer_set_text_color(message_layer, GColorBlack);
	text_layer_set_font(message_layer, s_font_jagged);
	text_layer_set_text_alignment(message_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(message_layer));
	
	// Temperature layers
	s_stone_layer = bitmap_layer_create(GRect(144-30, 82, 29, 21));
	bitmap_layer_set_compositing_mode(s_stone_layer, GCompOpSet);
	s_stone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE);
	bitmap_layer_set_bitmap(s_stone_layer, s_stone_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer));
	
	temp_layer = text_layer_create(GRect(144-28, 87, 17, 30));
	text_layer_set_background_color(temp_layer, GColorClear);
	text_layer_set_text_color(temp_layer, GColorLightGray);	
	text_layer_set_font(temp_layer, s_font_temp);
	text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
	text_layer_set_text(temp_layer, "...");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));
	
	// Second stone layer
	s_stone_layer_2 = bitmap_layer_create(GRect(2, 83, 28, 20));
	bitmap_layer_set_compositing_mode(s_stone_layer_2, GCompOpSet);
	s_stone_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STONE2);
	bitmap_layer_set_bitmap(s_stone_layer_2, s_stone_bitmap_2);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stone_layer_2));

	// Sparks layer
	sparks_layer = bitmap_layer_create(GRect(0, 168-126, 144, 126));	
	bitmap_layer_set_compositing_mode(sparks_layer, GCompOpSet);	
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(sparks_layer));
	
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
	text_layer_destroy(message_layer);
	text_layer_destroy(temp_layer);
	layer_destroy(date_layer_b);
	layer_destroy(time_layer_b);
	layer_destroy(batt_layer_b);
}

#endif
