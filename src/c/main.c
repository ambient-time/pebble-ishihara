// Copyright 2026 Luke Steuber. MIT License.
#include <pebble.h>
#include "plate.h"
static Window *s_window;static Layer *s_layer;static Plate *s_plate;
static bool s_focused,s_hands,s_built;static uint32_t s_seed,s_frames;
static int s_w,s_h;
#define HANDS_KEY 1
static int pixel(double value) {return (int)(value+.5);}
static void hand(GContext *ctx,int angle,int length,int width) {
  GPoint center=GPoint(s_w/2,s_h/2);
  GPoint tip=GPoint(center.x+sin_lookup(angle)*length/TRIG_MAX_RATIO,center.y-cos_lookup(angle)*length/TRIG_MAX_RATIO);
  graphics_context_set_stroke_color(ctx,GColorBlack);graphics_context_set_stroke_width(ctx,width+2);graphics_draw_line(ctx,center,tip);
  graphics_context_set_stroke_color(ctx,GColorWhite);graphics_context_set_stroke_width(ctx,width);graphics_draw_line(ctx,center,tip);
}
static void draw(Layer *layer,GContext *ctx) {
  time_t epoch=time(NULL);struct tm t=*localtime(&epoch);uint32_t seed=plate_seed(epoch,s_w,s_h);
  time_t start_seconds,end_seconds;uint16_t start,end;time_ms(&start_seconds,&start);
  if(!s_built||seed!=s_seed||plate_hour(t.tm_hour)!=s_plate->hour) {
    plate_build(s_plate,seed,t.tm_hour,s_w,s_h);s_seed=seed;s_built=true;
  }
  graphics_context_set_antialiased(ctx,true);
  graphics_context_set_fill_color(ctx,GColorBlack);graphics_fill_rect(ctx,layer_get_bounds(layer),0,GCornerNone);
  for(int i=0;i<s_plate->count;i++) {
    const PlateDot *d=&s_plate->dots[i];GPoint p=GPoint(pixel(d->x),pixel(d->y));int r=pixel(d->r);
#ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx,(GColor){.argb=d->color});graphics_fill_circle(ctx,p,r);
#else
    graphics_context_set_fill_color(ctx,GColorWhite);
    if(d->shape)graphics_fill_circle(ctx,p,r);
    else {graphics_context_set_stroke_color(ctx,GColorWhite);graphics_context_set_stroke_width(ctx,1);graphics_draw_circle(ctx,p,r);}
#endif
  }
  if(s_hands) {
    hand(ctx,TRIG_MAX_ANGLE*((t.tm_hour%12)*60+t.tm_min)/720,pixel(s_plate->radius*.52),9);
    hand(ctx,TRIG_MAX_ANGLE*t.tm_min/60,pixel(s_plate->radius*.8),6);
    graphics_context_set_fill_color(ctx,GColorBlack);graphics_fill_circle(ctx,GPoint(s_w/2,s_h/2),9);
    graphics_context_set_fill_color(ctx,GColorWhite);graphics_fill_circle(ctx,GPoint(s_w/2,s_h/2),8);
  }
  s_frames++;time_ms(&end_seconds,&end);
  APP_LOG(APP_LOG_LEVEL_INFO,"Plate time=%02d:%02d:%02d hour=%d seed=%lu dots=%d palette=%d sig=%08lx hands=%d",t.tm_hour,t.tm_min,t.tm_sec,s_plate->hour,(unsigned long)s_seed,s_plate->count,s_plate->palette,(unsigned long)plate_signature(s_plate),s_hands);
  APP_LOG(APP_LOG_LEVEL_INFO,"Plate cost ms=%ld heap=%lu frames=%lu",(long)((end_seconds-start_seconds)*1000+end-start),(unsigned long)heap_bytes_free(),(unsigned long)s_frames);
}
static void tap(AccelAxisType axis,int32_t direction) {s_hands=!s_hands;persist_write_bool(HANDS_KEY,s_hands);if(s_focused&&s_layer)layer_mark_dirty(s_layer);(void)axis;(void)direction;}
static void tick(struct tm *t,TimeUnits units) {if(s_focused&&s_layer)layer_mark_dirty(s_layer);(void)t;(void)units;}
static void focus(bool value) {s_focused=value;if(value&&s_layer)layer_mark_dirty(s_layer);}
static void load(Window *window) {
  s_plate=malloc(sizeof(*s_plate));
  if(!s_plate){APP_LOG(APP_LOG_LEVEL_ERROR,"Plate allocation failed");return;}
  GRect b=layer_get_bounds(window_get_root_layer(window));s_w=b.size.w;s_h=b.size.h;
  s_layer=layer_create(b);layer_set_update_proc(s_layer,draw);layer_add_child(window_get_root_layer(window),s_layer);
}
static void unload(Window *window) {if(s_layer)layer_destroy(s_layer);free(s_plate);(void)window;}
static void init(void) {s_hands=persist_exists(HANDS_KEY)?persist_read_bool(HANDS_KEY):false;s_window=window_create();window_set_window_handlers(s_window,(WindowHandlers){.load=load,.unload=unload});window_stack_push(s_window,false);tick_timer_service_subscribe(MINUTE_UNIT,tick);accel_tap_service_subscribe(tap);app_focus_service_subscribe(focus);focus(true);}
static void deinit(void) {tick_timer_service_unsubscribe();accel_tap_service_unsubscribe();app_focus_service_unsubscribe();window_destroy(s_window);}
int main(void) {init();app_event_loop();deinit();}
