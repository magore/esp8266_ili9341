/**
 @file ili9341_hal.c

 @brief ili9341 driver inspired by Adafruit ili9341 code
        All code in this file has been rewritten by Mike Gore
 @par Copyright &copy; 2016 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

  This is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option)
  any later version.
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "display/ili9341.h"

// TFT master window definition
extern window tftwin;
extern window *tft;

/// =============================================================
/// =============================================================
/// Start SPI Hardware Abstraction Layer
/// Keep all hardware dependent SPI code in this section

/// @brief cahce of SPI clock devisor
uint32_t tft_clock = -1;

uint16_t tft_ID;

#ifdef ESP8266
#ifndef ILI9341_CS
#error ILI9341_CS is undefined
#endif
#define tft_delay_us(a) os_delay_us(a)

#ifndef ADDR_0
#error ADDR_0 is undefined
#endif

#define TFT_DATA        chip_addr(1)
#define TFT_COMMAND     chip_addr(0)

#endif

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_init(uint32_t prescale)
{
	spi_init( (tft_clock = prescale) , ILI9341_CS);
}

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_begin()
{
	spi_begin(tft_clock, ILI9341_CS);
}

/// @brief  Release SPI bus from TFT display, deassert chip select
/// return: void
void tft_spi_end()
{
    spi_end(ILI9341_CS);
}

/// @brief  Initialize ILI9341 reset GPIO
/// return: void
void tft_reset_init()
{
#ifdef ILI9341_RESET
	chip_select_init(ILI9341_RESET);
#endif
}

/// @brief  enable ILI9341 reset 
/// return: void
void tft_reset_enable()
{
#ifdef ILI9341_RESET
	chip_enable(ILI9341_RESET);
#endif
}

/// @brief  Initialize ILI9341 command/data GPIO
/// return: void
void tft_addr_init()
{
	chip_addr_init();
}


/// @brief  disnable ILI9341 reset 
/// return: void
void tft_reset_disable()
{
#ifdef ILI9341_RESET
	chip_disable(ILI9341_RESET);
#endif
}



/// @brief  Transmit 8 bit data array
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_TX(uint8_t *data, int bytes, uint8_t command)
{
	spi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	spi_TX_buffer(data,bytes);
}

/// @brief  Transmit and read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_TXRX(uint8_t * data, int bytes, uint8_t command)
{
	spi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	spi_TXRX_buffer(data,bytes);
}


/// @brief  read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_RX(uint8_t *data, int bytes, uint8_t command)
{
	spi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	spi_RX_buffer(data,bytes);
}

/// @brief Initialize TFT
/// @return diplay ID 9341
MEMSPACE
window *tft_init(void)
{
	// Start with slow clock so tft_readId works
	// This is the only function that fails at less then 1.
	// tft_readId is the ONLY SPI bus command that needs this.
	// Nomal reads work fine.
	tft_reset_init();
	tft_addr_init();
	tft_spi_init(2);
    tft_reset_enable();
    tft_delay_us(10000);
    tft_reset_disable();
    tft_delay_us(1000);

	/* Adafruit 9341 TFT Display Initialization */
    tft_configRegister();

	/* Read the TFT ID value */
    tft_ID = tft_readId();

	// fast SPI
	tft_spi_init(1);

	/* Setup the master window */
    tft_window_init(tft, TFT_XOFF, TFT_YOFF, TFT_W, TFT_H);
    tft_setRotation(0);
    tft_fillWin(tft, tft->bg);

    return (tft);
}

/// End of SPI HAL interface
/// =============================================================
/// =============================================================
