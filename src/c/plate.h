// Copyright 2026 Luke Steuber. MIT License.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#if defined(PBL_PLATFORM_GABBRO) || !defined(PBL_DISPLAY_WIDTH)
#define PLATE_MAX_DOTS 1100
#define PLATE_MAX_GRID 108
#elif defined(PBL_PLATFORM_EMERY)
#define PLATE_MAX_DOTS 700
#define PLATE_MAX_GRID 83
#elif defined(PBL_PLATFORM_CHALK)
#define PLATE_MAX_DOTS 600
#define PLATE_MAX_GRID 75
#else
#define PLATE_MAX_DOTS 400
#define PLATE_MAX_GRID 60
#endif
typedef struct { double x,y,r; uint8_t color,shape; } PlateDot;
typedef struct {
  PlateDot dots[PLATE_MAX_DOTS];
  double cx,cy,radius;
  uint16_t count;
  uint8_t palette,hour;
} Plate;
uint32_t plate_signature(const Plate *plate);
int plate_hour(int local_hour);
uint32_t plate_seed(int64_t epoch_seconds,int width,int height);
bool plate_mask(int hour,int mask_size,int x,int y);
uint8_t plate_lab_color(double L,double a,double b);
void plate_build(Plate *plate,uint32_t minute_seed,int local_hour,int width,int height);
