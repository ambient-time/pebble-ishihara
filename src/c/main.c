// Copyright 2026 Luke Steuber. MIT License.
#include <pebble.h>
#include "plate.h"
static Window *s_window;static Layer *s_layer;static GFont s_font;static Plate s_plate;
static bool s_focused,s_hands;static uint32_t s_seed,s_frames;static bool s_built;
static int s_w,s_h,s_radius,s_cy,s_fontsize;
#define HANDS_KEY 1
#ifdef PBL_ROUND
#define ROUND true
#else
#define ROUND false
#endif
static GColor dot_color(uint8_t palette,uint8_t style){
#ifdef PBL_COLOR
  const uint32_t colors[3][2][4]={
    {{0xff5555,0xffaaaa,0xff55aa,0xffaaaa},{0x55aaaa,0x55ffaa,0xaaffaa,0x00aa55}},
    {{0xff55aa,0xff5555,0xffaaff,0xffaaaa},{0xaaff55,0xaaffaa,0x55aa55,0xaaaa55}},
    {{0xaa55aa,0xaa55ff,0x5555aa,0xaa55aa},{0xaaaa00,0xaaaa55,0xffff55,0x55aa00}}
  };
  return GColorFromHEX(colors[palette][style&1][style>>1]);
#else
  (void)palette;(void)style;return GColorWhite;
#endif
}
static void hand(GContext *ctx,int angle,int length,int width){
  GPoint center=GPoint(s_w/2,s_cy),tip=GPoint(center.x+sin_lookup(angle)*length/TRIG_MAX_RATIO,center.y-cos_lookup(angle)*length/TRIG_MAX_RATIO);
  graphics_context_set_stroke_color(ctx,GColorBlack);graphics_context_set_stroke_width(ctx,width+2);graphics_draw_line(ctx,center,tip);
  graphics_context_set_stroke_color(ctx,GColorWhite);graphics_context_set_stroke_width(ctx,width);graphics_draw_line(ctx,center,tip);
}
static void draw(Layer *layer,GContext *ctx){
  time_t epoch=time(NULL);struct tm t=*localtime(&epoch);uint32_t seed=plate_seed(epoch);
  uint16_t start,end;time_ms(NULL,&start);
  if(!s_built||seed!=s_seed||plate_hour(t.tm_hour)!=s_plate.hour){plate_build(&s_plate,seed,t.tm_hour);s_seed=seed;s_built=true;}
  graphics_context_set_antialiased(ctx,true);graphics_context_set_fill_color(ctx,GColorBlack);graphics_fill_rect(ctx,layer_get_bounds(layer),0,GCornerNone);
  for(int i=0;i<s_plate.count;i++){
    PlateDot d=s_plate.dots[i];GPoint p=GPoint(s_w/2+((int)d.x-64)*s_radius/62,s_cy+((int)d.y-64)*s_radius/62);
    int r=d.r*s_radius/62;if(r<1)r=1;
    graphics_context_set_fill_color(ctx,dot_color(s_plate.palette,d.style));
#ifdef PBL_COLOR
    graphics_fill_circle(ctx,p,r);
#else
    if(d.style&1)graphics_fill_circle(ctx,p,r);
    else{graphics_context_set_stroke_color(ctx,GColorWhite);graphics_context_set_stroke_width(ctx,1);graphics_draw_circle(ctx,p,r>2?r-1:1);}
#endif
  }
  if(s_hands){
    hand(ctx,TRIG_MAX_ANGLE*((t.tm_hour%12)*60+t.tm_min)/720,s_radius*51/100,s_w>=200?5:4);
    hand(ctx,TRIG_MAX_ANGLE*t.tm_min/60,s_radius*79/100,s_w>=200?4:3);
    graphics_context_set_fill_color(ctx,GColorBlack);graphics_fill_circle(ctx,GPoint(s_w/2,s_cy),s_w>=200?6:4);
    graphics_context_set_fill_color(ctx,GColorWhite);graphics_fill_circle(ctx,GPoint(s_w/2,s_cy),s_w>=200?4:3);
  }
  char text[6];plate_readout(text,t.tm_hour,t.tm_min,clock_is_24h_style());
  graphics_context_set_text_color(ctx,GColorWhite);
  graphics_draw_text(ctx,text,s_font,GRect(0,s_h-s_fontsize-(ROUND?9:6),s_w,s_fontsize+8),GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  s_frames++;time_ms(NULL,&end);
  APP_LOG(APP_LOG_LEVEL_INFO,"Plate time=%02d:%02d:%02d hour=%d seed=%lu dots=%d hands=%d frames=%lu ms=%d heap=%lu",t.tm_hour,t.tm_min,t.tm_sec,s_plate.hour,(unsigned long)s_seed,s_plate.count,s_hands,(unsigned long)s_frames,(end+1000-start)%1000,(unsigned long)heap_bytes_free());
}
static void tap(AccelAxisType axis,int32_t direction){s_hands=!s_hands;persist_write_bool(HANDS_KEY,s_hands);if(s_focused)layer_mark_dirty(s_layer);(void)axis;(void)direction;}
static void tick(struct tm *t,TimeUnits units){if(s_focused)layer_mark_dirty(s_layer);(void)t;(void)units;}
static void focus(bool value){s_focused=value;if(value)layer_mark_dirty(s_layer);}
static void load(Window *window){
  GRect b=layer_get_bounds(window_get_root_layer(window));s_w=b.size.w;s_h=b.size.h;
  s_radius=ROUND?s_w*42/100:s_w*46/100;s_cy=ROUND?s_h*44/100:(s_h-22*s_w/144)/2;
  s_fontsize=s_w>=200?24:s_w>=180?20:16;
  s_font=fonts_load_custom_font(resource_get_handle(s_fontsize==24?RESOURCE_ID_TIME_24:s_fontsize==20?RESOURCE_ID_TIME_20:RESOURCE_ID_TIME_16));
  s_layer=layer_create(b);layer_set_update_proc(s_layer,draw);layer_add_child(window_get_root_layer(window),s_layer);
}
static void unload(Window *window){layer_destroy(s_layer);fonts_unload_custom_font(s_font);(void)window;}
static void init(void){s_hands=persist_exists(HANDS_KEY)?persist_read_bool(HANDS_KEY):false;s_window=window_create();window_set_window_handlers(s_window,(WindowHandlers){.load=load,.unload=unload});window_stack_push(s_window,false);tick_timer_service_subscribe(MINUTE_UNIT,tick);accel_tap_service_subscribe(tap);app_focus_service_subscribe(focus);focus(true);}
static void deinit(void){tick_timer_service_unsubscribe();accel_tap_service_unsubscribe();app_focus_service_unsubscribe();window_destroy(s_window);}
int main(void){init();app_event_loop();deinit();}
