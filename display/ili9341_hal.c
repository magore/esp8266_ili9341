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
uint16_t tft_clock = -1;

uint16_t tft_ID;

#ifdef ESP8266
#define tft_delay_us(a) os_delay_us(a)
// We use automatic CS mode configured with hspi
#define TFT_CS_PIN		15
// Note: PERIPHS_IO_MUX_MTDO_U, is tied to GPIO 15 alternate function
#define TFT_CS_INIT     gpio_io_mode(TFT_CS_PIN);

#define TFT_CS_ACTIVE   GPIO_OUTPUT_SET(TFT_CS_PIN, 0)
#define TFT_CS_DEACTIVE GPIO_OUTPUT_SET(TFT_CS_PIN, 1)
// Display reset
// Alternative we just tie this to power on reset line and free up the line
#if 0
	#define TFT_RST_PIN		0
	#define TFT_RST_INIT     gpio_io_mode(TFT_RST_PIN);
	#define TFT_RST_ACTIVE    GPIO_OUTPUT_SET(TFT_RST_PIN, 0)
	#define TFT_RST_DEACTIVE  GPIO_OUTPUT_SET(TFT_RST_PIN, 1)
#else
	#define TFT_RST_PIN		
	#define TFT_RST_INIT     
	#define TFT_RST_ACTIVE
	#define TFT_RST_DEACTIVE
#endif

#ifdef SWAP45
        // Pin 4 and 5 are mislabled
	#define TFT_CMD_DATA_PIN	4
	#define TFT_INIT        gpio_io_mode(TFT_CMD_DATA_PIN); TFT_DATA
	#define TFT_DATA        GPIO_OUTPUT_SET(TFT_CMD_DATA_PIN, 1)
	#define TFT_COMMAND     GPIO_OUTPUT_SET(TFT_CMD_DATA_PIN, 0)
#else
	#define TFT_CMD_DATA_PIN	5
	#define TFT_INIT        gpio_io_mode(TFT_CMD_DATA_PIN);TFT_DATA
	#define TFT_DATA        GPIO_OUTPUT_SET(TFT_CMD_DATA_PIN, 1)
	#define TFT_COMMAND     GPIO_OUTPUT_SET(TFT_CMD_DATA_PIN, 0)
#endif

#endif

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_init(uint16_t prescale)
{
	// start with slow SPI, no hardware CS
    tft_spi_end();
	hspi_init( (tft_clock = prescale) , 0);
}

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_begin()
{
    hspi_waitReady();
	hspi_init(tft_clock, 0);
	hspi_cs_enable(TFT_CS_PIN);
    //TFT_CS_ACTIVE;
}

/// @brief  Release SPI bus from TFT display, deassert chip select
/// return: void
void tft_spi_end()
{
    hspi_waitReady();
	hspi_cs_disable(TFT_CS_PIN);
    //TFT_CS_DEACTIVE;
}

/// @brief  Transmit 8 bit data array
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_TX(uint8_t *data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_TX(data,bytes);
}

/// @brief  Transmit and read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_TXRX(uint8_t * data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_TXRX(data,bytes);
}


/// @brief  read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// @param[in] command: 1 = command, 0 = data 
/// return: void 
void tft_spi_RX(uint8_t *data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_RX(data,bytes);
}

/// @brief Initialize TFT
/// @return diplay ID 9341
MEMSPACE
window *tft_init(void)
{
    TFT_RST_INIT;	// RESET PIN
    TFT_INIT;		// DATA/COMMAND
    TFT_CS_INIT;	// CHIP SELLECT
	hspi_cs_disable(TFT_CS_PIN);
	// TFT_CS_DISABLE;

	// Start with slow clock so tft_readId works
	// This is the only function that fails at less then 1.
	// tft_readId is the ONLY SPI bus command that needs this.
	// Nomal reads work fine.
	tft_spi_init(2);
    TFT_RST_ACTIVE;	
    tft_delay_us(10000);
    TFT_RST_DEACTIVE;
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
