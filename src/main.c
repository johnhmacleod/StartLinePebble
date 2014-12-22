#include "pebble.h"

    #define KEY_LAY_TIME     0
    #define KEY_LAY_DIST  1
    #define KEY_LAY_BURN  2
    #define KEY_LAY_SEL	 3
    #define KEY_LINE_TIME  4
    #define KEY_LINE_BURN  5
    #define KEY_LINE_DIST  6
    #define KEY_LINE_ANGLE  7
    #define KEY_SECS_TO_START  8
    #define KEY_START_SEQUENCE 9
    #define KEY_CURRENT_MARK 10
    #define KEY_BOAT_SPEED 11
    #define KEY_MARK_TURN 12
    #define KEY_WIND_SHIFT 13
    #define KEY_LAST_TACK 14
    #define KEY_MARK_LAY_DIST 15
    #define KEY_TARGET_TACK 16

  
#define MAPPING_PKEY  100
  
  
static Window *s_main_window;
static TextLayer *s_data_layer[10];
static TextLayer *s_data_title[10];

void doupdate();
void updatescreen();

static int currentScreen = 0;
static int configuring = 0; // Set to 1 when configuring display
static int configuring_field = 0; // Index of the title we are currently configuring


#define MAX_TITLES  9 //Number of elements in title array

static int data_field_keys[] = {KEY_LAY_BURN, KEY_LAY_DIST, KEY_LAY_TIME, KEY_LINE_BURN, KEY_LINE_DIST, KEY_LINE_TIME,
                                   KEY_LINE_ANGLE, KEY_SECS_TO_START, KEY_LAY_SEL };
static char *data_titles[] = {"Lay Burn", "Lay Dist", "Lay Time", "Line Burn", "Line Dist", "Line Time",
                               "Line Angle", "To Start", "Lay Line"};


typedef struct 
{
  int num_fields; // How many fields on this screen
  int field_data_map[6]; // How do these fields map to data
  int field_layer_map[6]; // How do these fields map to display real estate
  int field_small_layer_map[6]; // And small font display real estate when needed - smaller font needs placing differently
  int is_start; // true if this is a start related screen
} Screen;

#define NUM_SCREENS 10
static Screen screens[NUM_SCREENS] = {
                                      {6,
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      true
                                        },
                                      {2,
                                      {0,1},
                                      {6,7},
                                      {6,7},
                                      true
                                      },
                                      {2,
                                      {2,3},
                                      {6,7},
                                      {6,7},
                                      true
                                      }
                                    };  // Code relies on the rest of the array being zero to indicate screens not in use.


static GFont s_data_font, s_data_font_alpha, s_title_font, s_small_data_font, s_large_title_font, font;

static BitmapLayer *s_background_layer, *s_arrow_layer;
static GBitmap *s_background_bitmap, *s_arrow_bitmap;

static void main_window_load(Window *window) {
  
  // Use system font, apply it and add to Window
  s_data_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  s_data_font_alpha = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_large_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_small_data_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //Display Arrow
  s_arrow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BW_ARROW);
  s_arrow_layer = bitmap_layer_create(GRect(58, 84, 33, 15));
  // bitmap_layer_set_bitmap(s_arrow_layer, s_arrow_bitmap);
  bitmap_layer_set_bitmap(s_arrow_layer, NULL); //Empty for now
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_arrow_layer));
                                      
  // Create Display RectAngles
  
  // Six data fields & their titles
  s_data_layer[0] = text_layer_create(GRect(0, 1, 72, 49));
  s_data_title[0] = text_layer_create(GRect(0, 48, 72, 15));
  s_data_layer[1] = text_layer_create(GRect(72, 1, 72, 49));
  s_data_title[1] = text_layer_create(GRect(72, 48, 72, 15));
  s_data_layer[2] = text_layer_create(GRect(0, 53, 72, 49));
  s_data_title[2] = text_layer_create(GRect(0, 102, 72, 14));
  s_data_layer[3] = text_layer_create(GRect(72, 53, 72, 49));
  s_data_title[3] = text_layer_create(GRect(72, 102, 72, 14));
  s_data_layer[4] = text_layer_create(GRect(0, 106, 72, 49));
  s_data_title[4] = text_layer_create(GRect(0, 153, 72, 15));
  s_data_layer[5] = text_layer_create(GRect(72, 106, 72, 49));
  s_data_title[5] = text_layer_create(GRect(72, 153, 72, 15));
  
  // Two data fields & their titles
  s_data_layer[6] = text_layer_create(GRect(0, 10, 144, 65));
  s_data_title[6] = text_layer_create(GRect(0, 55, 144, 34));
  s_data_layer[7] = text_layer_create(GRect(0, 89, 144, 65));
  s_data_title[7] = text_layer_create(GRect(0, 134, 144, 34));
  
  // Top title
  s_data_layer[8] = text_layer_create(GRect(0, 0, 144, 14));

  
  int i;
  for (i =0; i < 8; i++)
    {
    text_layer_set_background_color(s_data_layer[i], GColorClear);
    text_layer_set_text_color(s_data_layer[i], GColorWhite);
    text_layer_set_text_alignment(s_data_layer[i], GTextAlignmentCenter);
    text_layer_set_font(s_data_layer[i], s_data_font);      
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_layer[i]));

    text_layer_set_background_color(s_data_title[i], GColorClear);
    text_layer_set_text_color(s_data_title[i], GColorWhite);
    text_layer_set_text_alignment(s_data_title[i], GTextAlignmentCenter);
    
    if (i<6) // Small title fonts on the 6 field layout
      text_layer_set_font(s_data_title[i], s_title_font);
    else //Large title font on the 3 field layout
      text_layer_set_font(s_data_title[i], s_large_title_font);
    
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_title[i]));

  }
  
  // Set up top title area
    text_layer_set_background_color(s_data_layer[8], GColorClear);
    text_layer_set_text_color(s_data_layer[8], GColorWhite);
    text_layer_set_text_alignment(s_data_layer[8], GTextAlignmentCenter);
    text_layer_set_text(s_data_layer[8], "StartLine");
    text_layer_set_font(s_data_layer[8], s_title_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_layer[8])); 
  
  // Go find a screen with some fields in use
  for (currentScreen = 0; screens[currentScreen].num_fields == 0; currentScreen++)
    ;
  
  // And make it the current screen
  updatescreen(currentScreen,"00");
}


static void main_window_unload(Window *window) {
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_arrow_bitmap);
  
  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_arrow_layer);
  
  // Destroy TextLayer
  int i;
  for (i=0; i<8; i++)
  {
  text_layer_destroy(s_data_layer[i]);
  text_layer_destroy(s_data_title[i]);
  }

  text_layer_destroy(s_data_layer[8]);
}


//
// Handle incoming messages from phone
//
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information

  static char buffer[20][10];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  int j=0;
  bool foundKey;
  
  while(t != NULL) {
    foundKey = true;
    // Which key was received?
    switch(t->key) {
      case KEY_LAY_DIST:
      case KEY_LAY_BURN:
      case KEY_LAY_TIME:
      case KEY_LINE_DIST:
      case KEY_LINE_BURN:
      case KEY_LINE_ANGLE:
      case KEY_LINE_TIME:
      case KEY_SECS_TO_START:
      font = s_data_font; // Preselect a numbers-only font
      snprintf(buffer[j], sizeof(buffer[j]),"%d", (int)t->value->int32);
      // APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d Value %s", (int)t->key, (char *)buffer[j]);
      break;
      case KEY_LAY_SEL:
      font = s_data_font_alpha; // Switch to a font that can display alpha
      snprintf(buffer[j], sizeof(buffer[j]),"%s", t->value->cstring);
      // APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d Value %s", (int)t->key, (char *)buffer[j]);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      foundKey = false;
      break;
    }
    if (foundKey) // Now look through the fields on the current screen to see if it is displayed
      {
      int i;
        for (i=0; i<screens[currentScreen].num_fields; i++)
          {
          if (data_field_keys[screens[currentScreen].field_data_map[i]] == (int)t->key)
            {
            text_layer_set_font(s_data_layer[screens[currentScreen].field_layer_map[i]], font);  
            text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], buffer[j]);
          }
        }
    // Use next buffer - seems that the buffer remains in use until the data actually appears on the screen
    // So we need to use a different buffer for each message
    j++; 
    }
    t = dict_read_next(iterator); // Look for next item
    
  }
  
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
 
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
if (configuring)
    { // Set the current title back to normal
    text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorClear);
    text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
  
    configuring_field += 1; // Step to next field & wrap at the end
  if (configuring_field == screens[currentScreen].num_fields)
    configuring_field = 0;

  // Invert colours on current field title
    text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
    text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);

  }
  else { // Not configuring - just step to next screen
    do { // Search through screens to find the next one in use
  currentScreen++;
  if (currentScreen == NUM_SCREENS)
    currentScreen = 0;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"00");
  }
  
}

//
// Up Click Handler
//
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring) { // Currently only using this during configuration
    screens[currentScreen].field_data_map[configuring_field]++; //Step to the next data item in the list
    if (screens[currentScreen].field_data_map[configuring_field] == MAX_TITLES) // Wrap at the end
      screens[currentScreen].field_data_map[configuring_field] = 0;
          // Set the title
          text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                              data_titles[screens[currentScreen].field_data_map[configuring_field]]);
 
}
}


//
// Down Click Handler - just like the up click handler
//
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring) {
    screens[currentScreen].field_data_map[configuring_field]--;
    if (screens[currentScreen].field_data_map[configuring_field] < 0)
      screens[currentScreen].field_data_map[configuring_field] = MAX_TITLES -1;
          text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                              data_titles[screens[currentScreen].field_data_map[configuring_field]]);
 
}
}
  
//
// Long Select Handler
//
static void long_select_handler(ClickRecognizerRef recognizer, void *context) {
  
    if (configuring == 0) { 
      configuring = 1; // Select configuring mode
      
      configuring_field = 0;

      // Set current field title to inverse colours
      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
      
    }
    else { // End configuring, set current title back to normal display
      configuring = 0;
      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorClear);
      text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
    }
}
  
static void long_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring == 0)  // When not configuring, long up creates a new screen like the current one
  {
    int i;
    for (i=0; i<NUM_SCREENS; i++) // Go find a free screen slot
      if (screens[i].num_fields == 0)
      {
          screens[i].num_fields = screens[currentScreen].num_fields;
          int j;
          for (j=0; j<screens[currentScreen].num_fields; j++) // Copy the map from the current screen
            {
            screens[i].field_layer_map[j] = screens[currentScreen].field_layer_map[j];
            }
      screens[i].is_start = screens[currentScreen].is_start;
      break;
    }
    currentScreen = i;
    updatescreen(currentScreen, "");
    }    
  
}

static void long_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring == 0) // Long down deletes the current screen so long as it's not the last of its type
  {
    int i;
    bool ok = false;
    
    for (i=0; i<NUM_SCREENS; i++)
      if (screens[i].num_fields == screens[currentScreen].num_fields && i != currentScreen &&
         screens[i].is_start == screens[currentScreen].is_start)
      {
      ok = true; // We found another one like the one being deleted - OK to delete
      break;
    }
    
            
  if (ok) // OK to delete go find a new screen to display now
    {
    for (i=0; i < NUM_SCREENS; i++)
      if (i != currentScreen && screens[i].num_fields != 0) // Avoid selecting the screen we are deleting & unused screens
        break;
    updatescreen(i, "00");  // i is left set to the first in-use screen
    screens[currentScreen].num_fields = 0;  // Take current screen out of use. Can't do this earlier or it breaks updatescreen()
    currentScreen = i;
  }    
  
}
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_select_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 0, long_up_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, long_down_handler, NULL);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_fullscreen(s_main_window, true);
  // Set click handlers
  window_set_click_config_provider(s_main_window, click_config_provider);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  int i;

  //Read configuration back from storage - also need a way to reset to defaults
  for (i=0; i<NUM_SCREENS; i++)
      if (persist_exists(MAPPING_PKEY + i))
        persist_read_data(MAPPING_PKEY + i, &screens[i], sizeof(screens[i]));

  init();
  app_event_loop();
  deinit();
  
   //Save configuration to storage
  for (i=0; i<NUM_SCREENS; i++)
          persist_write_data(MAPPING_PKEY + i, &screens[i], sizeof(screens[i]));
}

//
// Change screen modes if needed - blank out old values, show new
//

void updatescreen(int thisScreen, char *initialValue)
  {  
    static int lastScreen = -1;  // Remember where we came from 
    int i;
//       bitmap_layer_set_bitmap(s_arrow_layer, s_arrow_bitmap);
    
  if (lastScreen != -1)  // If we had a last screen, blank out fields
    {
    for (i=0; i< screens[lastScreen].num_fields; i++)
        {
        text_layer_set_text(s_data_layer[screens[lastScreen].field_layer_map[i]], "");
        text_layer_set_text(s_data_title[screens[lastScreen].field_layer_map[i]], "");
        //text_layer_set_text(s_data_small[screens[lastScreen].field_layer_map[i]], "");
      } 
  }

    for (i=0; i<screens[thisScreen].num_fields; i++) // For now - put something in the fields
      {
      if (initialValue == NULL)
        initialValue = "00";
      text_layer_set_font(s_data_layer[screens[thisScreen].field_layer_map[i]], s_data_font);  // Switch to numeric font
      text_layer_set_text(s_data_layer[screens[thisScreen].field_layer_map[i]], initialValue);
    }
    
    // Set up titles
    for (i=0; i<screens[thisScreen].num_fields; i++)
      {
      text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], data_titles[screens[thisScreen].field_data_map[i]]);
    }
  lastScreen = thisScreen;
  }
  
