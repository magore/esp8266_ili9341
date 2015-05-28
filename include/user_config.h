
#define LOAD_FONT16
#define LOAD_FONT32

#define MEMSPACE ICACHE_FLASH_ATTR
//#define MEMSPACE_RO ICACHE_RODATA_ATTR
#define MEMSPACE_RO static const

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <mem.h>
#include <util.h>
#include <uart.h>
#include "newlib.h"
int snprintf(char *, size_t, const char *, ...);
#ifdef FORMAT_PRINTF
	#include "format.h"
	#include "format_config.h"
#endif
#ifdef MIKE_PRINTF
	#include "printf.h"
#endif

#define FONTSPECS
#define MEMSPACE_FONT ICACHE_FLASH_ATTR

#include "font.h"
#include "ili9341_adafruit.h"
#include "ili9341.h"
#include "cordic.h"
#ifdef EARTH
	#include "earth.h"
#endif
#ifdef CUBE
	#include "cube.h"
#endif

#define DELAY_TIMER 	10 // Delay timer in milliseconds
#define RUN_TEST  0
#define TEST_QUEUE_LEN  4

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#endif
