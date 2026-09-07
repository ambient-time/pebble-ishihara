// Copyright 2026 Luke Steuber. MIT License.
#include "plate.h"
#include "masks.h"
#include <string.h>
static uint32_t next(uint32_t *state){
  // The source's mulberry32 generator, with defined unsigned wraparound.
  uint32_t a=(*state+=0x6d2b79f5U),t=(a^(a>>15))*(1U|a);
  t=(t+(t^(t>>7))*(61U|t))^t;return t^(t>>14);
}
int plate_hour(int h){h=(h%24+24)%24;return h%12?h%12:12;}
uint32_t plate_seed(int64_t epoch){
  int64_t minute=epoch/60;if(epoch<0&&epoch%60)minute--;
  return (uint32_t)minute^0x9e37U^(62U*131U);
}
bool plate_mask(int hour,int x,int y){
  if(x<0||x>=128||y<0||y>=128)return false;
  x/=2;y/=2;return (HOUR_MASKS[plate_hour(hour)-1][y*8+x/8]>>(x%8))&1;
}
void plate_build(Plate *plate,uint32_t seed,int local_hour){
  static int16_t heads[16*16],links[PLATE_MAX_DOTS];
  memset(heads,-1,sizeof(heads));memset(plate,0,sizeof(*plate));
  uint32_t state=seed,roll=next(&state)%100;
  plate->palette=roll<45?0:roll<82?1:2;plate->hour=plate_hour(local_hour);
  // Bounded spatial rejection packing: organic positions and varying radii,
  // with a one-pixel gap. No frame loop or runtime allocation is involved.
  for(int attempt=0;attempt<26000&&plate->count<PLATE_MAX_DOTS;attempt++){
    int x=next(&state)%124+2,y=next(&state)%124+2,r=next(&state)%3+2;
    int dx=x-64,dy=y-64,limit=62-r;
    if(dx*dx+dy*dy>limit*limit)continue;
    int gx=x/8,gy=y/8;bool clear=true;
    for(int yy=gy-2;yy<=gy+2&&clear;yy++)for(int xx=gx-2;xx<=gx+2&&clear;xx++){
      if(xx<0||xx>=16||yy<0||yy>=16)continue;
      for(int i=heads[yy*16+xx];i>=0;i=links[i]){
        PlateDot d=plate->dots[i];dx=x-d.x;dy=y-d.y;int gap=r+d.r+1;
        if(dx*dx+dy*dy<gap*gap){clear=false;break;}
      }
    }
    if(!clear)continue;
    int index=plate->count++;uint8_t style=(plate_mask(local_hour,x,y)?1:0)|((next(&state)%4)<<1);
    plate->dots[index]=(PlateDot){x,y,r,style};links[index]=heads[gy*16+gx];heads[gy*16+gx]=index;
  }
}
void plate_readout(char out[6],int hour,int minute,bool twenty_four){
  hour=(hour%24+24)%24;minute=(minute%60+60)%60;if(!twenty_four)hour=plate_hour(hour);
  out[0]=hour<10&&!twenty_four?' ':'0'+hour/10;out[1]='0'+hour%10;
  out[2]=':';out[3]='0'+minute/10;out[4]='0'+minute%10;out[5]=0;
}
