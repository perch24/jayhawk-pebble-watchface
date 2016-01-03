#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_date_layer;
static GFont s_date_font;
static TextLayer *s_time_layer;
static GFont s_time_font;

static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static bool bluetooth_connected = false;
static GBitmap *s_connected_bitmap;
static GBitmap *s_notconnected_bitmap;
static BitmapLayer *s_connection_bitmap_layer;

static TextLayer *s_battery_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char time_buffer[12];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ?
                                          "%k:%M" : "%l:%M %p", tick_time);

  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %b %e", tick_time);

  // Display the date/time
  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void battery_handler(BatteryChargeState state) {
  static char buf[16];
  if (state.is_charging) {
    snprintf(buf, sizeof(buf), "CHRG");
  } else {
    snprintf(buf, sizeof(buf), "%d%%", state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, buf);
}

static void random_image() {
  gbitmap_destroy(s_bitmap);
  
  int image = rand() % 6;
  if (image == 0) {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_CURRENT);
  } else if (image == 1) {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_1912);
  } else if (image == 1) {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_1920);
  } else if (image == 1) {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_1929);
  } else if (image == 1) {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_1941);
  } else {
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_JAYHAWK_HEAD);
  }
  
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  random_image();
}

void update_connection() {
  if (bluetooth_connected) {
    bitmap_layer_set_bitmap(s_connection_bitmap_layer, s_connected_bitmap);
  } else {
    bitmap_layer_set_bitmap(s_connection_bitmap_layer, s_notconnected_bitmap);
  }
}

static void bluetooth_handler(bool connected) {
  if (bluetooth_connected != connected) {
    bluetooth_connected = connected;
    update_connection();
  }
}

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorWhite);
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Display the Jayhawk image
  s_bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_bitmap_layer, GAlignTop); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  
  s_connection_bitmap_layer = bitmap_layer_create( GRect(bounds.size.w - 25, 5, 25, 14) );
  layer_add_child(window_layer, bitmap_layer_get_layer(s_connection_bitmap_layer));
  s_connected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_CONNECTED);
  s_notconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_NOTCONNECTED);
  
  // Create the TextLayer with specific bounds
  s_battery_layer = text_layer_create(
      GRect(5, 5, 25, 14));
  s_date_layer = text_layer_create(
      GRect(0, bounds.size.h - 15, bounds.size.w, 14));
  s_time_layer = text_layer_create(
      GRect(0, bounds.size.h - 15 - 28, bounds.size.w, 28));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "100%");
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlue);
  text_layer_set_text(s_date_layer, "Mon Jan 01");

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorRed);
  text_layer_set_text(s_time_layer, "00:00");
 
  // Create GFont
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TRAJAN_BOLD_14));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TRAJAN_BOLD_28));

  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  random_image();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
  
  // Unload GFonts
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_time_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_bitmap);
  gbitmap_destroy(s_connected_bitmap);
  gbitmap_destroy(s_notconnected_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_bitmap_layer);
  bitmap_layer_destroy(s_connection_bitmap_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Make sure battery level displayed at startup
  battery_handler(battery_state_service_peek());
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  battery_state_service_subscribe(battery_handler);
  
  bluetooth_connection_service_subscribe(&bluetooth_handler);
  bluetooth_handler(bluetooth_connection_service_peek()); // initialize
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}