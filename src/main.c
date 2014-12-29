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
    #define KEY_TARGET_SPEED 16
    #define KEY_TARGET_ANGLE 17


  
#define MAPPING_PKEY  100

  // This is a simple menu layer
static SimpleMenuLayer *simple_menu_layer;

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[1];

#define NUM_MENU_ITEMS 3
#define NUM_MENU_SECTIONS 1
// Each section is composed of a number of menu items
static SimpleMenuItem menu_items[NUM_MENU_ITEMS];
  
static Window *s_main_window;
static TextLayer *s_data_layer[20];
static TextLayer *s_data_small[20];
static TextLayer *s_data_title[20];
static TextLayer *messageLayer;

void doupdate();
void updatescreen();


static int currentScreen = 0;
static int configuring = 0; // Set to 1 when configuring display
static int configuring_field = 0; // Index of the title we are currently configuring
static bool doubleClick = false;

#define MAX_TITLES  12 //Number of elements in title array

static int data_field_keys[] = {KEY_LAY_BURN, KEY_LAY_DIST, KEY_LAY_TIME, KEY_LINE_BURN, KEY_LINE_DIST, KEY_LINE_TIME,
                                   KEY_LINE_ANGLE, KEY_SECS_TO_START, KEY_LAY_SEL, KEY_TARGET_SPEED, KEY_TARGET_ANGLE, KEY_BOAT_SPEED };
static char *data_titles[] = {"Lay Burn", "Lay Dist", "Lay Time", "Line Burn", "Line Dist", "Line Time",
                               "Line Angle", "To Start", "Lay Line", "Tgt Speed", "Tgt Angle", "Boat Spd"};


typedef struct 
{
  int num_fields; // How many fields on this screen
  int field_data_map[6]; // How do these fields map to data
  int field_layer_map[6]; // How do these fields map to display real estate
  int field_small_layer_map[6]; // And small font display real estate when needed - smaller font needs placing differently
  int is_start; // true if this is a start related screen
} Screen;

#define NUM_SCREENS 10

static Screen screenDefault[3] = {
                                      {6,
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      true
                                        },
                                      {4,
                                      {0,1,2,3},
                                      {8,9,10,11},
                                      {8,9,10,11},
                                      true
                                      },
                                      {2,
                                      {2,3},
                                      {6,7},
                                      {6,7},
                                      true
                                      }
                                    };

static Screen screens[NUM_SCREENS] = {
                                      {6,
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      true
                                        },
                                      {4,
                                      {0,1,2,3},
                                      {8,9,10,11},
                                      {8,9,10,11},
                                      true
                                      },
                                      {2,
                                      {2,3},
                                      {6,7},
                                      {6,7},
                                      true
                                      }
                                    };  // Code relies on the rest of the array being zero to indicate screens not in use.


static GFont s_data_font, s_data_font_alpha, s_title_font, s_small_data_font, s_medium_title_font, s_large_title_font, font;

static BitmapLayer *s_background_layer, *s_arrow_layer;
static GBitmap *s_background_bitmap, *s_arrow_bitmap;

static void main_window_load(Window *window) {
  
  //APP_LOG(APP_LOG_LEVEL_ERROR, "In Main_window_load");
  // Use system font, apply it and add to Window
  s_data_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  s_data_font_alpha = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_large_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_medium_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
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
  #define SIX_FIELD_INDEX 0
  s_data_layer[0] = text_layer_create(GRect(0, 1, 71, 49));
  s_data_small[0] = text_layer_create(GRect(0, 15, 71, 34));
  s_data_title[0] = text_layer_create(GRect(0, 49, 71, 15));

  s_data_layer[1] = text_layer_create(GRect(73, 1, 71, 49));
  s_data_small[1] = text_layer_create(GRect(73, 15, 71, 34));
  s_data_title[1] = text_layer_create(GRect(73, 49, 71, 15));
  
  s_data_layer[2] = text_layer_create(GRect(0, 53, 71, 49));
  s_data_small[2] = text_layer_create(GRect(0, 68, 71, 34));
  s_data_title[2] = text_layer_create(GRect(0, 102, 71, 14));
  
  s_data_layer[3] = text_layer_create(GRect(73, 53, 71, 49));
  s_data_small[3] = text_layer_create(GRect(73, 68, 71, 34));
  s_data_title[3] = text_layer_create(GRect(73, 102, 71, 14));
  
  s_data_layer[4] = text_layer_create(GRect(0, 106, 71, 49));
  s_data_small[4] = text_layer_create(GRect(0, 121, 71, 34));
  s_data_title[4] = text_layer_create(GRect(0, 154, 71, 15));
  
  s_data_layer[5] = text_layer_create(GRect(73, 106, 71, 49));
  s_data_small[5] = text_layer_create(GRect(73, 121, 71, 34));
  s_data_title[5] = text_layer_create(GRect(73, 154, 71, 15));
  
  // Two data fields & their titles
  #define TWO_FIELD_INDEX 6
  s_data_layer[6] = text_layer_create(GRect(0, 10, 144, 50));
  s_data_small[6] = text_layer_create(GRect(0, 10, 144, 50)); // Plenty of space!
  s_data_title[6] = text_layer_create(GRect(12, 61, 120, 28));
  
  s_data_layer[7] = text_layer_create(GRect(0, 89, 144, 50));
  s_data_small[7] = text_layer_create(GRect(0, 89, 144, 50));
  s_data_title[7] = text_layer_create(GRect(12, 140, 120, 28));

  
  // Four data fields & their titles
  #define FOUR_FIELD_INDEX 8
  s_data_layer[8] = text_layer_create(GRect(0, 12, 71, 55));
  s_data_small[8] = text_layer_create(GRect(0, 25, 71, 34));
  s_data_title[8] = text_layer_create(GRect(0, 65, 71, 24));
  s_data_layer[9] = text_layer_create(GRect(73, 13, 71, 55));
  s_data_small[9] = text_layer_create(GRect(73, 25, 71, 34));
  s_data_title[9] = text_layer_create(GRect(73, 65, 71, 24));
 
  s_data_layer[10] = text_layer_create(GRect(0, 89, 71, 55));
  s_data_small[10] = text_layer_create(GRect(0, 104, 71, 34));
  s_data_title[10] = text_layer_create(GRect(0, 144, 71, 24));
  s_data_layer[11] = text_layer_create(GRect(73, 89, 71, 55));
  s_data_small[11] = text_layer_create(GRect(73, 104, 71, 34));
  s_data_title[11] = text_layer_create(GRect(73, 144, 71, 24));

  
  // Top title
  #define TITLE_INDEX 12
  s_data_layer[TITLE_INDEX] = text_layer_create(GRect(0, 0, 144, 16));
  
  

  // Set up top title area
    text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorBlack);
    text_layer_set_text_color(s_data_layer[TITLE_INDEX], GColorWhite);
    text_layer_set_text_alignment(s_data_layer[TITLE_INDEX], GTextAlignmentCenter);
    text_layer_set_text(s_data_layer[TITLE_INDEX], "StartLine");
    text_layer_set_font(s_data_layer[TITLE_INDEX], s_title_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_layer[TITLE_INDEX])); 
  
  // Set up the messgage layer
  messageLayer = text_layer_create(GRect(10,30,124,120));

                                   
  text_layer_set_background_color(messageLayer, GColorClear);
  text_layer_set_text_color(messageLayer, GColorWhite);
  text_layer_set_text_alignment(messageLayer, GTextAlignmentCenter);
  text_layer_set_font(messageLayer, s_large_title_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(messageLayer)); 
  
  
  static Layer *dataLayer, *titleLayer;
  
  titleLayer = layer_create(GRect(0, 0, 144, 168));
  layer_insert_below_sibling(titleLayer, (Layer *)s_data_layer[TITLE_INDEX]);
  
  dataLayer = layer_create(GRect(0, 0, 144, 168));
  layer_insert_below_sibling(dataLayer, titleLayer); 

  int i;
  for (i =0; i < TITLE_INDEX; i++)
    {
    text_layer_set_background_color(s_data_layer[i], GColorClear);
    text_layer_set_text_color(s_data_layer[i], GColorWhite);
    text_layer_set_text_alignment(s_data_layer[i], GTextAlignmentCenter);
    text_layer_set_font(s_data_layer[i], s_data_font);     
    layer_add_child(dataLayer, text_layer_get_layer(s_data_layer[i]));
    
    text_layer_set_background_color(s_data_small[i], GColorClear);
    text_layer_set_text_color(s_data_small[i], GColorWhite);
    text_layer_set_text_alignment(s_data_small[i], GTextAlignmentCenter);
    text_layer_set_font(s_data_small[i], s_small_data_font);
    layer_add_child(dataLayer, text_layer_get_layer(s_data_small[i]));

    text_layer_set_background_color(s_data_title[i], GColorClear);
    text_layer_set_text_color(s_data_title[i], GColorWhite);
    text_layer_set_text_alignment(s_data_title[i], GTextAlignmentCenter);
    
    if (i<(TWO_FIELD_INDEX)) // Small title fonts on the 6 field layout
      text_layer_set_font(s_data_title[i], s_title_font);
    else if (i<FOUR_FIELD_INDEX)
      text_layer_set_font(s_data_title[i], s_large_title_font);
    else //Large title font on the 2 & 4 field layout
      text_layer_set_font(s_data_title[i], s_medium_title_font);
   
    //layer_insert_above_sibling(text_layer_get_layer(s_data_title[i]), text_layer_get_layer(s_data_layer[i])); 
    layer_add_child(titleLayer, text_layer_get_layer(s_data_title[i]));
 
    //lastLayer = s_data_layer[i];

  }
  


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
  for (i=0; i<TITLE_INDEX; i++)
  {
  text_layer_destroy(s_data_layer[i]);
  text_layer_destroy(s_data_title[i]);
//  if (i < TWO_FIELD_INDEX)
    text_layer_destroy(s_data_small[i]);
  }

  text_layer_destroy(s_data_layer[TITLE_INDEX]);
  
  simple_menu_layer_destroy(simple_menu_layer);
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
  bool foundKey, negNum;
  
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
      case KEY_BOAT_SPEED:
      snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      negNum = ((int)t->value->int32 < 0);
      
      font = s_data_font; // Preselect a numbers-only font
  
//      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d Value %s", (int)t->key, (char *)buffer[j]);
      break;
      case KEY_LAY_SEL:
      negNum = false;
      font = s_data_font_alpha; // Switch to a font that can display alpha
      snprintf(buffer[j], sizeof(buffer[j]),"%s", t->value->cstring);
      // APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d Value %s", (int)t->key, (char *)buffer[j]);
      break;
    default:
      // APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      foundKey = false;
      break;
    }
    if (foundKey) // Now look through the fields on the current screen to see if it is displayed
      {
      int i;
        for (i=0; i<screens[currentScreen].num_fields; i++)
          {
          if (data_field_keys[screens[currentScreen].field_data_map[i]] == (int)t->key) // Did we find a match?
            {
            text_layer_set_font(s_data_layer[screens[currentScreen].field_layer_map[i]], font);  //Set to the chosen (num/alpha) font)
            if (strlen(buffer[j]) <= 2 || screens[currentScreen].num_fields == 2) // Short string, or wide fields available
              {
              text_layer_set_text(s_data_small[screens[currentScreen].field_layer_map[i]], ""); //Blank the small font field in case
              text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], buffer[j]); //Set the regular font field
              if (negNum) // Did we get a negative number
                {
                text_layer_set_background_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorClear);
                text_layer_set_text_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorWhite);
                text_layer_set_background_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorWhite);
                text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorBlack);
              } else // No, positive number
                {
                text_layer_set_background_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorClear);
                text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorWhite);
                text_layer_set_background_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorClear);
                text_layer_set_text_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorWhite);
              }
            }
            else // Long data >2 chars
              {
              text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], ""); // Blank normal font field & reset inverse in case
              text_layer_set_background_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorClear);
              text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[i]], GColorWhite);
              text_layer_set_text(s_data_small[screens[currentScreen].field_layer_map[i]], buffer[j]);
              if (negNum)
                {
                text_layer_set_background_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorWhite);
                text_layer_set_text_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorBlack);
              } else
                {
                text_layer_set_background_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorClear);
                text_layer_set_text_color(s_data_small[screens[currentScreen].field_layer_map[i]], GColorWhite);
              }
            }
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

//
//
// ****************************
// All the button handlers here
// ****************************
//

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
         // APP_LOG(APP_LOG_LEVEL_INFO, "SELECT click");
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
else {
 
}

  
}

//
// Up Click Handler
//
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring) { 
    screens[currentScreen].field_data_map[configuring_field]++; //Step to the next data item in the list
    if (screens[currentScreen].field_data_map[configuring_field] == MAX_TITLES) // Wrap at the end
      screens[currentScreen].field_data_map[configuring_field] = 0;
          // Set the title
          text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                              data_titles[screens[currentScreen].field_data_map[configuring_field]]);
 
}
  else if (doubleClick) //Confirming reset to default
    {
    doubleClick = false;
    text_layer_set_text(messageLayer,"");  
      
    int i;
    for (i=0; i<3; i++) // Step through 3 default screens
      {
          screens[i].num_fields = screenDefault[i].num_fields;
          int j;
          for (j=0; j<screenDefault[i].num_fields; j++) // Copy the maps from the default screen
            {
            screens[i].field_layer_map[j] = screenDefault[i].field_layer_map[j];
            screens[i].field_data_map[j] = screenDefault[i].field_data_map[j];
            }
        screens[i].is_start = screens[i].is_start;
    }
    for (i=3; i<NUM_SCREENS; i++)
      screens[i].num_fields = 0;
    
    currentScreen = 0;
    updatescreen(-2, "00");
  }
  else  // Step to next screen
    {
    do { // Search through screens to find the next one in use
  currentScreen--;
  if (currentScreen <0)
    currentScreen = NUM_SCREENS - 1;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"00");
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
  else if (doubleClick) // Cancelling reset to default
  {
    doubleClick = false;
    text_layer_set_text(messageLayer,"");  
    updatescreen(currentScreen, "00");
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
      updatescreen(currentScreen,"00"); // This should not be needed!
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
    if (i < NUM_SCREENS)
      {
      currentScreen = i;
      updatescreen(currentScreen, "");
      }    
    else
      vibes_long_pulse();
  } 
}

static void long_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring == 0) // Long down deletes the current screen so long as it's not the last one
  {
    int i;
    int ok = 0;  //Always allow delete
    
    for (i=0; i<NUM_SCREENS; i++)
      {
      if (screens[i].num_fields != 0)
        ok++;
    }
    
//      if (screens[i].num_fields == screens[currentScreen].num_fields && i != currentScreen &&
//         screens[i].is_start == screens[currentScreen].is_start)
//      {
//      ok = true; // We found another one like the one being deleted - OK to delete
//      break;
//    }
    
            
  if (ok > 1) // OK to delete go find a new screen to display now
    {
    int i;
    for (i=0; i < NUM_SCREENS; i++)
      if (i != currentScreen && screens[i].num_fields != 0) // Avoid selecting the screen we are deleting & unused screens
        break;
    updatescreen(i, "00");  // i is left set to the first in-use screen
    screens[currentScreen].num_fields = 0;  // Take current screen out of use. Can't do this earlier or it breaks updatescreen()
    currentScreen = i;
  }
    else
      vibes_long_pulse();
}
  
}

static void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!configuring)
  {
    updatescreen(-1,""); //Blank
  doubleClick = true;
  text_layer_set_text(messageLayer, "Press UP to reset to default, DOWN otherwise");
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_select_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 0, long_up_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, long_down_handler, NULL);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 2, 200, true, select_multi_click_handler);
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
  
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED); // Beware!! Increased power usage, but much better responsiveness

  app_event_loop();

  app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

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
  if (thisScreen == -2) //Act as if we are starting from scratch -- there is no last screen
    {
    lastScreen = -1;
    thisScreen = 0;
  }
  
  if (lastScreen != -1)  // If we had a last screen, blank out fields
    {
    for (i=0; i< screens[lastScreen].num_fields; i++)
        {
        text_layer_set_text(s_data_layer[screens[lastScreen].field_layer_map[i]], "");
        text_layer_set_text(s_data_small[screens[lastScreen].field_layer_map[i]], "");
        text_layer_set_text(s_data_title[screens[lastScreen].field_layer_map[i]], "");
        text_layer_set_background_color(s_data_title[screens[lastScreen].field_layer_map[i]], GColorClear); 
        text_layer_set_background_color(s_data_layer[screens[lastScreen].field_layer_map[i]], GColorClear);
        text_layer_set_text_color(s_data_layer[screens[lastScreen].field_layer_map[i]], GColorWhite);
        text_layer_set_background_color(s_data_small[screens[lastScreen].field_layer_map[i]], GColorClear);
        text_layer_set_text_color(s_data_small[screens[lastScreen].field_layer_map[i]], GColorWhite);
      } 
  }
  
if (thisScreen != -1) // -1 if there is no screen to go to -- just blanking out lastScreen prior to default restore;
  {
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
      text_layer_set_background_color(s_data_title[screens[thisScreen].field_layer_map[i]], GColorBlack);
      text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], data_titles[screens[thisScreen].field_data_map[i]]);
    }
  lastScreen = thisScreen;
  static char buf[50];
  snprintf(buf, sizeof(buf), "StartLine    Screen %d", thisScreen + 1);
  text_layer_set_text(s_data_layer[TITLE_INDEX], buf);
  }
}
  
