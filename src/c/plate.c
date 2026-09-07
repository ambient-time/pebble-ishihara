// Copyright 2026 Luke Steuber. MIT License.
#include "plate.h"
#include "masks.h"
#include <string.h>

// Preserve the source's random draw order, including rejected candidates.
static double next(uint32_t *state) {
  uint32_t a=(*state+=0x6d2b79f5U),t=(a^(a>>15))*(1U|a);
  t=(t+(t^(t>>7))*(61U|t))^t;
  return (t^(t>>14))/4294967296.0;
}
int plate_hour(int h) { h=(h%24+24)%24;return h%12?h%12:12; }
static int floor_int(double x) {int i=(int)x;return i-(x<i);}
static double sin_small(double x) {
  if(x>3.14159265358979323846)x-=6.28318530717958647692;
  double term=x,sum=x;
  for(int n=1;n<=12;n++) {term*=-x*x/((2*n)*(2*n+1));sum+=term;}
  return sum;
}
static double cos_small(double x) {
  if(x>3.14159265358979323846)x-=6.28318530717958647692;
  double term=1,sum=1;
  for(int n=1;n<=12;n++) {term*=-x*x/((2*n-1)*(2*n));sum+=term;}
  return sum;
}
// c^(1/2.4) = the twelfth root of c^5. Bisection avoids the SDK libm's
// non-relocatable double constants; 44 steps keep error below 6e-14.
static double gamma_power(double c) {
  double c2=c*c,target=c2*c2*c,lo=0,hi=1;
  for(int i=0;i<44;i++) {
    double mid=(lo+hi)*.5,m2=mid*mid,m6=m2*m2*m2;
    if(m6*m6<target)lo=mid;else hi=mid;
  }
  return (lo+hi)*.5;
}
static double radius(int w,int h) { return (w<h?w:h)*.44; }
uint32_t plate_seed(int64_t epoch,int w,int h) {
  int64_t minute=epoch/60;if(epoch<0&&epoch%60)minute--;
  return (uint32_t)minute^0x9e37U^((uint32_t)floor_int(radius(w,h)+.5)*131U);
}
bool plate_mask(int hour,int size,int x,int y) {
  return hour_mask(size,plate_hour(hour),x,y);
}

// CIELAB -> D65 XYZ -> sRGB, using the pinned source's constants and rounding.
static double inverse(double t) {
  const double delta=6.0/29.0;
  return t>delta?t*t*t:3*delta*delta*(t-4.0/29.0);
}
static uint8_t channel(double c) {
  c=(c<0?0:c>1?1:c);
  double gamma=c<=.0031308?12.92*c:1.055*gamma_power(c)-.055;
  int byte=(int)floor_int(gamma*255+.5);
  return (uint8_t)((byte+42)/85);
}
uint8_t plate_lab_color(double L,double a,double b) {
  double fy=(L+16)/116;
  double X=.95047*inverse(fy+a/500),Y=inverse(fy),Z=1.08883*inverse(fy-b/200);
  return 0xc0 | channel(3.2404542*X-1.5371385*Y-.4985314*Z)<<4
    | channel(-.9692660*X+1.8760108*Y+.0415560*Z)<<2
    | channel(.0556434*X-.2040259*Y+1.0572252*Z);
}
typedef struct {double L,LR,a,aR,b,bR;} LabRecipe;
static const LabRecipe RECIPES[3][2]={
  {{65,10,53,6,-3,8},{65,10,-23,6,9,8}}, // deutan: background, figure
  {{65,10,45,6,5,8},{65,10,-39,6,-3,8}}, // protan
  {{50,12,41,8,-55,8},{50,12,-23,8,53,8}} // tritan
};

// Static scratch space keeps the watch's small application stack bounded.
static int16_t grid[PLATE_MAX_GRID*PLATE_MAX_GRID];
static uint16_t active[PLATE_MAX_DOTS];
static int grid_width,active_count;
static double cell,min_radius,max_radius;
static int grid_index(Plate *plate,double x,double y) {
  int gx=(int)floor_int((x-plate->cx+plate->radius)/cell);
  int gy=(int)floor_int((y-plate->cy+plate->radius)/cell);
  return gx<0||gy<0||gx>=grid_width||gy>=grid_width?-1:gy*grid_width+gx;
}
static bool add(Plate *plate,uint32_t *state,double x,double y) {
  double r=min_radius+next(state)*(max_radius-min_radius);
  double bx=x-plate->cx,by=y-plate->cy,limit=plate->radius-1-r;
  if(bx*bx+by*by>limit*limit)return false;
  int index=grid_index(plate,x,y);if(index<0)return false;
  int gx=index%grid_width,gy=index/grid_width;
  for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++) {
    int nx=gx+dx,ny=gy+dy;
    if(nx<0||ny<0||nx>=grid_width||ny>=grid_width)continue;
    int other=grid[ny*grid_width+nx];
    if(other>=0) {
      const PlateDot *o=&plate->dots[other];
      double dx=x-o->x,dy=y-o->y,gap=r+o->r+.8;
      if(dx*dx+dy*dy<gap*gap)return false;
    }
  }
  if(plate->count==PLATE_MAX_DOTS)return false;
  int i=plate->count++;
  plate->dots[i]=(PlateDot){.x=x,.y=y,.r=r};
  active[active_count++]=i;grid[index]=i;return true;
}
void plate_build(Plate *plate,uint32_t seed,int hour,int w,int h) {
  memset(plate,0,sizeof(*plate));memset(grid,-1,sizeof(grid));active_count=0;
  plate->cx=w*.5;plate->cy=h*.5;plate->radius=radius(w,h);plate->hour=plate_hour(hour);
  double roll=next(&seed);plate->palette=roll<.45?0:roll<.82?1:2;
  min_radius=plate->radius*.014;if(min_radius<2)min_radius=2;
  max_radius=plate->radius*.036;if(max_radius<4.5)max_radius=4.5;
  cell=min_radius*1.5/1.4142135623730951;
  double cells=plate->radius*2/cell;grid_width=(int)cells+(cells>(int)cells);
  if(grid_width>PLATE_MAX_GRID)return;
  add(plate,&seed,plate->cx,plate->cy);
  while(active_count>0) {
    int ai=(int)floor_int(next(&seed)*active_count);
    const PlateDot *src=&plate->dots[active[ai]];bool found=false;
    for(int i=0;i<30;i++) {
      double angle=next(&seed)*3.14159265358979323846*2;
      double distance=min_radius*2+next(&seed)*min_radius*2.5;
      if(add(plate,&seed,src->x+cos_small(angle)*distance,src->y+sin_small(angle)*distance)) {found=true;break;}
    }
    if(!found) {
      active_count--;
      memmove(active+ai,active+ai+1,(active_count-ai)*sizeof(*active));
    }
  }
  // Color jitter follows packing, as in the source. Geometry consumes no colors.
  int size=(int)floor_int(plate->radius*2+.5);
  for(int i=0;i<plate->count;i++) {
    PlateDot *dot=&plate->dots[i];
    int x=(int)floor_int(dot->x-(plate->cx-plate->radius));
    int y=(int)floor_int(dot->y-(plate->cy-plate->radius));
    dot->shape=plate_mask(hour,size,x,y);
    LabRecipe recipe=RECIPES[plate->palette][dot->shape];
    // Explicit temporaries preserve JavaScript's left-to-right evaluation.
    double L=recipe.L+(next(&seed)-.5)*recipe.LR;
    double a=recipe.a+(next(&seed)-.5)*recipe.aR;
    double b=recipe.b+(next(&seed)-.5)*recipe.bR;
    dot->color=plate_lab_color(L,a,b);
  }
}

uint32_t plate_signature(const Plate *plate) {
  uint32_t signature=2166136261U;
  for(int i=0;i<plate->count;i++) {
    const PlateDot *d=&plate->dots[i];
    const uint16_t values[]={(uint16_t)(d->x+.5),(uint16_t)(d->y+.5),(uint16_t)(d->r+.5),d->shape,d->color};
    for(int j=0;j<5;j++)signature=(signature^values[j])*16777619U;
  }
  return signature;
}
