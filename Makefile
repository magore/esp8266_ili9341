#############################################################
#
# Based on original Makefile (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

# ===============================================================
# Note: project is hard coded for Assumes 4096K SPI flash
# Look for the comment FLASH_SIZE in this Makefile for sections to change
# ===============================================================


# ===============================================================
# Make Verbose messages while compiling
VERBOSE = 0
# ===============================================================
# Project Path definitions
PROJECT_DIR ?=/opt/Espressif/projects/esp8266_ili9341

# ===============================================================
# ESP OPEN SDK path definitions
ROOT_DIR=/opt/Espressif/esp-open-sdk
# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(ROOT_DIR)/xtensa-lx106-elf/bin
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ROOT_DIR)/sdk

# esptool path and port
SDK_TOOLS	?= $(SDK_BASE)/tools
ESPTOOL		?= $(SDK_TOOLS)/esptool.py
ESPPORT		?= /dev/ttyUSB0
# Baud rate for programmer
BAUD=256000
# ===============================================================
# Project Defines


# SOFTCS not yet working
# SPI chip select in software ?
SOFTCS = 
# Display Debug messages via serial
ILI9341_DEBUG = 0 

# TELNET serial bridge
TELNET_SERIAL = 1

# HSPI Prescaler
# A value of 1 works with all except tft_readId() tft_readRegister
HSPI_PRESCALER = 2

# wireframe earth
EARTH = 1

# Spinning Cube
WIRECUBE = 1

# NETWORK Client test
NETWORK_TEST = 1
# The ipaddress of the module - either fixed or by DHCP
IPADDR=192.168.200.116

# SPI FIFO CODE enabled
USE_FIFO =  1

# Netwokr PORT for listener
TCP_PORT = 31415

# Build Directory
BUILD_BASE	= build

# Firmware Directory
FW_BASE		= firmware

# name for the target project
TARGET		= app

# ===============================================================
# Flash memory defines
# Load Address for irom0 data
# See: http://bbs.espressif.com/download/file.php?id=532
ADDRESS=0x40000

# Default SPI speed
# SPI_SPEED = 20MHz, 26.7MHz, 40MHz, 80MHz
SPI_SPEED?=80
# SPI_MODE: QIO, QOUT, DIO, DOUT
SPI_MODE?=QIO
# FLASH SIZE 4096K
SPI_SIZE_MAP=6


# ===============================================================
# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# libraries used in this project, mainly provided by the SDK
LIBS		= cirom gcc hal phy pp net80211 ssc ssl lwip wpa main m

# ===============================================================
# compiler flags using during compilation of source files
CFLAGS	= -Os \
	-g \
	-O2 \
	-Wpointer-arith \
	-Wundef -Werror \
	-Wl,-EL \
    -fno-inline-functions \
	-nostdlib \
	-mlongcalls \
	-mtext-section-literals  \
	-D__ets__ \
	-DICACHE_FLASH \
	-ffunction-sections \
    -fdata-sections

# linker flags used to generate the main object file
LDFLAGS	= -nostdlib \
	-Wl,--no-check-sections \
	-u call_user_start \
	-Wl,-static \
	-Wl,-gc-sections \
	-Wl,-Map=map.txt  \
	-Wl,--undefined=_xtos_set_exception_handler \
	-Wl,--wrap=_xtos_set_exception_handler \
	-Wl,--cref \
	#--check-sections

# ===============================================================


ifeq ($(SPI_SPEED), 26.7)
    freqdiv = 1
	flashimageoptions = -ff 26m
else
    ifeq ($(SPI_SPEED), 20)
        freqdiv = 2
        flashimageoptions = -ff 20m
    else
        ifeq ($(SPI_SPEED), 80)
            freqdiv = 15
			flashimageoptions = -ff 80m
        else
            freqdiv = 0
			flashimageoptions = -ff 40m
        endif
    endif
endif

ifeq ($(SPI_MODE), QOUT)
    mode = 1
	flashimageoptions += -fm qout
else
    ifeq ($(SPI_MODE), DIO)
        mode = 2
		flashimageoptions += -fm dio
    else
        ifeq ($(SPI_MODE), DOUT)
            mode = 3
			flashimageoptions += -fm dout
        else
            mode = 0
			flashimageoptions += -fm qio
        endif
    endif
endif

ifeq ($(SPI_SIZE_MAP), 1)
  size_map = 1
  flash = 256
  flashimageoptions += -fs 2m
else
  ifeq ($(SPI_SIZE_MAP), 2)
    size_map = 2
    flash = 1024
	flashimageoptions += -fs 8m
  else
    ifeq ($(SPI_SIZE_MAP), 3)
      size_map = 3
      flash = 2048
	  flashimageoptions += -fs 16m
    else
      ifeq ($(SPI_SIZE_MAP), 4)
        size_map = 4
        flash = 4096
		flashimageoptions += -fs 32m
      else
        ifeq ($(SPI_SIZE_MAP), 5)
          size_map = 5
          flash = 2048
		  flashimageoptions += -fs 16m
        else
          ifeq ($(SPI_SIZE_MAP), 6)
            size_map = 6
            flash = 4096
			flashimageoptions += -fs 32m
          else
            size_map = 0
            flash = 512
			flashimageoptions += -fs 4m
          endif
        endif
      endif
    endif
  endif
endif
 

# ===============================================================
# which modules (subdirectories) of the project to include in compiling
MODULES	= driver user utils printf cordic display 

# Project Include Directories
EXTRA_INCDIR    = user include $(SDK_BASE)/../include 

# ===============================================================

CFLAGS += -DDEBUG_PRINTF=ets_uart_printf
CFLAGS += -DMAX_USER_RECEIVE_CB=4

ifdef TELNET_SERIAL
	CFLAGS += -DTELNET_SERIAL
	MODULES	+= bridge
endif

ifdef HSPI_PRESCALER
	CFLAGS += -DHSPI_PRESCALER=$(HSPI_PRESCALER)
endif

# Include font specifications - used by proportional fonts 
	CFLAGS  += -DFONTSPECS 

ifdef USE_FIFO
	CFLAGS  += -DUSE_FIFO
endif

ifdef ILI9341_DEBUG
	CFLAGS  += -DILI9341_DEBUG=$(ILI9341_DEBUG)
endif

ifdef SOFTCS
	CFLAGS  += -DSOFTCS
endif

CFLAGS  += -DMIKE_PRINTF

ifdef WIRECUBE
	MODULES	+= wire
	CFLAGS  += -DWIRECUBE
endif

ifdef EARTH
	MODULES	+= wire
	CFLAGS  += -DEARTH
endif

ifdef NETWORK_TEST
	MODULES	+= network
	CFLAGS  += -DNETWORK_TEST -DTCP_PORT=$(TCP_PORT)
endif

# ===============================================================

# linker script used for the above linkier step
#FLASH_ADDRESS
#FIXME computer this
LD_SCRIPT	= eagle.app.v6.new.2048.ld
LD_SCRIPT	:= $(addprefix -T$(PROJECT_DIR)/ld/,$(LD_SCRIPT))

# no user configurable options below here
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ			:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))
# ===============================================================

ifeq ("$(VERBOSE)","1")
	Q := 
	vecho := @true
else
	Q := @
	vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef
# ===============================================================

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) send

$(TARGET_OUT): $(APP_AR)
	$(vecho) "$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@"

	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	$(Q) $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	$(Q) memanalyzer.exe $(OBJDUMP) $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Run objcopy, please wait..."
	$(Q) $(OBJCOPY) --only-section .text -O binary $@ eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $@ eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $@ eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $@ eagle.app.v6.irom0text.bin
	$(vecho) "objcopy done"
	$(vecho) "Run gen_appbin.py"

	$(Q) $(SDK_TOOLS)/gen_appbin.py $@ 0 $(mode) $(freqdiv) $(size_map)
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
	$(Q) rm eagle.app.v6.*
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->$(ADDRESS)"
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@
	$(Q) mkdir -p $@/upgrade

# ===============================================================
flashonefile:   all
	$(vecho) "No boot needed."
	$(ESPTOOL) elf2image $(TARGET_OUT) -ofirmware/ -ff 40m -fm qio -fs 32m
	$(vecho) "Generate eagle.app.flash.bin successully in folder firmware."
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin $(ADDRESS) firmware/$(ADDRESS).bin

flashboot: all flashinit
	$(vecho) "No boot needed."

flash: all
	$(ESPTOOL) -p $(ESPPORT) -b $(BAUD) write_flash $(flashimageoptions) 0x00000 $(FW_BASE)/eagle.flash.bin $(ADDRESS) $(FW_BASE)/eagle.irom0text.bin

# ===============================================================
# From http://bbs.espressif.com/viewtopic.php?f=10&t=305
# master-device-key.bin is only need if using espressive services
# master_device_key.bin 0x3e000 is not used , write blank
# See 2A-ESP8266__IOT_SDK_User_Manual__EN_v1.1.0.pdf
# http://bbs.espressif.com/download/file.php?id=532
#
# 
# System parameter area is the last 16KB of flash
# 512KB flash - system parameter area starts from 0x7C000 
# 	download blank.bin to 0x7E000 as initialization.
# 1024KB flash - system parameter area starts from 0xFC000 
# 	download blank.bin to 0xFE000 as initialization.
# 2048KB flash - system parameter area starts from 0x1FC000 
# 	download blank.bin to 0x1FE000 as initialization.
# 4096KB flash - system parameter area starts from 0x3FC000 
# 	download blank.bin to 0x3FE000 as initialization.
# ===============================================================


# FLASH SIZE
flashinit:
	$(vecho) "Flash init data:"
	$(vecho) "Default config (Clear SDK settings):"
	$(vecho) "blank.bin-------->0x3e000"
	$(vecho) "blank.bin-------->0x3fc000"
	$(vecho) "esp_init_data_default.bin-------->0x3fc000"
	$(ESPTOOL) -p $(ESPPORT) write_flash $(flashimageoptions) \
		0x3e000 $(SDK_BASE)/bin/blank.bin \
		0x3fc000 $(SDK_BASE)/bin/esp_init_data_default.bin \
		0x3fe000 $(SDK_BASE)/bin/blank.bin

rebuild: clean all

# ===============================================================
clean:
	rm -f $(APP_AR)
	rm -f $(TARGET_OUT)
	rm -rf $(BUILD_DIR)
	rm -rf $(BUILD_BASE)
	rm -rf $(FW_BASE)
	rm -f map.txt
	rm -f log
	rm -f eagle.app.*bin
	rm -f send
	#rm -rf doxygen

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))

# ===============================================================
# If makefile changes, update doxygens list
DOCDIRS = $(MODULES) cube wire earth fonts include cordic/make_cordic earth/make_wireframe 

# If makefile changes, maybe the list of sources has changed, so update doxygens list
doxyfile.inc:
	echo "INPUT         =  $(SRCDIRS)" > doxyfile.inc
	echo "FILE_PATTERNS =  *.h *.h" >> doxyfile.inc

.PHONY: doxy
doxy:   doxyfile.inc $(SRCS)
	#export PYTHONPATH=$(PYTHONPATH):/share/embedded/testgen-0.11/extras
	doxygen Doxyfile
# ===============================================================

#Network message sending code

send:	send.c
	gcc send.c -DTCP_PORT=$(TCP_PORT) -o send

sendtest:	send
	./send -i $(IPADDR) -m 'testing\nTest3\nscrolling\ntext and even more text\n1\n3'
