// Copyright 2026 Luke Steuber. MIT License.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PLATE_MAX_DOTS 650
typedef struct {uint8_t x,y,r,style;} PlateDot;
typedef struct {PlateDot dots[PLATE_MAX_DOTS];uint16_t count;uint8_t palette,hour;} Plate;
int plate_hour(int local_hour);
uint32_t plate_seed(int64_t epoch_seconds);
bool plate_mask(int hour,int x,int y);
void plate_build(Plate *plate,uint32_t minute_seed,int local_hour);
void plate_readout(char out[6],int hour,int minute,bool twenty_four);
