#include "pebble.h"
#include "math.h"

    #define KEY_LAY_TIME     19
    #define KEY_LAY_DIST  1
    #define KEY_LAY_BURN  2
    #define KEY_LAY_SEL	 3 //Selected Lay Line at Start
    #define KEY_LINE_TIME  4
    #define KEY_LINE_BURN  5
    #define KEY_LINE_DIST  6
    #define KEY_LINE_ANGLE  7
    #define KEY_SECS_TO_START  8
    #define KEY_START_SEQUENCE 9 // Not used
    #define KEY_CURRENT_MARK 100
    #define KEY_BOAT_SPEED 11
    #define KEY_MARK_TURN 52
    #define KEY_WIND_SHIFT 13 // Not used
    #define KEY_LAST_TACK 58
    #define KEY_MARK_LAY_DIST 54
    #define KEY_TARGET_TACK 59
    #define KEY_TARGET_SPEED 17
    #define KEY_TARGET_ANGLE 18
    #define KEY_BOAT_SPEED_MARK 50
    #define KEY_MARK_BEARING 56
    #define KEY_TIME_TO_MARK 53
    #define KEY_TACK_HEADER  55
    #define KEY_HEADING_COG  51
    #define KEY_ROUTE_NUMBER 57

    

  
#define MAPPING_PKEY  100 // Start key number for persistent storage
  
static Window *s_main_window;
static TextLayer *s_data_layer[20];
static TextLayer *s_data_small[20];
static TextLayer *s_data_title[20];
static TextLayer *messageLayer;

void doupdate();
void updatescreen();

#define TRANSITION_IDLE 5 //Number of seconds of idle before we do auto transition
static int holdThisScreen = TRANSITION_IDLE; // When this is non-zero, no auto transition
static int currentScreen = 0;
static int configuring = 0; // Set to 1 when configuring display
static int configuring_field = 0; // Index of the title we are currently configuring
static bool doubleClick = false;

// This structure maps the KEY values to the titles used on the screen.  
// The field_data_map array contains an index into this array so we know what data to display, so if you rearrange the order here, all stored field mappings
// will be messed up!  Always add new items to the end.
  
typedef struct
  {
  int key;
  char *title;
  bool preStart;
} keyTitle;

#define NUM_KEYTITLES ((int)sizeof(keyTitles)/(int)sizeof(keyTitles[0]))
  
static keyTitle keyTitles[] = {
{KEY_LAY_BURN,"Lay Burn", true},
{KEY_LAY_DIST,"Lay Dist", true},
{KEY_LAY_TIME,"Lay Time", true},
{KEY_LINE_BURN,"Line Burn", true},
{KEY_LINE_DIST, "Line Dist", true},
{KEY_LINE_TIME,"Line Time", true},
{KEY_LINE_ANGLE, "Line Angle", true},
{KEY_SECS_TO_START, "To Start", true},
{KEY_LAY_SEL, "Lay Line", true},
{KEY_TARGET_SPEED,"Tgt Speed", false},
{KEY_TARGET_ANGLE,"Tgt Angle", true},
{KEY_BOAT_SPEED,"Boat Spd", false},
{KEY_MARK_TURN,"Trn to Mk", false},
{KEY_MARK_LAY_DIST,"MkLay Dst", false},
{KEY_HEADING_COG,"COG", false},
{KEY_TIME_TO_MARK, "Mins 2 Mk", false},
{KEY_TACK_HEADER,"Tack Hdr" , false},
{KEY_MARK_BEARING, "Mk Bearng", false},
{KEY_CURRENT_MARK, "Mark", false},
{KEY_LAST_TACK,"Lst Tack", false},
{KEY_TARGET_TACK, "Tgt Tack", false}
};
 

typedef struct 
{
  int num_fields; // How many fields on this screen
  int field_data_map[6]; // How do these fields map to data
  int field_layer_map[6]; // How do these fields map to display real estate
  int field_small_layer_map[6]; // And small font display real estate when needed - smaller font needs placing differently
  int is_start; // true if this is a start related screen
} Screen;

#define NUM_SCREENS 10
#define THREE_FIELD_INDEX 12
  
static Screen screenDefault[4] = {
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
                                      },
                                      {3,
                                      {0,1,2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      true
                                      }
                                    };  // Code relies on the rest of the array being zero to indicate screens not in use.

static GFont s_2_font, s_2_font_small, s_title_font, s_4_font, s_4_font_small, 
              s_6_font, s_6_font_small, s_medium_title_font, s_large_title_font;

static BitmapLayer *s_background_layer, *s_arrow_layer;
static GBitmap *s_background_bitmap, *s_arrow_bitmap;

static Layer *dataLayer, *titleLayer; /* Layers to hold all the titles & data - for Z control */

static void main_window_load(Window *window) {
  
  //APP_LOG(APP_LOG_LEVEL_ERROR, "In Main_window_load");
  // Use system font, apply it and add to Window
  s_2_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_59));
  s_2_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_59));
  s_4_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_50));
  s_4_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_50));
  s_6_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_47));
  s_6_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_47));
 
  
  s_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_large_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_medium_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  
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
  #define SIX_FIELD_MAX 5
  s_data_layer[0] = text_layer_create(GRect(0, 1, 71, 49));
  s_data_small[0] = text_layer_create(GRect(0, 1, 71, 49));
  s_data_title[0] = text_layer_create(GRect(0, 49, 71, 15));

  s_data_layer[1] = text_layer_create(GRect(73, 1, 71, 49));
  s_data_small[1] = text_layer_create(GRect(73, 1, 71, 49));
  s_data_title[1] = text_layer_create(GRect(73, 49, 71, 15));
  
  s_data_layer[2] = text_layer_create(GRect(0, 53, 71, 49));
  s_data_small[2] = text_layer_create(GRect(0, 53, 71, 49));
  s_data_title[2] = text_layer_create(GRect(0, 102, 71, 14));
  
  s_data_layer[3] = text_layer_create(GRect(73, 53, 71, 49));
  s_data_small[3] = text_layer_create(GRect(73, 53, 71, 49));
  s_data_title[3] = text_layer_create(GRect(73, 102, 71, 14));
  
  s_data_layer[4] = text_layer_create(GRect(0, 105, 71, 49));
  s_data_small[4] = text_layer_create(GRect(0, 106, 71, 49));
  s_data_title[4] = text_layer_create(GRect(0, 154, 71, 15));
  
  s_data_layer[5] = text_layer_create(GRect(73, 105, 71, 49));
  s_data_small[5] = text_layer_create(GRect(73, 106, 71, 49));
  s_data_title[5] = text_layer_create(GRect(73, 154, 71, 15));
  
  // Two data fields & their titles
  #define TWO_FIELD_INDEX 6
  #define TWO_FIELD_MAX 7
  s_data_layer[6] = text_layer_create(GRect(0, 2, 144, 59));
  s_data_small[6] = text_layer_create(GRect(0, 2, 144, 59)); // Plenty of space!
  s_data_title[6] = text_layer_create(GRect(0, 64, 144, 28));
  
  s_data_layer[7] = text_layer_create(GRect(0, 79, 144, 59));
  s_data_small[7] = text_layer_create(GRect(0, 79, 144, 59));
  s_data_title[7] = text_layer_create(GRect(0, 140, 144, 28));

  
  // Four data fields & their titles
  #define FOUR_FIELD_INDEX 8
  #define FOUR_FIELD_MAX 11
  s_data_layer[8] = text_layer_create(GRect(0, 8, 71, 61));
  s_data_small[8] = text_layer_create(GRect(0, 8, 71, 61));
  s_data_title[8] = text_layer_create(GRect(0, 65, 71, 24));
  
  s_data_layer[9] = text_layer_create(GRect(73, 8, 71, 61));
  s_data_small[9] = text_layer_create(GRect(73, 8, 71, 61));
  s_data_title[9] = text_layer_create(GRect(73, 65, 71, 24));
  
  s_data_layer[10] = text_layer_create(GRect(0, 87, 71, 61));
  s_data_small[10] = text_layer_create(GRect(0, 87, 71, 61));
  s_data_title[10] = text_layer_create(GRect(0, 144, 71, 24));
  
  s_data_layer[11] = text_layer_create(GRect(73, 87, 71, 61));
  s_data_small[11] = text_layer_create(GRect(73, 87, 71, 61));
  s_data_title[11] = text_layer_create(GRect(73, 144, 71, 24));
  
  // Three fields - One big, two small
  //#define THREE_FIELD_INDEX 12
  #define THREE_FIELD_MAX 14
  s_data_layer[12] = text_layer_create(GRect(0, 12, 144, 59));
  s_data_small[12] = text_layer_create(GRect(0, 12, 144, 59)); // Plenty of space!
  s_data_title[12] = text_layer_create(GRect(0, 78, 144, 28));
  
  s_data_layer[13] = text_layer_create(GRect(0, 104, 71, 49));
  s_data_small[13] = text_layer_create(GRect(0, 104, 71, 49));
  s_data_title[13] = text_layer_create(GRect(0, 154, 71, 15));
  
  s_data_layer[14] = text_layer_create(GRect(73, 104, 71, 49));
  s_data_small[14] = text_layer_create(GRect(73, 104, 71, 49));
  s_data_title[14] = text_layer_create(GRect(73, 154, 71, 15));
  
  
  // Top title
  #define TITLE_INDEX 15
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
    layer_add_child(dataLayer, text_layer_get_layer(s_data_layer[i]));
    
    text_layer_set_background_color(s_data_small[i], GColorClear);
    text_layer_set_text_color(s_data_small[i], GColorWhite);
    text_layer_set_text_alignment(s_data_small[i], GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_data_small[i], GTextOverflowModeTrailingEllipsis);
    layer_add_child(dataLayer, text_layer_get_layer(s_data_small[i]));

    text_layer_set_background_color(s_data_title[i], GColorClear);
    text_layer_set_text_color(s_data_title[i], GColorWhite);
    text_layer_set_text_alignment(s_data_title[i], GTextAlignmentCenter);
    
    if (i >= SIX_FIELD_INDEX && i <= SIX_FIELD_MAX) // Small title fonts on the 6 field layout
      {
      text_layer_set_font(s_data_layer[i], s_6_font);    
      text_layer_set_font(s_data_small[i], s_6_font_small);    
      text_layer_set_font(s_data_title[i], s_title_font);
    }
    else if (i >= TWO_FIELD_INDEX && i <= TWO_FIELD_MAX) // This is 2 fields
      {
      text_layer_set_font(s_data_layer[i], s_2_font); 
      text_layer_set_font(s_data_small[i], s_2_font_small);   
      text_layer_set_font(s_data_title[i], s_large_title_font);
    }

    else if (i >= FOUR_FIELD_INDEX && i <= FOUR_FIELD_MAX) // 4 field layout
      {
      text_layer_set_font(s_data_layer[i], s_4_font); 
      text_layer_set_font(s_data_small[i], s_4_font_small);   
      text_layer_set_font(s_data_title[i], s_medium_title_font);
    }
    else if (i >= THREE_FIELD_INDEX && i <= THREE_FIELD_MAX)
      {
      if (i == THREE_FIELD_INDEX) // First field is big
        {
        text_layer_set_font(s_data_layer[i], s_2_font); 
        text_layer_set_font(s_data_small[i], s_2_font_small);   
        text_layer_set_font(s_data_title[i], s_large_title_font);
      } else
        {
        text_layer_set_font(s_data_layer[i], s_6_font);    
        text_layer_set_font(s_data_small[i], s_6_font_small);    
        text_layer_set_font(s_data_title[i], s_title_font);        
      }
    }      
   
    layer_add_child(titleLayer, text_layer_get_layer(s_data_title[i]));
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
    text_layer_destroy(s_data_small[i]);
  }
  
  layer_destroy(dataLayer);
  layer_destroy(titleLayer);
  text_layer_destroy(messageLayer);
  text_layer_destroy(s_data_layer[TITLE_INDEX]);  
  
  fonts_unload_custom_font(s_2_font);
  fonts_unload_custom_font (s_2_font_small);
  fonts_unload_custom_font(s_4_font);
  fonts_unload_custom_font (s_4_font_small);
  fonts_unload_custom_font(s_6_font);
  fonts_unload_custom_font (s_6_font_small);
}


//
// Handle incoming messages from phone
//
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char *layDecode[] = {"SPN", "SCB", "SMD",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"PPN","PCB","PMD","PPN","PCB","PMD"};
  static char buffer[20][10];
  static bool warnedLineBurn = false;
  bool earlyWarnDone = false;
  bool weAreRacing = false;
  bool doScreenTransition;
  int j, tmp;
  bool foundKey, negNum, useSmallFont;
  int startingScreen; // To remember which screen we were at when we arrived here so we don't loop forever looking for a screen with data on it
  
  startingScreen = currentScreen;
  
  if (holdThisScreen > 0)
    holdThisScreen--;
  // For all items
  
  weAreRacing = false; // Assume we're not racing unless we receive a mark name
  do
    {
    doScreenTransition = false;
    weAreRacing = false;
    // Read first item
    Tuple *t = dict_read_first(iterator);
    j = 0;
    while(t != NULL) {
    useSmallFont = false;
    foundKey = true;
    // Which key was received?
    //APP_LOG(APP_LOG_LEVEL_INFO, "Key %d Value %d", (int)t->key, (int)t->value->int32);
    switch(t->key) {
      
      case KEY_LINE_BURN:      
      case KEY_LAY_TIME:
      case KEY_LINE_TIME:
      case KEY_SECS_TO_START:
      case KEY_LAY_BURN:
      case KEY_TIME_TO_MARK:
      tmp = abs((int)t->value->int32) ;
      negNum = ((int)t->value->int32 < 0);
      if (screens[currentScreen].num_fields == 2 || tmp < 600) // We have room for mins & seconds
        {
        snprintf(buffer[j], sizeof(buffer[j]),"%d:%02d", tmp / 60, tmp % 60);
        useSmallFont = true;
      }
      else
    		{
          snprintf(buffer[j], sizeof(buffer[j]),"%d", tmp / 60);
    	}
      break;
      
      case KEY_LAY_DIST:
      case KEY_LINE_DIST:
      case KEY_LINE_ANGLE:
      case KEY_TARGET_ANGLE:
      case KEY_MARK_BEARING:
      case KEY_HEADING_COG:
      case KEY_ROUTE_NUMBER:
      case KEY_LAST_TACK:
      case KEY_TARGET_TACK:
      negNum = ((int)t->value->int32 < 0);
      snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      break;

/*      case KEY_LINE_BURN:      
      case KEY_LAY_TIME:
      case KEY_LINE_TIME:
      case KEY_SECS_TO_START:
      case KEY_LAY_BURN:
      negNum = ((int)t->value->int32 <= 0);
      snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      break; */
      
      case KEY_BOAT_SPEED_MARK:      
      case KEY_BOAT_SPEED:
      case KEY_TARGET_SPEED: 
      snprintf(buffer[j], sizeof(buffer[j]),"%d.%d", abs((int)t->value->int32)/10, abs((int)t->value->int32) % 10);
      negNum = ((int)t->value->int32 < 0);
      break;
      
      case KEY_MARK_LAY_DIST:
      negNum = ((int)t->value->int32 < 0);
      if (abs((int)t->value->int32) > 999)
        {
        float a, b;
        a = abs(t->value->int32);
        b = a / 185.2;
        if (b < 100)
          snprintf(buffer[j], sizeof(buffer[j]), "%d.%d", (int)b/10, (int)b % 10);
        else
          snprintf(buffer[j], sizeof(buffer[j]), "-");
        //APP_LOG(APP_LOG_LEVEL_INFO, "Key %s", buffer[j]);
      }
      else
        snprintf(buffer[j], sizeof(buffer[j]), "%d", (int)t->value->int32);
      break;
      
      case KEY_LAY_SEL:
      //APP_LOG(APP_LOG_LEVEL_INFO, "Key %d Value %d", (int)t->key, (int)t->value->int32);
      negNum = false;
      snprintf(buffer[j], sizeof(buffer[j]),"%s", layDecode[(int)t->value->int32]);
      break;
      
      /* These are the turn style values - Rnn & Lnn */
      case KEY_MARK_TURN:
      case KEY_TACK_HEADER:
      negNum = false;
      if (t->value->int32 >= 0)
        snprintf(buffer[j], sizeof(buffer[j]), "R%02d", (int)t->value->int32);
      else
        snprintf(buffer[j], sizeof(buffer[j]), "L%02d", -(int)(t->value->int32));
      break;
      
      case KEY_CURRENT_MARK:
      weAreRacing = true; //This data only arrives once the start is over, we must be racing
      negNum = false;
      snprintf(buffer[j], sizeof(buffer[j]), "%s", (char *)t->value->cstring);
      useSmallFont = true; // Some letters are pretty wide!
      break;
      
      default:
       //APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      foundKey = false;
      break;
    }
    if (foundKey) // Now look through the fields on the current screen to see if it is displayed
      {
      if (t->key == KEY_LINE_BURN) // Do vibrate regardless of what is displayed
              {
              if (!earlyWarnDone && t->value->int32 < 10 && t->value->int32 >0)
                {
                vibes_double_pulse ();
                earlyWarnDone = true;
              }
              else if (t->value->int32 <= 0)
                {
                if (!warnedLineBurn)
                    {
                    vibes_long_pulse();
                    warnedLineBurn = true;
                }
              }
              else
                {
                warnedLineBurn = false;
              }
            }
                
      int i;
        for (i=0; i<screens[currentScreen].num_fields; i++)
          {
          if (weAreRacing && keyTitles[screens[currentScreen].field_data_map[i]].preStart) // We are racing & we have pre-start data displayed
              doScreenTransition = true; // Force a transition if we are racing with a screen displaying prestart data
          if (keyTitles[screens[currentScreen].field_data_map[i]].key == (int)t->key) // Did we find a match?
            {

            if (useSmallFont == true || (screens[currentScreen].num_fields != 2 && strlen(buffer[j]) > 2))
              {
              if (strlen(buffer[j]) == 3 && buffer[j][1] == '.')
                useSmallFont = false; /* Can squeeze a . into big fonts! */ 
              else
                useSmallFont = true;
            }
            else
              useSmallFont = false;
            if (!useSmallFont) //Big fonts in use
              {
              text_layer_set_text(s_data_small[screens[currentScreen].field_layer_map[i]], ""); //Blank the small font field in case
              text_layer_set_text(s_data_layer[screens[currentScreen].field_layer_map[i]], buffer[j]); //Set the regular font field
              if (negNum) // Did we get a negative number
                {
                // Small fields are reset here in case we just switched from small font -ve to large font -ve
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
            else // Using Small Font Fields
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
    // So we need to use a different buffer for each message.
      j++;
    }
    t = dict_read_next(iterator); // Look for next item
    
  }
    
  // We've read all the data & updated the current screen.  Now work out if we should stay here!
  
  if (weAreRacing && doScreenTransition && holdThisScreen == 0) // We are racing, didn't find a match & don't need to hold this screen
    {
      do
        {
        currentScreen++;
        if (currentScreen == NUM_SCREENS)
          currentScreen = 0;
      } while (screens[currentScreen].num_fields == 0);
    if (currentScreen == startingScreen)
      doScreenTransition = false; // Stop looking - we're back where we started!
    updatescreen(currentScreen, NULL); 
  }
  } while (weAreRacing && doScreenTransition && holdThisScreen == 0); // j is the index to the next available buffer.  If it is zero, we didn't find
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
      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
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

// Blank Normal
static void blankNormal(int screen, int field)
  {
    text_layer_set_text(s_data_layer[screens[screen].field_layer_map[field]], "");
    text_layer_set_text(s_data_small[screens[screen].field_layer_map[field]], "");
    text_layer_set_background_color(s_data_layer[screens[screen].field_layer_map[field]], GColorClear);
    text_layer_set_text_color(s_data_layer[screens[screen].field_layer_map[field]], GColorWhite);
    text_layer_set_background_color(s_data_small[screens[screen].field_layer_map[field]], GColorClear);
    text_layer_set_text_color(s_data_small[screens[screen].field_layer_map[field]], GColorWhite);
}


//
// Up Click Handler
//
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring) { 
    screens[currentScreen].field_data_map[configuring_field]++; //Step to the next data item in the list
    if (screens[currentScreen].field_data_map[configuring_field] == NUM_KEYTITLES) // Wrap at the end
      screens[currentScreen].field_data_map[configuring_field] = 0;
          // Set the title & blank data
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[screens[currentScreen].field_data_map[configuring_field]].title);

    // Need to clear out data that may have just arrived & ensure that the field is not inverted
    blankNormal(currentScreen, configuring_field);
}
  else if (doubleClick) //Confirming reset to default
    {
    doubleClick = false;
    holdThisScreen = TRANSITION_IDLE;
    text_layer_set_text(messageLayer,"");  
      
    int i;
    for (i=0; i<4; i++) // Step through 4 default screens
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
    for (i=4; i<NUM_SCREENS; i++)
      screens[i].num_fields = 0;
    
    currentScreen = 0;
    updatescreen(-2, "");
  }
  else  // Step to next screen
    {
    holdThisScreen = TRANSITION_IDLE;
    do { // Search through screens to find the next one in use
  currentScreen--;
  if (currentScreen <0)
    currentScreen = NUM_SCREENS - 1;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"");
  }
}


//
// Down Click Handler - just like the up click handler
//
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring) {

    screens[currentScreen].field_data_map[configuring_field]--;

    if (screens[currentScreen].field_data_map[configuring_field] < 0)
        screens[currentScreen].field_data_map[configuring_field] = NUM_KEYTITLES -1;
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[screens[currentScreen].field_data_map[configuring_field]].title);

    blankNormal(currentScreen, configuring_field);
  }
  else if (doubleClick) // Cancelling reset to default
  {
    doubleClick = false;
    holdThisScreen = TRANSITION_IDLE;
    text_layer_set_text(messageLayer,"");  
    updatescreen(currentScreen, "");
  }
  else { // Not configuring - just step to next screen
    holdThisScreen = TRANSITION_IDLE;
    do { // Search through screens to find the next one in use
  currentScreen++;
  if (currentScreen == NUM_SCREENS)
    currentScreen = 0;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"");
  }
  
}
  
//
// Long Select Handler
//
static void long_select_handler(ClickRecognizerRef recognizer, void *context) {
  
    if (configuring == 0) { 
      holdThisScreen = -1;
      configuring = 1; // Select configuring mode
      
      configuring_field = 0;

      // Set current field title to inverse colours
      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
      
    }
    else { // End configuring, set current title back to normal display
      configuring = 0;
      holdThisScreen = TRANSITION_IDLE;
      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
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
            screens[i].field_data_map[j] = screens[currentScreen].field_data_map[j];
            screens[i].field_small_layer_map[j] = screens[currentScreen].field_small_layer_map[j];
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
    int ok = 0;  //Always allow delete as long as there is at least one remaining screen
    
    for (i=0; i<NUM_SCREENS; i++)
      {
      if (screens[i].num_fields != 0)
        ok++;
    }
    
            
  if (ok > 1) // OK to delete go find a new screen to display now
    {
    int i;
    for (i=0; i < NUM_SCREENS; i++)
      if (i != currentScreen && screens[i].num_fields != 0) // Avoid selecting the screen we are deleting & unused screens
        break;
    updatescreen(i, "");  // i is left set to the first in-use screen
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
    holdThisScreen = -1;
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
      if (initialValue != NULL)
        text_layer_set_text(s_data_layer[screens[thisScreen].field_layer_map[i]], initialValue);
    }
    
    // Set up titles
    for (i=0; i<screens[thisScreen].num_fields; i++)
      {
      if (screens[thisScreen].field_data_map[i] >= NUM_KEYTITLES)
        screens[thisScreen].field_data_map[i] = 0;
      text_layer_set_background_color(s_data_title[screens[thisScreen].field_layer_map[i]], GColorBlack);
      // text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], data_titles[screens[thisScreen].field_data_map[i]]);
      text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], keyTitles[screens[thisScreen].field_data_map[i]].title);
    }
  lastScreen = thisScreen;
  
  static char buf[50];
  snprintf(buf, sizeof(buf), "StartLine    Screen %d", thisScreen + 1);
  text_layer_set_text(s_data_layer[TITLE_INDEX], buf);
  }
}
  
