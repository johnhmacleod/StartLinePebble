#include "pebble.h"
#include "math.h"
#include "startline.h"





Window *s_main_window;
TextLayer *s_data_layer[16];
InverterLayer *inverter; //Used for title inversion
InverterLayer *dataInverter[6]; // Used for negative data

//static TextLayer *s_data_small[20];
TextLayer *s_data_title[20];
TextLayer *messageLayer;

void doupdate();
void updatescreen();


int holdThisScreen = TRANSITION_IDLE; // When this is non-zero, no auto transition
int currentScreen = 0;
int configuring = 0; // Set to 1 when configuring display
int configuring_field = 0; // Index of the title we are currently configuring
bool doubleClick = false;
bool messageClick = false;

// This structure maps the KEY values to the titles used on the screen.  
// The field_data_map array contains an index into this array so we know what data to display, so if you rearrange the order here, all stored field mappings
// will be messed up!  Always add new items to the end.
  



//Only add items to the end of this array
keyTitle keyTitles[] = {
{KEY_LAY_BURN,"Lay Burn", true, false},
{KEY_LAY_DIST,"Lay Dist", true, false},
{KEY_LAY_TIME,"Lay Time", true, false},
{KEY_LINE_BURN,"Line Burn", true, false},
{KEY_LINE_DIST, "Line Dist", true, false},
{KEY_LINE_TIME,"Line Time", true, false},
{KEY_LINE_ANGLE, "Line Angle", true, false},
{KEY_SECS_TO_START, "To Start", true, false},
{KEY_LAY_SEL, "Lay Line", true, false},
{KEY_TARGET_SPEED,"Tgt Speed", false, false}, //No
{KEY_TARGET_ANGLE,"Tgt Angle", true, false}, //No
{KEY_BOAT_SOG,"SOG", false, false},
{KEY_MARK_TURN,"TURN", false, true},
{KEY_MARK_LAY_DIST,"MkLay Dst", false, true},
{KEY_HEADING_COG,"COG", false, true},
{KEY_TIME_TO_MARK, "Mins 2 Mk", false, true},
{KEY_TACK_HEADER,"Tack Hdr" , false, true},
{KEY_MARK_BEARING, "Mk Bearng", false, true}, //No
{KEY_CURRENT_MARK, "Mark", false, true},
{KEY_LAST_TACK,"Lst Tack", false, true},
{KEY_TARGET_TACK, "Tgt Tack", false, true}, //No
{KEY_MARK_DIST, "Dist 2 Mk", false, true},
{KEY_TACK_STATE, "Tack State", false, true},
{KEY_TACK_LOG, "Tack Log", false, true},
  // New fields
{KEY_AWS, "AWS", false, true},
{KEY_AWA, "AWA", false, true},
{KEY_TWS, "TWS", false, true},
{KEY_TWA, "TWA", false, true},
{KEY_TWD, "TWD", false, true},
{KEY_DEPTH, "Depth", false, true},
{KEY_HEEL, "Heel", false, true},
{KEY_CURRENT_SPEED, "Crnt Spd", false, true},
{KEY_CURRENT_DIR, "Crnt Dir", false, true},
{KEY_BOAT_SPEED, "Boat Spd", false, true},
{KEY_VMG_WIND, "VMG Wind", false, true},
//{KEY_VMG_WIND, "VMG Wind", false, true},
};
 
int num_keytitles = sizeof(keyTitles) / sizeof(keyTitles[0]);

#define THREE_FIELD_INDEX 12
  
Screen screenDefault[4] = {
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
                                      },
                                      {3,
                                      {0,1},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      true
                                      }
                                    };

Screen screens[NUM_SCREENS] = {
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
                                      },
                                      {3,
                                      {0,1,2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      true
                                      }
                                    };  // Code relies on the rest of the array being zero to indicate screens not in use.

static GFont s_2_font, s_title_font, s_4_font, s_3_font,
              s_6_font, s_medium_title_font, s_large_title_font;

//static BitmapLayer *s_background_layer, *s_arrow_layer;
//static GBitmap *s_background_bitmap, *s_arrow_bitmap;

InverterLayer *inverter;
InverterLayer *flash;

static Layer *dataLayer, *titleLayer; /* Layers to hold all the titles & data - for Z control */

void doDataInvert(int field)
  {
  GRect a = layer_get_frame(text_layer_get_layer(s_data_layer[screens[currentScreen].field_layer_map[field]]));
  a.origin.y += a.size.h / 4;
  a.size.h -= a.size.h/4 ;
  layer_set_bounds(inverter_layer_get_layer(dataInverter[field]), a);
  
  // To revert
  // layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0));
}

void doDataRevert(int field)
  {
  layer_set_bounds(inverter_layer_get_layer(dataInverter[field]), GRect(0,0,0,0));
}

static void main_window_load(Window *window) {
  
  //APP_LOG(APP_LOG_LEVEL_ERROR, "In Main_window_load");
  // Use system font, apply it and add to Window
  s_3_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_64));
  s_2_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_59));
  s_4_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_50));
  s_6_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_47));
 
  
  s_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_large_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_medium_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  inverter = inverter_layer_create(GRect(0,0,144,168));
  layer_set_bounds(inverter_layer_get_layer(inverter),GRectZero);
  
  flash = inverter_layer_create(GRect(0,0,10,10));
  
  int jj;
  for (jj = 0; jj < 6; jj++) {
  dataInverter[jj] = inverter_layer_create(GRect(0,0,144,168));
  layer_set_bounds(inverter_layer_get_layer(dataInverter[jj]),GRectZero);  
  }
  
  // Create Display RectAngles
  
  // Six data fields & their titles
  #define SIX_FIELD_INDEX 0
  #define SIX_FIELD_MAX 5
  s_data_layer[0] = text_layer_create(GRect(0, 2, 71, 49));
  s_data_title[0] = text_layer_create(GRect(0, 49, 71, 15));

  s_data_layer[1] = text_layer_create(GRect(73, 2, 71, 49));
  s_data_title[1] = text_layer_create(GRect(73, 49, 71, 15));
  
  s_data_layer[2] = text_layer_create(GRect(0, 53, 71, 49));
  s_data_title[2] = text_layer_create(GRect(0, 102, 71, 14));
  
  s_data_layer[3] = text_layer_create(GRect(73, 53, 71, 49));
  s_data_title[3] = text_layer_create(GRect(73, 102, 71, 14));
  
  s_data_layer[4] = text_layer_create(GRect(0, 105, 71, 49));
  s_data_title[4] = text_layer_create(GRect(0, 154, 71, 15));
  
  s_data_layer[5] = text_layer_create(GRect(73, 105, 71, 49));
  s_data_title[5] = text_layer_create(GRect(73, 154, 71, 15));
  
  // Two data fields & their titles
  #define TWO_FIELD_INDEX 6
  #define TWO_FIELD_MAX 7
  s_data_layer[6] = text_layer_create(GRect(0, 2, 288, 59));
  layer_set_frame((Layer *) s_data_layer[6], GRect(0, 2, 144, 59));
  s_data_title[6] = text_layer_create(GRect(0, 64, 144, 28));
  
  s_data_layer[7] = text_layer_create(GRect(0, 79, 288, 59));
  layer_set_frame((Layer *) s_data_layer[7], GRect(0, 79, 144, 59));
  s_data_title[7] = text_layer_create(GRect(0, 140, 144, 28));

  
  // Four data fields & their titles
  #define FOUR_FIELD_INDEX 8
  #define FOUR_FIELD_MAX 11
  s_data_layer[8] = text_layer_create(GRect(0, 12, 142, 51));
  layer_set_frame((Layer *) s_data_layer[8], GRect(0, 12, 71, 51));
  s_data_title[8] = text_layer_create(GRect(0, 65, 71, 24));
  
  s_data_layer[9] = text_layer_create(GRect(73, 12, 71, 51));
  s_data_title[9] = text_layer_create(GRect(73, 65, 71, 24));
  
  s_data_layer[10] = text_layer_create(GRect(0, 91, 142, 51));
  layer_set_frame((Layer *) s_data_layer[10], GRect(0, 91, 71, 51));
  s_data_title[10] = text_layer_create(GRect(0, 144, 71, 24));
  
  s_data_layer[11] = text_layer_create(GRect(73, 91,  71, 51));
  s_data_title[11] = text_layer_create(GRect(73, 144, 71, 24));
  
  // Three fields - One big, two small
  //#define THREE_FIELD_INDEX 12
  #define THREE_FIELD_MAX 14
  s_data_layer[12] = text_layer_create(GRect(0, 10, 432, 65));
  layer_set_frame((Layer *) s_data_layer[12], GRect(0, 10, 144, 65));
  s_data_title[12] = text_layer_create(GRect(0, 75, 144, 28));

  s_data_layer[13] = text_layer_create(GRect(0, 91, 150, 51));
  layer_set_frame((Layer *) s_data_layer[13], GRect(0, 91, 71, 51));
  s_data_title[13] = text_layer_create(GRect(0, 144, 71, 24));
  
  
  //s_data_layer[14] = text_layer_create(GRect(73, 91, 142, 51));
  //layer_set_frame((Layer *) s_data_layer[14], GRect(73, 91, 71, 51));
  s_data_layer[14] = text_layer_create(GRect(73, 91,  71, 51));
  s_data_title[14] = text_layer_create(GRect(73, 144, 71, 24));
  
  
  // Top title
  s_data_layer[TITLE_INDEX] = text_layer_create(GRect(0, 0, 144, 16));
  
  

  // Set up top title area
    text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorBlack);
    text_layer_set_text_color(s_data_layer[TITLE_INDEX], GColorWhite);
    text_layer_set_text_alignment(s_data_layer[TITLE_INDEX], GTextAlignmentCenter);
    text_layer_set_text(s_data_layer[TITLE_INDEX], "StartLine");
    text_layer_set_font(s_data_layer[TITLE_INDEX], s_title_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_layer[TITLE_INDEX])); 
 
  window_set_background_color(window, GColorBlack);
  // Set up the messgage layer
  messageLayer = text_layer_create(GRect(10,30,124,120));
  text_layer_set_background_color(messageLayer, GColorClear);
  text_layer_set_text_color(messageLayer, GColorWhite);
  text_layer_set_text_alignment(messageLayer, GTextAlignmentCenter);
  text_layer_set_font(messageLayer, s_large_title_font);
  
  

  
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
    text_layer_set_overflow_mode(s_data_layer[i], GTextOverflowModeWordWrap);
    layer_add_child(dataLayer, text_layer_get_layer(s_data_layer[i]));
    text_layer_set_background_color(s_data_title[i], GColorClear);
    text_layer_set_text_color(s_data_title[i], GColorWhite);
    text_layer_set_text_alignment(s_data_title[i], GTextAlignmentCenter);
    
    if (i >= SIX_FIELD_INDEX && i <= SIX_FIELD_MAX) // Small title fonts on the 6 field layout
      {
      text_layer_set_font(s_data_layer[i], s_6_font);    
      text_layer_set_font(s_data_title[i], s_title_font);
    }
    else if (i >= TWO_FIELD_INDEX && i <= TWO_FIELD_MAX) // This is 2 fields
      {
      text_layer_set_font(s_data_layer[i], s_2_font); 
      text_layer_set_font(s_data_title[i], s_large_title_font);
    }

    else if (i >= FOUR_FIELD_INDEX && i <= FOUR_FIELD_MAX) // 4 field layout
      {
      text_layer_set_font(s_data_layer[i], s_4_font); 
      text_layer_set_font(s_data_title[i], s_medium_title_font);
    }
    else if (i >= THREE_FIELD_INDEX && i <= THREE_FIELD_MAX)
      {
      if (i == THREE_FIELD_INDEX) // First field is big
        {
        text_layer_set_font(s_data_layer[i], s_3_font); 
        text_layer_set_font(s_data_title[i], s_large_title_font);
      } else
        {
        text_layer_set_font(s_data_layer[i], s_4_font);    
        text_layer_set_font(s_data_title[i], s_medium_title_font);        
      }
    }      
   
    layer_add_child(titleLayer, text_layer_get_layer(s_data_title[i]));
    
  }
  
 layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter));


  int ii;
  for (ii = 0; ii < 6; ii++) {
     layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(dataInverter[ii]));
  }
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(messageLayer)); 

   layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(flash));

  
  // Go find a screen with some fields in use
  for (currentScreen = 0; screens[currentScreen].num_fields == 0; currentScreen++)
    ;

  // And make it the current screen
  updatescreen(currentScreen,"00");
}


void screenMessage(char *message)
  {
  static int preHoldThisScreen = TRANSITION_IDLE;
  
  if (*message != '\0') // Setting a message
    {
    updatescreen(-1,""); //Blank
    layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0)); // Turn off the inverter layer
    preHoldThisScreen = holdThisScreen; // Remember what it was
    holdThisScreen = -1;
    messageClick = true;
    text_layer_set_text(messageLayer, message);
  }
  else //Clearing a message
    {
    messageClick = false;
    holdThisScreen = preHoldThisScreen; // Restore it to what it was
    text_layer_set_text(messageLayer,"");  
    updatescreen(currentScreen, "");
  }
  
}

static void main_window_unload(Window *window) {
  int i;
  for (i=0; i<TITLE_INDEX; i++)
    {
    text_layer_destroy(s_data_layer[i]);
    text_layer_destroy(s_data_title[i]);
  }
  
  layer_destroy(dataLayer);
  layer_destroy(titleLayer);
  text_layer_destroy(messageLayer);
  text_layer_destroy(s_data_layer[TITLE_INDEX]);  
  
  inverter_layer_destroy(inverter);
  inverter_layer_destroy(flash);
  
  int jj;
  for (jj = 0; jj < 6; jj++) {
    inverter_layer_destroy(dataInverter[jj]);
  }
  
  fonts_unload_custom_font(s_2_font);
  fonts_unload_custom_font(s_4_font);
  fonts_unload_custom_font(s_6_font);
}


void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    //Free the memoery used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(PropertyAnimation **anim, Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
     
    //Declare animation      
    *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) *anim, duration);
    animation_set_delay((Animation*) *anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) *anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) *anim);
}

// setField displays some data in a field & handles the reverse font

void setField(int i /* Field Index */,  bool negNum, char* value)
  {
  static PropertyAnimation *pa1[6] = {NULL}, *pa2[6] = {NULL}; //Arrays to cope with 6 fields
    {
    static GSize textContent;
    static GRect gfrom, gto, gframe;

    text_layer_set_text_alignment(s_data_layer[screens[currentScreen].field_layer_map[i]], GTextAlignmentLeft);
    text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], value); // This line only
    textContent = text_layer_get_content_size(s_data_layer[screens[currentScreen].field_layer_map[i]]);
    gfrom = layer_get_bounds((Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]]);
    gframe = layer_get_frame((Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]]);
    
    // APP_LOG(APP_LOG_LEVEL_INFO, "gframe.size.w=%d textContent.w=%d i=%d", gframe.size.w, textContent.w, i);
    if (textContent.w > gframe.size.w) // Overflowed
      {
      // APP_LOG(APP_LOG_LEVEL_INFO, "setfield11 value=%s", value);
      if ( (pa1[i] == NULL || !animation_is_scheduled((Animation*)pa1[i])) 
          && (pa2[i] == NULL || !animation_is_scheduled((Animation*) pa2[i]))) // We are not already animating
        {
        // APP_LOG(APP_LOG_LEVEL_INFO, "setField 10");
        gto = gfrom;
        gfrom.origin.x = 0;
        gto.origin.x = (gframe.size.w - textContent.w)/2; //Work out har far left to move animate the text
        // APP_LOG(APP_LOG_LEVEL_INFO, "setField11 gfrom.x=%d gfrom.y=%d", gfrom.origin.x, gfrom.origin.y);
        // APP_LOG(APP_LOG_LEVEL_INFO, "setField11 gto.x=%d gto.y=%d", gto.origin.x, gto.origin.y);
        int tim = (int)(-2000.0 * ((float)gto.origin.x) / 20.0);
        animate_layer_bounds(&pa1[i], (Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]], &gfrom, &gto, tim, 0);
        animate_layer_bounds(&pa2[i], (Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]], &gto, &gfrom, tim, tim);
        }
      else
        {
        //APP_LOG(APP_LOG_LEVEL_INFO, "Already scheduled screen=%d i=%d %d %d %d %d", currentScreen, i, (int)pa1[i], animation_is_scheduled((Animation*)pa1[i]), (int)pa2[i], animation_is_scheduled((Animation*)pa2[i]) );
        text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], value); // Animation running - just set the text
      }
    }
    else // We need to redraw the text centred in the reset bounds
      {
      // APP_LOG(APP_LOG_LEVEL_INFO, "setfield11 value=%s", value);
      GRect bF = layer_get_bounds((Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]]);
      GRect fF = layer_get_frame((Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]]);
      if (bF.size.w != fF.size.w) // is there extra space?
        {
        bF.origin.x = -(bF.size.w / 2 - fF.size.w / 2) /2;
        // APP_LOG(APP_LOG_LEVEL_INFO, "origin.x =%d", bF.origin.x);
        layer_set_bounds((Layer *)s_data_layer[screens[currentScreen].field_layer_map[i]], bF); // Centre the Bounds below the Frame
      }
      text_layer_set_text_alignment(s_data_layer[screens[currentScreen].field_layer_map[i]], GTextAlignmentCenter); //Should be Center but need to work out how!
      text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], value); // This line only
    }
    
  }
  if (negNum) // Did we get a negative number
  {
    doDataInvert(i);
  } 
  else // No, positive number
  {
    doDataRevert(i);
  }
}

//
// Returns true if it can find the current key in a large field on the screen. Used to work out how to format data
//

bool isBigField(int key)
  {
  int i;
  
  int nf = screens[currentScreen].num_fields;
  for (i = 0; i < nf; i++)
    if (keyTitles[screens[currentScreen].field_data_map[i]].key == key)
      break;
  if (i == nf) // Didn't find it
    return false;
  if (nf == 2 || /* nf == 4 || */ (nf == 3 && i == 0))
    return true;
  else
    return false;
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}


// Blank Normal
void blankNormal(int screen, int field)
  {
    text_layer_set_text(s_data_layer[screens[screen].field_layer_map[field]], "");
    text_layer_set_background_color(s_data_layer[screens[screen].field_layer_map[field]], GColorClear);
    text_layer_set_text_color(s_data_layer[screens[screen].field_layer_map[field]], GColorWhite);
}



static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_select_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 0, long_up_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, long_down_handler, NULL);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 3, 200, true, select_multi_click_handler);
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
  
   //APP_LOG(APP_LOG_LEVEL_INFO, "Inbox: %d OutBox %d", (int)app_message_inbox_size_maximum(),(int)app_message_outbox_size_maximum());
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

//
// Main
//
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
// APP_LOG(APP_LOG_LEVEL_DEBUG, "updatescreen free=%d", (int)heap_bytes_free()); 
//  for (int xx = -360; xx <= 360; xx+=5)
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "x=%d  cos(x)=%d 5.4*cos(x)=%d.%02d", xx, 
//            (int)(1000*mycos(xx/180.0 * M_PI)), abs((int)(5.4 * mycos(xx/180.0 * M_PI))), abs((int)(540 * mycos(xx/180.0 * M_PI))%100));
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
        text_layer_set_text(s_data_title[screens[lastScreen].field_layer_map[i]], "");
        text_layer_set_background_color(s_data_title[screens[lastScreen].field_layer_map[i]], GColorClear); 
        text_layer_set_background_color(s_data_layer[screens[lastScreen].field_layer_map[i]], GColorClear);
        text_layer_set_text_color(s_data_layer[screens[lastScreen].field_layer_map[i]], GColorWhite);
      } 
  }
  
if (thisScreen != -1) // -1 if there is no screen to go to -- just blanking out lastScreen prior to default restore;
  {
    for (i=0; i<screens[thisScreen].num_fields; i++) // For now - put something in the fields
      {
      if (initialValue != NULL)
        //text_layer_set_text(s_data_layer[screens[thisScreen].field_layer_map[i]], initialValue);
        setField(i, false, initialValue);
    }
    
    // Set up titles
    for (i=0; i<screens[thisScreen].num_fields; i++)
      {
      if (screens[thisScreen].field_data_map[i] >= num_keytitles)
        screens[thisScreen].field_data_map[i] = 0;
      text_layer_set_background_color(s_data_title[screens[thisScreen].field_layer_map[i]], GColorClear);
      text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], keyTitles[screens[thisScreen].field_data_map[i]].title);
    }
  lastScreen = thisScreen;
  int jj;
  for (jj = 0; jj < 6; jj++) {
    doDataRevert(jj);
  }
  static char buf[50];
  snprintf(buf, sizeof(buf), "StartLine    Screen %d", thisScreen + 1);
  text_layer_set_text(s_data_layer[TITLE_INDEX], buf);
  }
}
  
