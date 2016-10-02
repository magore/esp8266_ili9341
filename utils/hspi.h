/*
 * hspi.h
 *
 *  Created on: 12 џэт. 2015 у.
 *      Author: Sem
 */

#ifndef INCLUDE_HSPI_H_
#define INCLUDE_HSPI_H_

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

#define SPI         0
#define HSPI        1
#define HSPI_FIFO_SIZE  64

#define CONFIG_FOR_TX           1
#define CONFIG_FOR_RX_TX        2

/* hspi.c */
void hspi_cs_enable ( uint8_t cs );
void hspi_cs_disable ( uint8_t cs );
void hspi_init ( uint16_t prescale , int hwcs );
void hspi_waitReady ( void );
void hspi_TX ( uint8_t *data , int count );
void hspi_TXRX ( uint8_t *data , int count );
void hspi_RX ( uint8_t *data , int count );

#endif                                            /* INCLUDE_HSPI_H_ */
