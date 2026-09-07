// Copyright 2026 Luke Steuber. MIT License.
#include "../src/c/plate.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static Plate a,b;
int main(int argc,char **argv) {
  if(argc==6&&!strcmp(argv[1],"--dump")) {
    int w=atoi(argv[2]),h=atoi(argv[3]),hour=atoi(argv[4]);
    plate_build(&a,(uint32_t)strtoul(argv[5],NULL,10),hour,w,h);
    printf("{\"palette\":%d,\"signature\":%u,\"dots\":[",a.palette,plate_signature(&a));
    for(int i=0;i<a.count;i++) {
      PlateDot *d=&a.dots[i];
      printf("%s[%.15g,%.15g,%.15g,%u,%u]",i?",":"",d->x,d->y,d->r,d->shape,d->color);
    }
    puts("]}");return 0;
  }
  assert(plate_hour(0)==12&&plate_hour(12)==12&&plate_hour(23)==11&&plate_hour(24)==12);
  const int sizes[][2]={{144,168},{180,180},{200,228},{260,260}};
  for(int screen=0;screen<4;screen++) {
    int w=sizes[screen][0],h=sizes[screen][1],size=(int)floor(w*.88+.5);
    assert(plate_seed(59,w,h)==plate_seed(0,w,h));
    assert(plate_seed(60,w,h)!=plate_seed(59,w,h));
    assert(plate_seed(86399,w,h)!=plate_seed(86400,w,h));
    assert(plate_seed(-1,w,h)==plate_seed(-60,w,h));
    assert(plate_seed(0,w,h)==(0x9e37U^((uint32_t)floor(w*.44+.5)*131U)));
    for(int hour=1;hour<=12;hour++) {
      int pixels=0,left=size,right=0,top=size,bottom=0;
      for(int y=0;y<size;y++)for(int x=0;x<size;x++)if(plate_mask(hour,size,x,y)) {
        pixels++;if(x<left)left=x;if(x>right)right=x;if(y<top)top=y;if(y>bottom)bottom=y;
      }
      assert(pixels>size*size*.045&&pixels<size*size*.20);
      assert(left>size*.15&&right<size*.85&&top>size*.2&&bottom<size*.8);
      assert(bottom-top>size*.29&&bottom-top<size*.47);
      uint32_t seed=plate_seed(1788728880+hour*3600,w,h);
      plate_build(&a,seed,hour,w,h);plate_build(&b,seed,hour,w,h);
      assert(!memcmp(&a,&b,sizeof(a)));assert(a.count>100&&a.count<PLATE_MAX_DOTS);
      assert(a.cx==w*.5&&a.cy==h*.5&&a.radius==w*.44&&a.hour==hour);
      int foreground=0;
      for(int i=0;i<a.count;i++) {
        PlateDot *d=&a.dots[i];
        assert(d->r>=2&&d->r<=4.5);
        assert(hypot(d->x-a.cx,d->y-a.cy)+d->r<=a.radius-1+1e-8);
        assert(d->shape==plate_mask(hour,size,(int)floor(d->x-a.cx+a.radius),(int)floor(d->y-a.cy+a.radius)));
        assert(d->color>=0xc0);foreground+=d->shape;
      }
      assert(foreground>5);
      plate_build(&b,plate_seed(1788728940+hour*3600,w,h),hour,w,h);
      assert(memcmp(&a,&b,sizeof(a))!=0);
    }
  }
  puts("Ishihara: all hours and screen sizes, source mask proportions, minute/day rollover, deterministic packing and color encoding pass.");
}
