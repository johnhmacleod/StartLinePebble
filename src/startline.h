#define STARTLINE_PEBBLE_VERSION "StartLine Pebble V1.8 build 3"

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
    #define KEY_MARK_DIST  60
    #define KEY_TACK_STATE 61
    #define KEY_TACK_LOG 200

    #define NUM_SCREENS 10

    #define TRANSITION_IDLE 5 //Number of seconds of idle before we do auto transition
  
  #define TITLE_INDEX 15
  
    #define MAPPING_PKEY  100 // Start key number for persistent storage
  
typedef struct
  {
  int key;
  char *title;
  bool preStart; // Exclusively pre-start
  bool postStart; // Exclusively post-start
} keyTitle;


      
typedef struct {
  int num_fields; // How many fields on this screen
  int field_data_map[6]; // How do these fields map to data
  int field_layer_map[6]; // How do these fields map to display real estate
  int field_small_layer_map[6]; // Not used but retained for compatibility with stored maps
  int is_start; // true if this is a start related screen
} Screen;

extern TextLayer *s_data_layer[];
extern InverterLayer *inverter, *flash;
extern Screen screens[];
extern Screen screenDefault[];
extern keyTitle keyTitles[];
bool doubleClick, messageClick;
int currentScreen, holdThisScreen, configuring_field, configuring, num_keytitles;
extern TextLayer *s_data_title[];

bool isBigField(int key);
void setField(int i /* Field Index */,  bool negNum, char* value);
void updatescreen(int thisScreen, char *initialValue);
bool isBigField(int key);
void inbox_received_callback(DictionaryIterator *iterator, void *context);
void select_click_handler(ClickRecognizerRef recognizer, void *context);
void select_multi_click_handler(ClickRecognizerRef recognizer, void *context);
void long_down_handler(ClickRecognizerRef recognizer, void *context);
void long_up_handler(ClickRecognizerRef recognizer, void *context);
void long_select_handler(ClickRecognizerRef recognizer, void *context);
void down_click_handler(ClickRecognizerRef recognizer, void *context);
void up_click_handler(ClickRecognizerRef recognizer, void *context);
void screenMessage(char *message);
void blankNormal(int screen, int field);
void doDataRevert(int field);