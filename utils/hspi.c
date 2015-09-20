/**
 @file hspi.c

 @brief HSPI driver for ESP8255
 Based on initial work from Sem 2015 - rewrittem
 Added code to handle proper aligned reads and writes and buffering
 @par Copyright &copy; 2015 Sem
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

#include <user_config.h>

// @brief HSPI Prescale value - You should set this in your Makefile
#ifndef HSPI_PRESCALER
#define HSPI_PRESCALER 16
#endif


/// @brief hspi CS cached value
uint8_t _cs_pin = 0xff;
/// @brief HSPI CS enable function
/// @param[in] cs: GPIO CS pin
void hspi_cs_enable(uint8_t cs)
{
    hspi_waitReady();
	if(_cs_pin != 0xff)
	{
		// This implies a bug!
		printf("CS en was:%d\n", 0xff & _cs_pin);
	}
	GPIO_OUTPUT_SET(cs, 0);
	_cs_pin = cs;
}

/// @brief HSPI CS disable function
/// @param[in] cs: GPIO CS pin
void hspi_cs_disable(uint8_t cs)
{
    hspi_waitReady();
	if(_cs_pin != cs && _cs_pin != 0xff )
	{
		// This implies a bug!
		printf("CS dis was:%d\n", 0xff & _cs_pin);
	}
	GPIO_OUTPUT_SET(cs, 1);
	_cs_pin = 0xff;
}


static _hspi_init_done = 0;

/// @brief hspi clock cached value
uint16_t hspi_clock = -1;
/// @brief HSPI Initiaization - with automatic chip select
/// Pins:
/// 	MISO GPIO12
/// 	MOSI GPIO13
/// 	CLK GPIO14
/// 	CS GPIO15 - optional
/// 	DC GPIO2
/// @param[in] prescale: prescale from CPU clock 0 .. 0x1fff
/// @param[in] hwcs: enable GPIO15 hardware chip select 
/// @return  void
void hspi_init(uint16_t prescale, int hwcs)
{

// FIXME DEBUG 
#if 1
	// We only make changes if the speed actually changes
	if(prescale == hspi_clock)
		return;
#endif

	// If we have been called make sure all spi transactions have finished before 
	// we change anything

    if(_hspi_init_done)
		hspi_waitReady();
	
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);    // HSPIQ MISO GPIO12
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);    // HSPID MOSI GPIO13
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);    // CLK        GPIO14

	// HARDWARE SPI CS
	if(hwcs)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); // CS AUTOMATIC ONLY ON GPIO15

	if(prescale == 0)
	{
		WRITE_PERI_REG(PERIPHS_IO_MUX, 0x305); //set bit9
		WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI), SPI_CLK_EQU_SYSCLK);
	}
	else
	{
		// prescale >= 1
		WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);        //clear bit9
		WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI),
        (((prescale - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
        ((1 & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
        ((0 & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
        ((1 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
/*
		WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI),
        (((prescale - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
        ((1 & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
        ((0 & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
        ((1 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
*/
	}

    WRITE_PERI_REG(SPI_FLASH_CTRL1(HSPI), 0);
	hspi_clock = prescale;
	_hspi_init_done = 1;
}

/// @brief HSPI Configuration for tranasmit and receive
/// @param[in] configState: CONFIG_FOR_RX_TX or CONFIG_FOR_RX
/// @return  void
static void hspi_config(int configState)
{
    uint32_t valueOfRegisters = 0;

    hspi_waitReady();

    if (configState == CONFIG_FOR_TX)
    {
        valueOfRegisters |=  SPI_FLASH_DOUT;
//clear bit 2 see example IoT_Demo
        valueOfRegisters &= \
            ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | \
            SPI_FLASH_USR_DIN | SPI_USR_COMMAND | SPI_DOUTDIN);
    }
    else if  (configState == CONFIG_FOR_RX_TX)
    {
        valueOfRegisters |=  SPI_FLASH_DOUT | SPI_DOUTDIN | SPI_CK_I_EDGE;
//clear bit 2 see example IoT_Demo
        valueOfRegisters &= \
            ~(BIT2 | SPI_FLASH_USR_ADDR | SPI_FLASH_USR_DUMMY | \
            SPI_FLASH_USR_DIN | SPI_USR_COMMAND);
    }
    else
    {
        return;                                   // Error
    }
    WRITE_PERI_REG(SPI_FLASH_USER(HSPI), valueOfRegisters);

}


/// @brief HSPI FIFO send or receive byte count
/// @param[in] bytes: bytes to send or receive
// Only called from hspi_writeFIFO or hspi_readFIFO
// So bounds testing has already been done
/// @return  void
static void hspi_setBits(uint16_t bytes)
{
    uint16_t bits = (bytes << 3) - 1;
    WRITE_PERI_REG(SPI_FLASH_USER1(HSPI),
        ( (bits & SPI_USR_OUT_BITLEN) << SPI_USR_OUT_BITLEN_S ) |
        ( (bits & SPI_USR_DIN_BITLEN) << SPI_USR_DIN_BITLEN_S ) );
}


/// @brief HSPI Start Send
/// @return  void
static void hspi_startSend(void)
{
    SET_PERI_REG_MASK(SPI_FLASH_CMD(HSPI), SPI_FLASH_USR);
}


/// @brief HSPI Ready wait
/// @return  void
void hspi_waitReady(void)
{
    while (READ_PERI_REG(SPI_FLASH_CMD(HSPI)) & SPI_FLASH_USR) {};
}


/// @brief HSPI FIFO write in units of 32 bits (4 byte words)
/// @param[in] write_data: write buffer
/// @param[in] bytes: bytes to write
/// @return  void
static void hspi_writeFIFO(uint8_t *write_data, int bytes)
{
    uint8_t word_ind = 0;

    if(bytes > HSPI_FIFO_SIZE)                    // TODO set error status
        return;

    hspi_setBits(bytes);                          // Update FIFO with number of bits we will send

// First do a fast write with 32 bit chunks at a time
    while(bytes >= 4)
    {
// Cast both source and destination to 4 byte word pointers
        ((uint32_t *)SPI_FLASH_C0(HSPI)) [word_ind] = \
            ((uint32_t *)write_data) [word_ind];

        bytes -= 4;
        word_ind++;
    }

// Next write remaining bytes (if any) to FIFO as a single word
// Note: We follow the good practice of avoiding access past the end of an array.
//	(ie. protected memory, PIC using retw from flash ... could crash some CPU's)
    if(bytes)                                     // Valid counts are now: 3,2,1 bytes
    {
        uint32_t last_word = 0;                   // Last word to send
        uint16_t byte_ind = word_ind << 2;        // Convert to Byte index

// Working MSB to LSB allows assigment to LSB without shifting
// Compiler can optimize powers of 8 into byte swaps, etc... (on some CPU's)
        while(bytes--)                            // index is now 2,1,0 matching required storage offset
        {
            last_word <<= 8;
// 2,1,0
            last_word |= ((uint32_t) 0xffU & write_data[byte_ind + bytes ]);
        }
// Send last partial word
        ((uint32_t *)SPI_FLASH_C0(HSPI)) [word_ind] = last_word;
    }
}


/// @brief HSPI FIFO Read in units of 32 bits (4 byte words)
/// @param[in] read_data: read buffer
/// @param[in] bytes: bytes to write
/// @return  void
static void hspi_readFIFO(uint8_t *read_data, int bytes)
{

    uint8_t word_ind = 0;

    if(bytes > HSPI_FIFO_SIZE)                    // TODO set error status
        return;

// Update FIFO with number of bits to read ?
    hspi_setBits(bytes);

// First do a fast read 32 bit chunks at a time
    while(bytes >= 4)
    {
// Cast both source and destination to 4 byte word pointers
        ((uint32_t *)read_data) [word_ind] = \
            ((uint32_t *)SPI_FLASH_C0(HSPI)) [word_ind];
        bytes -= 4;
        word_ind++;
    }

// Next read remaining bytes (if any) from FIFO as a single word
// Note: We follow the good practice of avoiding access past the end of an array.
//	(ie. protected memory, PIC using retw from flash ... could crash some CPU's)
    if(bytes)                                     // Valid counts are now: 3,2,1 bytes
    {
        uint32_t last_word = ((uint32_t *)SPI_FLASH_C0(HSPI)) [word_ind];
        uint16_t byte_ind = word_ind << 2;        // Convert to Byte index

        while(bytes--)                            // index is now 2,1,0 matching required storage offset
        {
// 2,1,0 order
            read_data[byte_ind++] = (uint8_t) 0xff & last_word;
            last_word >>= 8;
        }
// Send last partial word
    }
}




/// =================================================================
/// @brief
/// SPI buffered write functions

/// @brief HSPI write using FIFO
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
void hspi_TX(uint8_t *data, int count)
{
	int bytes;

	while(count > 0)
	{
		// we will never have zero - we already checked
		bytes = count;
		if(bytes > HSPI_FIFO_SIZE)
			bytes = HSPI_FIFO_SIZE;

        hspi_config(CONFIG_FOR_TX);   // Does hspi_waitReady(); first
		// clear buffer first
        hspi_writeFIFO(data, bytes);
        hspi_startSend();
		hspi_waitReady();

		data += bytes;
		count -= bytes;
	}
}

/// @brief HSPI write and read using FIFO
/// @param[in] *data: transmit / receive buffer
/// @param[in] count: number of bytes to write / read
/// @return  void

void hspi_TXRX(uint8_t *data, int count)
{
	int bytes;

	while(count > 0)
	{
		// we will never have zero - we already checked
		bytes = count;
		if(bytes > HSPI_FIFO_SIZE)
			bytes = HSPI_FIFO_SIZE;

        hspi_config(CONFIG_FOR_RX_TX);   // Does hspi_waitReady(); first
        hspi_writeFIFO(data, bytes);
        hspi_startSend();
		hspi_waitReady();
		hspi_readFIFO(data, bytes);

		data += bytes;
		count -= bytes;
	}
}

/// @brief HSPI read using FIFO
/// @param[in] *data: receive buffer
/// @param[in] count: number of bytes to read
/// @return  void
void hspi_RX(uint8_t *data, int count)
{
	int bytes;

	while(count > 0)
	{
		// we will never have zero - we already checked
		bytes = count;
		if(bytes > HSPI_FIFO_SIZE)
			bytes = HSPI_FIFO_SIZE;

		// discard TX data
        hspi_config(CONFIG_FOR_RX_TX);   
		// Writes are ignored, so we set data to 0xff
        memset(data, 0xff, bytes);
        hspi_writeFIFO(data, bytes);
        hspi_startSend();
		hspi_waitReady();
		// read result
		hspi_readFIFO(data, bytes);

		data += bytes;
		count -= bytes;
	}
}

