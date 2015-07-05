/**
 @file ili9341_adafruit.h
 @par Copyright (c) 2013 Adafruit Industries.  All rights reserved.
  Minor Revisions by 2015 Mike Gore to add multiple window support
 @see https://github.com/adafruit/Adafruit-GFX-Library
*/

/**
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _ILI9341_ADAFRUIT_H_
#define _ILI9341_ADAFRUIT_H_

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <mem.h>
#include "util.h"
#include "hspi.h"
#include "font.h"
#include "ili9341.h"

typedef union
{
    struct
    {
        uint8_t b0 :8;
        uint8_t b1 :8;
        uint8_t b2 :8;
        uint8_t b3 :8;
    } bytes;

    uint32_t all;
} uint32_t_bytes;

// ==========================================================
// We use automatic CS mode configured with hspi
#ifdef TFT_USE_CS
#define TFT_CS_ACTIVE   GPIO_OUTPUT_SET(4, 0)
#define TFT_CS_DEACTIVE GPIO_OUTPUT_SET(4, 1)
#define TFT_CS_INIT     PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); TFT_CS_DEACTIVE
#else
#define TFT_CS_ACTIVE
#define TFT_CS_DEACTIVE
#define TFT_CS_INIT
#endif

#define TFT_DATA     GPIO_OUTPUT_SET(2, 1)
#define TFT_COMMAND  GPIO_OUTPUT_SET(2, 0)
#define TFT_INIT     PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); TFT_DATA
#define TFT_RST_ACTIVE    GPIO_OUTPUT_SET(4, 0)
#define TFT_RST_DEACTIVE  GPIO_OUTPUT_SET(4, 1)
#define TFT_RST_INIT     PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); TFT_RST_DEACTIVE

#define MIN_TFT_Y               0
#define MAX_TFT_Y               319
#define MIN_TFT_X               0
#define MAX_TFT_X               239
// ==========================================================

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_BGR 0x08
#define MADCTL_RGB 0x00
#define MADCTL_MH  0x04

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
/*
#define ILI9341_PWCTR6  0xFC

// ========================================================================
*/

// Color definitions
#define ILI9341_BLACK       0x0000      /,   0,   0 */
#define ILI9341_NAVY        0x000F                /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0                /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF                /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800                /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F                /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0                /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618                /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF                /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F                /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0                /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF                /*   0, 255, 255 */
#define ILI9341_RED         0xF800                /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F                /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0                /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF                /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20                /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5                /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

// ============================================================

/* ili9341_adafruit.c */
MEMSPACE void tft_configRegister ( void );
MEMSPACE void tft_drawBitmap ( window *win , int16_t x , int16_t y , const uint16_t *bitmap , int16_t w , int16_t h );
MEMSPACE void tft_drawCircle ( window *win , int16_t x0 , int16_t y0 , int16_t r , uint16_t color );
MEMSPACE void tft_drawCircleHelper ( window *win , int16_t x0 , int16_t y0 , int16_t r , uint8_t cornername , uint16_t color );
MEMSPACE void tft_fillCircleHelper ( window *win , int16_t x0 , int16_t y0 , int16_t r , uint8_t cornername , int16_t delta , uint16_t color );
MEMSPACE void tft_fillCircle ( window *win , int16_t x0 , int16_t y0 , int16_t r , uint16_t color );
MEMSPACE void tft_drawRect ( window *win , int16_t x , int16_t y , int16_t w , int16_t h , uint16_t color );
MEMSPACE void tft_drawRoundRect ( window *win , int16_t x , int16_t y , int16_t w , int16_t h , int16_t r , uint16_t color );
MEMSPACE void tft_fillRoundRect ( window *win , int16_t x , int16_t y , int16_t w , int16_t h , int16_t r , uint16_t color );
MEMSPACE void tft_drawTriangle ( window *win , int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , int16_t x2 , int16_t y2 , uint16_t color );
MEMSPACE void tft_fillTriangle ( window *win , int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , int16_t x2 , int16_t y2 , uint16_t color );
#endif                                            // _ILI9341_ADAFRUIT_H_
