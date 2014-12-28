/*
#include <pebble.h>

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1
static Window *menu_window;

// This is a simple menu layer
static SimpleMenuLayer *simple_menu_layer;

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[NUM_MENU_SECTIONS];

// Each section is composed of a number of menu items
static SimpleMenuItem first_menu_items[NUM_FIRST_MENU_ITEMS];
static SimpleMenuItem second_menu_items[NUM_SECOND_MENU_ITEMS];

// You can capture when the user selects a menu icon with a menu item select callback
static void menu_select_callback(int index, void *ctx) {
  // Here we just change the subtitle to a literal string
first_menu_items[index].subtitle = "You've hit select here!";
// Mark the layer to be updated
layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}


// This initializes the menu upon window load
static void menu_window_load(Window *window) {

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
// an int as such to easily change the order of menu items later
int num_a_items = 0;
// This is an example of how you'd set a simple menu item
first_menu_items[num_a_items++] = (SimpleMenuItem){
// You should give each menu item a title and callback
.title = "First Item",
.callback = menu_select_callback,
};
// The menu items appear in the order saved in the menu items array
first_menu_items[num_a_items++] = (SimpleMenuItem){
.title = "Second Item",
// You can also give menu items a subtitle
.subtitle = "Here's a subtitle",
.callback = menu_select_callback,
};
first_menu_items[num_a_items++] = (SimpleMenuItem){
.title = "Third Item",
.subtitle = "This has an icon",
.callback = menu_select_callback,
// This is how you would give a menu item an icon
.icon = menu_icon_image,
};
// This initializes the second section
second_menu_items[0] = (SimpleMenuItem){
.title = "Special Item",
// You can use different callbacks for your menu items
.callback = special_select_callback,
};
// Bind the menu items to the corresponding menu sections
menu_sections[0] = (SimpleMenuSection){
.num_items = NUM_FIRST_MENU_ITEMS,
.items = first_menu_items,
};
menu_sections[1] = (SimpleMenuSection){
// Menu sections can also have titles as well
.title = "Yet Another Section",
.num_items = NUM_SECOND_MENU_ITEMS,
.items = second_menu_items,
};
// Now we prepare to initialize the simple menu layer
// We need the bounds to specify the simple menu layer's viewport size
// In this case, it'll be the same as the window's
Layer *window_layer = window_get_root_layer(window);
GRect bounds = layer_get_frame(window_layer);
// Initialize the simple menu layer
simple_menu_layer = simple_menu_layer_create(bounds, window, menu_sections, NUM_MENU_SECTIONS, NULL);
// Add it to the window for display
layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));
}
// Deinitialize resources on window unload that were initialized on window load
void window_unload(Window *window) {
simple_menu_layer_destroy(simple_menu_layer);
// Cleanup the menu icon
gbitmap_destroy(menu_icon_image);
}
int main(void) {
window = window_create();
// Setup the window handlers
window_set_window_handlers(window, (WindowHandlers) {
.load = window_load,
.unload = window_unload,
});
window_stack_push(window, true );
app_event_loop();
window_destroy(window);
}
*/