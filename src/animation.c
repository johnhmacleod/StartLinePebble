#include <pebble.h>
#include "startline.h"
  
  

int16_t getLayerBounds(void *subject)
{
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "In bounds getter function:");
  GRect temp = layer_get_bounds((Layer*)subject);
  return(temp.origin.x);
};
 
void setLayerBounds(void *subject, int16_t new_bounds)
{
  // APP_LOG(APP_LOG_LEVEL_INFO, "In setter function: x=%d y=%d w=%d h=%d",new_bounds.origin.x,new_bounds.origin.y,new_bounds.size.w,new_bounds.size.h);
  // APP_LOG(APP_LOG_LEVEL_INFO, "In setter with %d", new_bounds);
  GRect b;
  b = layer_get_bounds(subject);
  b.origin.x = new_bounds;
  layer_set_bounds((Layer *)subject, b);
};


/*
void myUpdateInt16(PropertyAnimation *anim, int norm)
  {
  APP_LOG(APP_LOG_LEVEL_INFO, "In myUpdateInt16 with norm=%d", norm);
  int fr, to, val, dif;
  //fr = global_fr;  // Cheat to get the values passed in
  //to = global_to;  // Cheat to get the values passed in

  fr = anim->values.from.int16;
  to = anim->values.to.int16;
  dif = to - fr;
  APP_LOG(APP_LOG_LEVEL_INFO, "In updater with norm=%d fr=%d to=%d", norm, fr, to);
  val = ((float)norm / (ANIMATION_NORMALIZED_MAX - ANIMATION_NORMALIZED_MIN)) * dif + fr;
  ((PropertyAnimationImplementation *)(anim->animation.implementation))->accessors.setter.int16(anim->subject, val);
      
}
*/

static const PropertyAnimationImplementation my_implementation = {
  .base = {
    // using the "stock" update callback:
    .update = (AnimationUpdateImplementation) property_animation_update_int16,
    // .update = (AnimationUpdateImplementation) myUpdateInt16,
  },
  .accessors = {
    // my accessors that get/set a int16 from/onto my subject:
    .setter = { .int16 = (const Int16Setter)setLayerBounds, },
    .getter = { .int16 = (const Int16Getter)getLayerBounds, },
  },
};
 
static void animation_stopped(PropertyAnimation *animation, bool finished, void *data) {
  property_animation_destroy(animation);
  *((Animation **)data) = NULL;
}



void animate_layer_bounds(PropertyAnimation **anim, Layer* layer, GRect *start, GRect *finish, int duration, int delay)
{
  static int s, f;
  // APP_LOG(APP_LOG_LEVEL_INFO, "animate_layer_bounds -> start: x:y w:h %d:%d %d:%d finish: %d:%d %d:%d", 
//          (int)start->origin.x, (int)start->origin.y,(int)start->size.w, (int)start->size.h,
//          (int)finish->origin.x, (int)finish->origin.y,(int)finish->size.w, (int)finish->size.h);


  s = start->origin.x;
  f = finish->origin.x;

  
  *anim = property_animation_create(&my_implementation, layer, &s, &f);
  
  // Workaround SDK bug
  (*anim)->values.from.int16 = s;
  (*anim)->values.to.int16 = f;
  
  
  AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) animation_stopped
    };
  
  animation_set_handlers((Animation*) *anim, handlers, anim);
  animation_set_duration((Animation*) *anim, duration);
  animation_set_delay((Animation*) *anim, delay);
  animation_schedule((Animation*) *anim);

}


