#ifndef _USER_TASK_H_
#define _USER_TASK_H_

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef USER_CONFIG
#include "user_config.h"
#endif

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif


/* user_task.c */
int atexit ( void (*func )());
void abort ( void );
void esp_yield ( void );
void esp_schedule ( void );
//void __yield ( void );
void yield ( void );
void loop_wrapper ( void );
void init_done ( void );
void user_init ( void );

#endif                                            /* _USER_TASK_H_ */
