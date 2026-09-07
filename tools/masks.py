"""Rebuild the smooth hour masks from the bundled OFL Barlow font."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parent.parent
output=['// Copyright 2026 Luke Steuber. MIT License.','// Rendered from Barlow Condensed Bold, SIL OFL 1.1. See resources/fonts/OFL.txt.','#pragma once','#include <stdint.h>','static const uint8_t HOUR_MASKS[12][512] = {']
for hour in range(1,13):
    font=ImageFont.truetype(str(ROOT/'resources/fonts/BarlowCondensed-Bold.ttf'),250 if hour<10 else 225)
    text=str(hour);bbox=font.getbbox(text,stroke_width=3)
    glyph=Image.new('L',(bbox[2]-bbox[0],bbox[3]-bbox[1]))
    ImageDraw.Draw(glyph).text((-bbox[0],-bbox[1]),text,font=font,fill=255,stroke_width=3,stroke_fill=255)
    factor=1.4 if hour<10 else 1.15
    glyph=glyph.resize((round(glyph.width*factor),glyph.height),Image.Resampling.LANCZOS)
    if glyph.width>204:glyph=glyph.resize((204,round(glyph.height*204/glyph.width)),Image.Resampling.LANCZOS)
    image=Image.new('L',(256,256));image.paste(glyph,((256-glyph.width)//2,(256-glyph.height)//2))
    image=image.resize((64,64),Image.Resampling.LANCZOS)
    data=[]
    for y in range(64):
        for x in range(0,64,8):data.append(sum((image.getpixel((x+b,y))>127)<<b for b in range(8)))
    output.append('  { // '+str(hour))
    for start in range(0,512,32):output.append('    '+','.join(f'0x{n:02x}' for n in data[start:start+32])+',')
    output.append('  },')
output.append('};')
(ROOT/'src/c/masks.h').write_text('\n'.join(output)+'\n')
