#include <pebble.h>
#include "startline.h"
#include "menu.h"

  void doTitleInvert()
  {
  GRect a = layer_get_frame(text_layer_get_layer(s_data_title[screens[currentScreen].field_layer_map[configuring_field]]));
  a.origin.y += a.size.h / 4;
  a.size.h -= a.size.h/4 ;
  layer_set_bounds(inverter_layer_get_layer(inverter), a);
  
  // To revert
  // layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0));
}
  
//
//
// ****************************
// All the button handlers here
// ****************************
//

void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (messageClick && configuring)
    {
    screenMessage(""); // Check that configuring is not turned off in updatescreen
    //text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
    //text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
  doTitleInvert();

      return;
  }
  if (configuring)
      { // Set the current title back to normal
//      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
//      text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0));

    
      configuring_field += 1; // Step to next field & wrap at the end
    if (configuring_field == screens[currentScreen].num_fields)
      configuring_field = 0;
  
    // Invert colours on current field title
      //text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      //text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
        doTitleInvert();

    }
  else if (messageClick) 
    {
    doubleClick = false;
    screenMessage("");
  }
//  else
    // show_menu();
}

  
  
//
// Up Click Handler
//
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick && configuring) // Displayed a message whilst configuring
    {
    screenMessage("");
    //text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
    //text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
    doTitleInvert();

  }
  if (configuring) { 
    doDataRevert(configuring_field);
    screens[currentScreen].field_data_map[configuring_field]++; //Step to the next data item in the list
    if (screens[currentScreen].field_data_map[configuring_field] == num_keytitles) // Wrap at the end
      screens[currentScreen].field_data_map[configuring_field] = 0;
          // Set the title & blank data
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[screens[currentScreen].field_data_map[configuring_field]].title);

    // Need to clear out data that may have just arrived & ensure that the field is not inverted
    blankNormal(currentScreen, configuring_field);
}
    else if (doubleClick) //Confirming reset to default
    {
    screenMessage("");
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
    for (i=4; i<NUM_SCREENS; i++) // Blank out the rest of the screens
      screens[i].num_fields = 0; 
      
    currentScreen = 0;
    updatescreen(-2, "");
    doubleClick = false;
    }
  else if (messageClick)
    {
    screenMessage("");
  }
  else  // Step to next screen
    {    
//Here

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
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick && configuring)
    {
    screenMessage(""); // Blank the message
//    text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
//    text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
    doTitleInvert();
  }
  if (configuring) {
    doDataRevert(configuring_field);
    screens[currentScreen].field_data_map[configuring_field]--;

    if (screens[currentScreen].field_data_map[configuring_field] < 0)
        screens[currentScreen].field_data_map[configuring_field] = num_keytitles -1;
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[screens[currentScreen].field_data_map[configuring_field]].title);

    blankNormal(currentScreen, configuring_field);
  }
  else if (messageClick) // Cancelling reset to default
  {
    doubleClick = false;
    screenMessage("");
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
void long_select_handler(ClickRecognizerRef recognizer, void *context) {
    if (messageClick)
      return; // Not allowed while a messgae is displayed
    if (configuring == 0) { 
      holdThisScreen = -1;
      configuring = 1; // Select configuring mode
      
      configuring_field = 0;
      doTitleInvert();
      
      // Set current field title to inverse colours
      //text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      //text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
      
    }
    else { // End configuring, set current title back to normal display
      int i;
      bool preStart = false, postStart = false;
      for (i = 0; i < screens[currentScreen].num_fields; i++)
        {
        if (keyTitles[screens[currentScreen].field_data_map[i]].preStart)
                      preStart = true;
        if (keyTitles[screens[currentScreen].field_data_map[i]].postStart)
                      postStart = true;
      }
      if (preStart && postStart)
        {
        screenMessage("You may not mix pre and post start fields on one screen");
      }
      else
        {
      configuring = 0;
      holdThisScreen = TRANSITION_IDLE;
      layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0));

//      text_layer_set_background_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorBlack);
//      text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], GColorWhite);
      }
    }
}
  
void long_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring == 0 && !messageClick && !doubleClick)  // When not configuring, long up creates a new screen like the current one
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
//            screens[i].field_small_layer_map[j] = screens[currentScreen].field_small_layer_map[j];
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

void long_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (configuring == 0 && !messageClick && !doubleClick) // Long down deletes the current screen so long as it's not the last one
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


void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick)
    return; // Not allowed when a message is displayed
  int i = click_number_of_clicks_counted(recognizer);
  if (i==2)
    {
  if (!configuring)
    {
  doubleClick = true;
  screenMessage("Press UP to reset to default, DOWN otherwise");
  }
  }
  else if (i == 3)
    {
  if (!configuring && !doubleClick)
  {
  screenMessage(STARTLINE_PEBBLE_VERSION);
  }
    
  }
}
