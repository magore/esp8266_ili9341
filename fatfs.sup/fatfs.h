#ifndef _FATFS_H_
#define _FATFS_H_

#include "time.h"
#include "timer.h"

// FATFS
#define MMC_SLOW (F_CPU/500000UL)
#define MMC_FAST (F_CPU/2500000UL)
#include "ffconf.h"
#include "integer.h"
#include "ff.h"
#include "diskio.h"
#include "fatfs_sup.h"
#include "mmc_hal.h"
#include "mmc.h"

// FATFS POSIX WRAPPER
#include "posix.h"

// FATFS user tests and user interface
#include "fatfs_utils.h"

#endif
