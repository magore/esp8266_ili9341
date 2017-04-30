/**
 @file hal.c

 @brief HAL layer
  SPI, GPIO, address and chip select 
 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  Please retain a copy of this notice in any code you use it in.

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
#include  "hal.h"

#ifdef ESP8266
#include <eagle_soc.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef ESP8266
#include "hspi.h"
#include "gpio.h"
#endif

#ifdef AVR
#include "hardware/iom1284p.h"
#endif

/// ==========================================================
/// @brief GPIO HAL
/** 
 @brief set GPIO SFR to permit normal GPIO mode
 @param[in] pin: gpin pin number
 @return void
*/
void 
gpio_pin_sfr_mode(int pin)
{
#ifdef ESP8266
	switch(pin)
	{

		case  0:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
			break;
		case  1:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
			break;
		case  2:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
			break;
		case  3:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
			break;
		case  4: // some esp8266-12 boards have incorrect labels 4 and 5 swapped
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
			break;
		case  5: // some esp8266-12 boards have incorrect labels 4 and 5 swapped
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
			break;
		case  6:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, FUNC_GPIO6);
			break;
		case  7:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, FUNC_GPIO7);
			break;
		case  8:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, FUNC_GPIO8);
			break;
		case  9:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9);
			break;
		case 10:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10);
			break;
		case 11:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, FUNC_GPIO11);
			break;
		case 12:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
			break;
		case 13:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
			break;
		case 14:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
			break;
		case 15:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
			break;
		case 16:
			// mux configuration for XPD_DCDC to output rtc_gpio0
			WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
			   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	
			//mux configuration for out enable
			WRITE_PERI_REG(RTC_GPIO_CONF,
			   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);
			//out enable
			WRITE_PERI_REG(RTC_GPIO_ENABLE,
			   (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);
			break;
	}
#endif
#ifdef AVR
	//TODO
#endif
}

#ifdef ESP8266
/** 
 @brief set GPIO16 pin direction
 @param[in] out: 1 for output, 0 for input
 @return void
*/
void 
gpio16_pin_dir(uint8_t out)
{
// mux XPD_DCDC to rtc_gpio0
    WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
	   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & (uint32_t)0xffffffbc) | (uint32_t)1L); 	

// mux out enable
    WRITE_PERI_REG(RTC_GPIO_CONF,
	   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32_t)0xfffffffe) | (uint32_t)0L);	

// out enable
    WRITE_PERI_REG(RTC_GPIO_ENABLE,
	   (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32_t)0xfffffffe) | (uint32_t)out);	
}

#endif // ESP8266


/** 
 @brief set GPIO pin state HI or LOW
 @param[in] pin: GPIO pin number
 @param[in] val: 1 for output, 0 for input
 @return void
*/
void gpio_pin_out(uint8_t pin, uint8_t val)
{
#ifdef ESP8266
	if(pin == 16)
	{
		if(val)
			GPIO16_PIN_HI();
		else
			GPIO16_PIN_LOW();
	}
	else
#endif
	if(val)
		GPIO_PIN_HI(pin);
	else
		GPIO_PIN_LOW(pin);
}

/** 
 @brief read GPIO pin 
 @param[in] pin: GPIO pin number
 @return 1 or 0
*/
uint8_t gpio_pin_rd(uint8_t pin)
{
#ifdef ESP8266
	if(pin == 16)
	{
		return( GPIO16_PIN_RD() );
	}
	else 
#endif
	return( GPIO_PIN_RD(pin) );
}


/// ==========================================================
/// @brief CHIP select HAL
/** 
 @brief initilize GPIO as an active HI chip select
 @param[in] pin: GPIO pin number
 @return void
*/
void chip_select_init(uint8_t pin)
{
//FIXME add address decoder options
#ifdef HAVE_DECODER
#error add address decoder code
#endif
	GPIO_PIN_HI(pin);
	GPIO_PIN_MODE(pin);
}

/** 
 @brief set GPIO to select - LOW
 @param[in] pin: GPIO pin number
 @return void
*/
void chip_select(uint8_t pin)
{
//FIXME add address decoder options
#ifdef HAVE_DECODER
#error add address decoder code
#endif
	GPIO_PIN_LOW(pin);
}

/** 
 @brief set GPIO to deselect - HI
 @param[in] pin: GPIO pin number
 @return void
*/
void chip_deselect(uint8_t pin)
{
//FIXME add address decoder options
#ifdef HAVE_DECODER
#error add address decoder code
#endif
	GPIO_PIN_HI(pin);
}

/// ==========================================================
/// @brief ADDRESS select HAL
/** 
 @brief initialize GPIO pin as address lines for a device
 @return void
*/
void chip_addr_init()
{
#ifdef ADDR_0
	GPIO_PIN_LOW(ADDR_0);
	GPIO_PIN_MODE(ADDR_0);
#endif

#ifdef ADDR_1
	GPIO_PIN_LOW(ADDR_1);
	GPIO_PIN_MODE(ADDR_1);
#endif

#ifdef ADDR_2
	GPIO_PIN_LOW(ADDR_2);
	GPIO_PIN_MODE(ADDR_2);
#endif

#ifdef ADDR_3
	GPIO_PIN_LOW(ADDR_3);
	GPIO_PIN_MODE(ADDR_3);
#endif
}

/** 
 @brief set address on GPIO lines
 @see chip_addr_init
 @param[in] addr: device address
 @return void
*/
void chip_addr(int addr)
{

#ifdef ADDR_0

	if(addr & 1)
		GPIO_PIN_HI(ADDR_0);
	else
		GPIO_PIN_LOW(ADDR_0);
#endif
#ifdef ADDR_1
	if(addr & 2)
		GPIO_PIN_HI(ADDR_1);
	else
		GPIO_PIN_LOW(ADDR_1);
#endif
#ifdef ADDR_2
	if(addr & 4)
		GPIO_PIN_HI(ADDR_2);
	else
		GPIO_PIN_LOW(ADDR_2);
#endif
#ifdef ADDR_3
	if(addr & 8)
		GPIO_PIN_HI(ADDR_3);
	else
		GPIO_PIN_LOW(ADDR_3);
#endif
}

/// ==========================================================
/// @brief SPI bus wrapper functions for multiple device support
/** 
 @brief Function waits for current SPI tranaction to finish before proceeding
 @return void
*/
void spi_waitReady()
{
#ifdef AVR
#endif
#ifdef ESP8266
    hspi_waitReady();
#endif
}


/** 
 @brief SPI init function
  Function waits for current tranaction to finish before proceeding
 @see spi_waitReady
 @see chip_select
 @param[in] clock: SPI clock rate
 @param[in] pin: GPIO CS pin
 @return void
*/
uint8_t _cs_pin = 0xff;
uint32_t _spi_clock = -1L;
void spi_init(uint32_t clock, int pin)
{
    spi_waitReady();
    chip_deselect(pin);
    _cs_pin = 0xff;

#ifdef AVR
	SPI0_Init(clock);   // Initialize the SPI bus - does nothing if clock unchanged
	SPI0_Mode(0);       // Set the clocking mode, etc
#endif
#ifdef ESP8266
	hspi_init(clock,0); // Initialize the SPI bus - does nothing if clock unchanged
#endif
	spi_TX(0xff);
	_spi_clock = clock;
	// waits for any prior transactions to complete before updating
    spi_waitReady();
}

/** 
 @brief SPI chip enable function
  Function waits for current tranaction to finish before proceeding
 @see spi_waitReady
 @see chip_select
 @param[in] clock: SPI clock rate
 @param[in] pin: GPIO CS pin
 @return void
*/
void spi_begin(uint32_t clock, int pin)
{
    // FIXME allow nesting by using an array of clock values for each pin

	//@brief if there is a prior chip select in progress flag an error
    if(_cs_pin != 0xff)
    {
        // This implies a bug!
        printf("cs_enable was: %d, want: %d\n", 0xff & _cs_pin, pin);
    }

	// waits for any prior transactions to complete before updating
    spi_waitReady();

	///@brief initialize pin in case it has not been done yet
	///@ we cache the clock frequency seeting for multiple device support
	if(_spi_clock != clock)
	{
		spi_init(clock,pin);
	}

    chip_select(pin);
    _cs_pin = pin;
}

/** 
 @brief SPI chip disable function
  wait for current tranaction to finish!
 @see spi_waitReady
 @see chip_deselect
 @param[in] pin: GPIO CS pin
 @return void
*/
void spi_end(uint8_t pin)
{
// DEBUG
    // FIXME allow nesting
    if(_cs_pin != pin && _cs_pin != 0xff )
    {
        // This implies a bug!
        printf("cs_disable was: %d, want: %d\n", 0xff & _cs_pin, pin);
    }
    spi_waitReady();
	chip_deselect(pin);
    _cs_pin = 0xff;
}

/// @brief SPI CS pin status
/// return CS GPIO pin number or 0xff
uint8_t spi_chip_select_status()
{
    return(_cs_pin);
}


/// @brief SPI write buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
void spi_TX_buffer(const uint8_t *data, int count)
{
#ifdef ESP8266
    hspi_TX((uint8_t *) data,count);
#endif
#ifdef AVR
    SPI0_TX((uint8_t *) data,count);
#endif
}

/// @brief SPI read buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
void spi_RX_buffer(const uint8_t *data, int count)
{
#ifdef ESP8266
    hspi_RX((uint8_t *) data,count);
#endif
#ifdef AVR
    SPI0_RX((uint8_t *)data,count);
#endif
}

/// @brief SPI write/read buffer
/// @param[in] *data: transmit/receive buffer
/// @param[in] count: number of bytes to write
/// @return  void
void spi_TXRX_buffer(const uint8_t *data, int count)
{
#ifdef ESP8266
    hspi_TXRX((uint8_t *) data,count);
#endif
#ifdef AVR
    SPI0_TXRX((uint8_t *) data,count);
#endif
}

/// @brief SPI read 1 byte
/// @return  uint8_t value
uint8_t spi_RX()
{
    uint8_t data;
#ifdef ESP8266
    hspi_RX(&data,1);
#endif
#ifdef AVR
    SPI0_RX(&data,1);
#endif
	return(data);
}

/// @brief SPI write 1 byte
/// @param[in] data: value to transmit
/// @return  void
void spi_TX(uint8_t data)
{
#ifdef ESP8266
    hspi_TX(&data,1);
#endif
#ifdef AVR
    SPI0_TX(&data,1);
#endif
}


/// @brief SPI read and write 1 byte
/// @param[in] data: value to transmit
/// @return  uint8_t value read
uint8_t spi_TXRX(uint8_t data)
{
#ifdef ESP8266
    hspi_TXRX(&data,1);
#endif
#ifdef AVR
    SPI0_TXRX(&data,1);
#endif
    return(data);
}
