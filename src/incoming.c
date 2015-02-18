#include <pebble.h>
#include "startline.h"


//
// Handle incoming messages from phone
//
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char *layDecode[] = {"SPN", "SCB", "SMD",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"PPN","PCB","PMD","PPN","PCB","PMD"};
  static char buffer[20][10];
  static char tackLogBuffer[7][6];
  static int tackLog[7] = {0,0,0,0,0,0,0};
  static bool warnedLineBurn = false;
  static int currentState = -1;
  static int thisTack = 0, oldTack = 0;
  bool earlyWarnDone = false;
  bool weAreRacing = false;
  bool doScreenTransition;
  int j, tmp;
  bool foundKey, negNum; 
  int startingScreen; // To remember which screen we were at when we arrived here so we don't loop forever looking for a screen with data on it
  int a;
  float b;
  static int flashFlag = 0;
  
  
  
  if (doubleClick || messageClick)
    return;
  startingScreen = currentScreen;
  
  if (flashFlag == 0) {
    layer_set_bounds(inverter_layer_get_layer(flash), GRect(0,0,0,0)); 
//    text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorBlack);
//    text_layer_set_text_color(s_data_layer[TITLE_INDEX], GColorWhite);
    flashFlag = 1;
      }
  else {
    layer_set_bounds(inverter_layer_get_layer(flash), GRect(0,0,10,10)); // Turn on the inverter layer
 //   text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorWhite);
 //   text_layer_set_text_color(s_data_layer[TITLE_INDEX], GColorBlack);
    flashFlag = 0;
  }
  
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
      negNum = false;
      
      tmp = abs((int)t->value->int32) ;
      negNum = ((int)t->value->int32 < 0);

      if (tmp >= 100)
      {
        if ((tmp < 6000 && isBigField(t->key)) || tmp < 600) // We have room for mins & seconds & always when mins < 10
        {
          snprintf(buffer[j], sizeof(buffer[j]),"%d:%02d", tmp / 60, tmp % 60);
        }
        else if (tmp < 3600)
        {
          snprintf(buffer[j], sizeof(buffer[j]),"%dm", tmp / 60 + ((tmp % 60) >= 30 ? 1:0));
        }
        else
          {
          snprintf(buffer[j], sizeof(buffer[j]),"%dh", tmp / 3600 + ((tmp % 3600) >= 1800 ? 1:0));          
        }
      }
      else // (tmp < 100)
      {
        snprintf(buffer[j], sizeof(buffer[j]),"%ds", tmp);
      }

      break;
      

      case KEY_LAST_TACK:
      oldTack = thisTack; // Remember the tack from the last message 
      thisTack = (int)t->value->int32;
      negNum = ((int)t->value->int32 < 0); // This definitely shouldn't happen!
      if (currentState != 1)
        buffer[j][0] = '\0';
      else
        snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      break;
      
      case KEY_TARGET_TACK:
      negNum = false;
      snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      
      case KEY_LAY_DIST:
      case KEY_LINE_DIST:
      case KEY_LINE_ANGLE:
      case KEY_TARGET_ANGLE:
      case KEY_MARK_BEARING:
      case KEY_HEADING_COG:
      negNum = ((int)t->value->int32 < 0);
      snprintf(buffer[j], sizeof(buffer[j]),"%d", abs((int)t->value->int32));
      break;
 
      case KEY_BOAT_SPEED:
      case KEY_TARGET_SPEED: 
      snprintf(buffer[j], sizeof(buffer[j]),"%d.%d", abs((int)t->value->int32)/10, abs((int)t->value->int32) % 10);
      negNum = ((int)t->value->int32 < 0);
      break;
      
      case KEY_MARK_DIST:
      case KEY_MARK_LAY_DIST:
      negNum = false;
      if (currentState != 1  && t->key == KEY_MARK_LAY_DIST)
        buffer[j][0] = '\000';
      else
        {
        a = (int)t->value->int32;
        negNum = (a < 0);
        a = abs(a);
        b = a / 18.52;
        bool bf = isBigField((int)t->key);
        if (a < 1000) // Less than 1000m - just show m
          snprintf(buffer[j], sizeof(buffer[j]), "%d", a);
        else if (b < 1000 || bf) // less than 100nm or it's a big field - show nm.n
          {
          int d1, d2, d3;
          d1 = (int)b/100;
          d2 = (((int)b % 100)/10);
          d3 = ((((int)b % 10) > 4) ? 1 : 0);
          if (d3 == 1)
            d2 += 1;
          if (d2 == 10)
            {
            d1 += 1;
            d2 = 0;
          }
          if (d1 > 999) // Over 999 nm away - Only when we're flying!
            snprintf(buffer[j], sizeof(buffer[j]), "%d", d1);
          else
            snprintf(buffer[j], sizeof(buffer[j]), "%d.%d", d1, d2);
        }
        else
            {
            snprintf(buffer[j], sizeof(buffer[j]), "%d", (int)b/10);
        }
      }
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
      if (currentState != 1 && t-> key == KEY_TACK_HEADER)
        buffer[j][0] = '\000';
      else
        {
        if (t->value->int32 >= 0)
          snprintf(buffer[j], sizeof(buffer[j]), "R%02d", (int)t->value->int32);
        else
          snprintf(buffer[j], sizeof(buffer[j]), "L%02d", -(int)(t->value->int32));
      }
      break;
      
      case KEY_CURRENT_MARK:
      weAreRacing = true; //This data only arrives once the start is over, we must be racing
      negNum = false;
      snprintf(buffer[j], sizeof(buffer[j]), "%s", (char *)t->value->cstring);
   
      break;
      
      case KEY_TACK_STATE:
      a = (int)t->value->int32;
      negNum = a < 0;
      snprintf(buffer[j], sizeof(buffer[j]), "%d", abs((int)(t->value->int32)));
      if (currentState == 1 && a != 1) // Just detected a tack
        {
        tackLog[0] = oldTack;
        int i;
        for (i = 6; i > 0; i--) // Shuffle them all up one
          tackLog[i] = tackLog[i-1];
      }
      currentState = a;
      // Now display the log
      int k = 1; // Start the log at 1 - 0 is the current tack ready to be shuffled up
      int i = 0;
      for (i=0; i < screens[currentScreen].num_fields; i++) // Go look for TACK LOG on the screen
      {
        if (keyTitles[screens[currentScreen].field_data_map[i]].key == KEY_TACK_LOG) // Tack Log is displayed
        {
          if (tackLog[k] == 0)
            tackLogBuffer[k][0] = '\000';
          else
            snprintf(tackLogBuffer[k], sizeof(tackLogBuffer[k]), "%d", tackLog[k]);
          setField(i, false, tackLogBuffer[k]);
          k++; // Step to the next tacklog entry
        }
      }
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
            setField(i /* Field Index */ , negNum, buffer[j] );
          }
        }
    // Use next buffer - seems that the buffer remains in use until the data actually appears on the screen
    // So we need to use a different buffer for each message.
      j++;
    }
    t = dict_read_next(iterator); // Look for next item
    
  }
    
    
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
