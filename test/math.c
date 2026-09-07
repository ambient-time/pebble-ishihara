// Copyright 2026 Luke Steuber. MIT License.
#include "../src/c/plate.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
  assert(plate_hour(0)==12);assert(plate_hour(12)==12);assert(plate_hour(23)==11);assert(plate_hour(24)==12);
  assert(plate_seed(59)==plate_seed(0));assert(plate_seed(60)!=plate_seed(59));
  assert(plate_seed(86399)!=plate_seed(86400));assert(plate_seed(-1)==plate_seed(-60));
  char text[6];plate_readout(text,0,0,false);assert(!strcmp(text,"12:00"));plate_readout(text,23,59,true);assert(!strcmp(text,"23:59"));
  for(int hour=1;hour<=12;hour++){
    int pixels=0;for(int y=0;y<128;y++)for(int x=0;x<128;x++)pixels+=plate_mask(hour,x,y);
    assert(pixels>900&&pixels<7000);
    Plate a,b;plate_build(&a,plate_seed(hour*3600),hour);plate_build(&b,plate_seed(hour*3600),hour);
    assert(!memcmp(&a,&b,sizeof(a)));assert(a.count>180&&a.count<PLATE_MAX_DOTS);assert(a.hour==hour);
    int foreground=0;
    for(int i=0;i<a.count;i++){
      PlateDot d=a.dots[i];int dx=d.x-64,dy=d.y-64;assert(dx*dx+dy*dy<=(62-d.r)*(62-d.r));foreground+=d.style&1;
      assert((bool)(d.style&1)==plate_mask(hour,d.x,d.y));
      for(int j=i+1;j<a.count;j++){PlateDot e=a.dots[j];dx=d.x-e.x;dy=d.y-e.y;assert(dx*dx+dy*dy>=(d.r+e.r+1)*(d.r+e.r+1));}
    }
    assert(foreground>25);plate_build(&b,plate_seed(hour*3600+60),hour);assert(memcmp(&a,&b,sizeof(a))!=0);
  }
  puts("Ishihara: every hour mask, deterministic organic packing, gaps, minute regeneration and day rollovers pass.");
}
