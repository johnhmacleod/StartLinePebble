#include <pebble.h>
#include "screens.h"
#include "startline.h"


// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_18_bold;
static GBitmap *s_res_bwarrow;
static TextLayer *s_textlayer_11;
static BitmapLayer *s_bitmaplayer_1;
static InverterLayer *s_inverterlayer_1;
char *sTxt[10] = {"Screen 1", "Screen 2", "Screen 3", "Screen 4", "Screen 5", "Screen 6", "Screen 7", "Screen 8", "Screen 9", "Screen 10"};
int sNum[10] = {0,1,2,3,4,5,6,7,8,9};
TextLayer *l[10] = {};

static TextLayer *setTL(GRect g, GColor colour, char *name, Window *window) {
  TextLayer *t;
  t = text_layer_create(g);
  text_layer_set_background_color(t, colour);
  text_layer_set_text(t, name);
  layer_add_child(window_get_root_layer(window), (Layer *)t);
  return t;
}


static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_bwarrow = gbitmap_create_with_resource(RESOURCE_ID_BWARROW);
  
  l[0] = setTL(GRect(16, 28, 100, 16), GColorClear, sTxt[0], s_window);
  l[1] = setTL(GRect(16,40,100,16), GColorClear, sTxt[1], s_window);
  l[2] = setTL(GRect(16, 52, 100, 16), GColorClear, sTxt[2], s_window);
  l[3] = setTL(GRect(16, 64, 100, 16), GColorClear, sTxt[3], s_window);
  l[4] = setTL(GRect(16, 76, 100, 16), GColorClear, sTxt[4], s_window);
  l[5] = setTL(GRect(16, 88, 100, 16), GColorClear, sTxt[5], s_window);
  l[6] = setTL(GRect(16, 100, 100, 16), GColorClear, sTxt[6], s_window);
  l[7] = setTL(GRect(16, 112, 100, 16), GColorClear, sTxt[7], s_window);
  l[8] = setTL(GRect(16, 124, 100, 16), GColorClear, sTxt[8], s_window);
  l[9] = setTL(GRect(16, 136, 100, 16), GColorClear, sTxt[9], s_window);
  s_textlayer_11 = setTL(GRect(3,1,138,24), GColorClear, "Reorder Screens", s_window);
  
  // s_bitmaplayer_1
  s_bitmaplayer_1 = bitmap_layer_create(GRect(0, 28, 16, 16));
  bitmap_layer_set_bitmap(s_bitmaplayer_1, s_res_bwarrow);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_1);
  
  // s_inverterlayer_1
  s_inverterlayer_1 = inverter_layer_create(GRect(14, 29, 65, 13));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_1);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  for (int i=0; i<10; i++)
    text_layer_destroy(l[i]);
  text_layer_destroy(s_textlayer_11);
  bitmap_layer_destroy(s_bitmaplayer_1);
  inverter_layer_destroy(s_inverterlayer_1);
  gbitmap_destroy(s_res_bwarrow);
}
// END AUTO-GENERATED UI CODE

static int s = 0;
static bool moving = false;
void moveInverter() {
  GRect i;
  
  i.size.h = 13;
  i.size.w = 65;
  i.origin = layer_get_frame((Layer *)l[s]).origin;
  i.origin.x -= 2;
  i.origin.y += 3;
  layer_set_frame((Layer *)s_inverterlayer_1, i);
}

void blankInverter() {
  layer_set_frame((Layer *)s_inverterlayer_1, GRectZero);
}

static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (moving) {
    moving = false;
    blankInverter();
  }
  else {
    moving = true;
    moveInverter();
  }
}

void swapText(int old, int new) {
 int tmp;
  tmp = sNum[old];
  sNum[old] = sNum[new];
  sNum[new] = tmp;
  if (screens[sNum[new]].num_fields == 0) 
    text_layer_set_text(l[new], "Empty");
  else
    text_layer_set_text(l[new], sTxt[sNum[new]]);
  if (screens[sNum[old]].num_fields == 0) 
    text_layer_set_text(l[old], "Empty");
  else
    text_layer_set_text(l[old], sTxt[sNum[old]]);
  
}

void moveArrow() {
  GRect g, h;
  g = layer_get_frame((Layer *)l[s]);
  h = layer_get_frame((Layer *)s_bitmaplayer_1);
  h.origin.x = 0;
  h.origin.y = g.origin.y + 1;
  layer_set_frame((Layer *)s_bitmaplayer_1, h);
}

static void s_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  int old_s;
  
  old_s = s;
  s--;
  if (s < 0)
    s = 9;
  moveArrow();
  if (moving) {
    moveInverter();
    swapText(old_s, s);
  }
}

static void s_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  int old_s;
  
  old_s = s;
  s++;
  s = s % 10;
  moveArrow();
  if (moving) {
    moveInverter();
    swapText(old_s, s);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, s_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, s_down_click_handler);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
  int i;
  Screen scr[NUM_SCREENS];
  
  updatescreen(-1, "");
  for (i = 0; i < NUM_SCREENS; i++){
          scr[i].num_fields = screens[sNum[i]].num_fields;
          int j;
          for (j=0; j<screens[sNum[i]].num_fields; j++) // Copy the map from the current screen
            {
            scr[i].field_layer_map[j] = screens[sNum[i]].field_layer_map[j];
            scr[i].field_data_map[j] = screens[sNum[i]].field_data_map[j];
            }
  }
  for (i = 0; i< NUM_SCREENS; i++) {
          screens[i].num_fields = scr[i].num_fields;
          int j;
          for (j=0; j<scr[i].num_fields; j++) // Copy the map from the current screen
            {
            screens[i].field_layer_map[j] = scr[i].field_layer_map[j];
            screens[i].field_data_map[j] = scr[i].field_data_map[j];
            }
  }
  currentScreen = 0;
  updatescreen(currentScreen, "");
}

void show_screens(void) {
  initialise_ui();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "done ui free=%d", (int)heap_bytes_free());
  int i;
  for (i = 0; i< NUM_SCREENS; i++)
    sNum[i] = i;
  s = 0;
  layer_set_frame((Layer *)s_inverterlayer_1, GRectZero);
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  
  for (i = 0; i< NUM_SCREENS; i++) 
    if (screens[i].num_fields == 0)
      text_layer_set_text(l[i], "Empty");
  window_stack_push(s_window, true);
}

void hide_screens(void) {
  window_stack_remove(s_window, true);
}
