#ifndef _FATFS_H_
#define _FATFS_H_

#include "time.h"
#include "timer.h"

// FATFS
#ifdef AVR
#define MMC_SLOW (500000UL)
#define MMC_FAST (2500000UL)
#endif
#ifdef ESP8266
#define MMC_SLOW (F_CPU/500000UL)
#define MMC_FAST (F_CPU/2500000UL)
#endif

#include "ffconf.h"
#include "integer.h"
#include "ff.h"
#include "diskio.h"
#include "fatfs_sup.h"
#include "mmc_hal.h"
#include "mmc.h"

// FATFS POSIX WRAPPER
#ifdef POSIX_WRAPPERS
#include "posix.h"
#endif


// FATFS user tests and user interface
#include "fatfs_utils.h"

#endif
