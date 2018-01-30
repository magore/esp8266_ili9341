#############################################################
#
# Based on original Makefile (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

# ===============================================================
# Note: project is hard coded for Assumes 512K SPI flash
# ===============================================================


# ===============================================================
# Make Verbose messages while compiling
VERBOSE = 1
# ===============================================================
# Project Path definitions
PROJECT_DIR =/opt/Espressif/projects/esp8266_ili9341

# ===============================================================
# ESP OPEN SDK path definitions
ROOT_DIR    = /opt/Espressif/esp-open-sdk
# Base directory for the compiler
XTENSA_TOOLS_ROOT = ${ROOT_DIR}/xtensa-lx106-elf/bin
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	= ${ROOT_DIR}/sdk
SDK_TOOLS	= ${SDK_BASE}/tools
DEF_INIT     = ${SDK_BASE}/bin/esp_init_data_default.bin
BLANK_INIT  = ${SDK_BASE}/bin/blank.bin

#ESPTOOL		= esptool-ck/esptool
#ESPTOOL		= ${SDK_TOOLS}/esptool.py
ESPTOOL		= esptool/esptool.py
ESPPORT		= /dev/ttyUSB0

# Export path
export PATH := ${XTENSA_TOOLS_ROOT}:$(PATH)

# esptool baud rate 
#BAUD=115200
#BAUD=256000
BAUD=921600

# CPU frequency
F_CPU=80000000UL

# ===============================================================

# ===============================================================
# Build Directory
BUILD_BASE	= build
# name for the target project
TARGET		= demo
# Firmware Directory
FW_BASE		= firmware


# ===============================================================
# The settings in this section are related to the flash size of the ESP board
# esptool.py flash arguments for 512K SPI flash
# WARNING ADDR_IROM MUST match settings in LD_SCRIPT!


#ESP12=1
# SWAP GPIO4 and GPIO5 on some esp-12 boards have labels reversed
# My board is reversed
ifdef ESP12
	SWAP45=1
	FW_ARGS := -ff 80m -fm qio 
	LD_SCRIPT		= eagle.app.v6.new.2048.ld
	# The ipaddress of the module - either fixed or by DHCP
	IPADDR=192.168.200.110
else
	# Default 512K boards
	FW_ARGS := -ff 80m -fm qio 
	IPADDR=192.168.200.110
	LD_SCRIPT		= eagle.app.v6.new.512.ld
endif

ADDR_IRAM		= 0x00000
ADDR_IROM		= 0x10000
FILE_IRAM		:= ${BUILD_BASE}/region-$(ADDR_IRAM).bin
FILE_IRAM_PAD	:= ${BUILD_BASE}/region-$(ADDR_IRAM)-padded.bin
FILE_IROM		:= ${BUILD_BASE}/region-$(ADDR_IROM).bin
FW				:= ${BUILD_BASE}/firmware.bin

# ===============================================================
# which modules (subdirectories) of the project to include in compiling
MODULES	= esp8266 lib 3rd_party display cordic network user

# Project Include Directories
EXTRA_INCDIR    = . user include ${SDK_BASE}/include 

# ===============================================================

# Base compiler flags using during compilation of source files
CFLAGS	= -Os \
	-g \
	-O2 \
	-Wpointer-arith \
	-Wundef \
	-Wl,-EL \
    -fno-inline-functions \
	-nostdlib \
	-mlongcalls \
	-mtext-section-literals  \
	-D__ets__ \
	-DICACHE_FLASH \
	-ffunction-sections \
    -fdata-sections \
	-DF_CPU=$(F_CPU)
	# -Werror \

# linker flags used to generate the main object file
LDFLAGS	= -nostdlib \
	-Wl,--no-check-sections \
	-u call_user_start \
	-Wl,-static \
	-Wl,-gc-sections \
	-Wl,-Map=linkmap \
	-Wl,--cref \
	-Wl,--allow-multiple-definition 

# for cal_dex - exception debugging
LDFLAGS += -Wl,--undefined=_xtos_set_exception_handler -Wl,--wrap=_xtos_set_exception_handler
# ===============================================================
# Project Defines

# ESP8266 specific support
CFLAGS += -DESP8266

# Named address space aliases
#CFLAGS += -DMEMSPACE=ICACHE_FLASH_ATTR
#CFLAGS += -DMEMSPACE_RO=ICACHE_RODATA_ATTR
CFLAGS += -D_GNU_SOURCE

# files should include user_config.h
CFLAGS += -DUSER_CONFIG

# =========================



# =========================
# Run a web server
WEBSERVER = 1

# Web server Debugging
# 0 no WEB debugging
# 1 error only
# 2 connection information
# 4 send/yield task information
# 8 HTML processing
# 16 characters from socket I/O
# 32 file processing
#WEB_DEBUG = 1+8
WEB_DEBUG = 1

# Maximum number of WEB connections
MAX_CONNECTIONS = 8

# =========================
# Matrix Debugging
# 0 no debugging
# 1 errors
# 2 details
MATDEBUG = 1
	CFLAGS += -DMATDEBUG=$(MATDEBUG)
# =========================
# printf, sscanf and math IO functions

# Debugging printf function
DEBUG_PRINTF=uart0_printf

SSCANF=1
PRINTF=1
# Floating point support for IO functions
FLOATIO=1

ifdef PRINTF
	#CFLAGS += -DDEFINE_PRINTF
	CFLAGS += -Dprintf=$(DEBUG_PRINTF) 
	MODULES	+= printf
endif

ifdef SSCANF
	MODULES	+= io
    CFLAGS += -DSMALL_SSCANF
endif

ifdef FLOATIO
CFLAGS += -DFLOATIO
endif

# =========================
# POIX tests
POSIX_TESTS = 1

# FatFS tests
FATFS_SUPPORT = 1
FATFS_TESTS = 1
FATFS_DEBUG=1
#FATFS_UTILS_FULL=1

ifdef FATFS_SUPPORT
    CFLAGS  += -DFATFS_SUPPORT

ifdef FATFS_TESTS
    CFLAGS  += -DFATFS_TESTS
endif

ifdef FATFS_UTILS_FULL
    CFLAGS += -DFATFS_UTILS_FULL
endif
ifdef FATFS_DEBUG
    CFLAGS  += -DFATFS_DEBUG=$(FATFS_DEBUG)
endif

ifdef POSIX_TESTS
    CFLAGS += -DPOSIX_TESTS
	MODULES	+= posix
endif

ifdef SWAP45
	MMC_CS=5
else
	MMC_CS=4
endif
    CFLAGS  += -DMMC_CS=$(MMC_CS)

	MODULES	+= fatfs.sup
	MODULES	+= fatfs
	MODULES	+= fatfs/option
	MODULES	+= fatfs.hal
endif

# =========================
# DS1307 I2C real time clock
# RTC_SUPPORT = 1

# =========================
# ADF4351 demo
#ADF4351 = 1

# =========================
# XPT2046 demo
XPT2046 = 1

# 0 no debugging
# 1 error only
# 2 information 
# 4 mapped results
XPT2046_DEBUG = 5

# =========================
# Yield function support thanks to Arduino Project 
# You should always leave this on
YIELD_TASK = 1

# =========================
# =========================
DISPLAY = 1
ifdef DISPLAY
    CFLAGS  += -DDISPLAY
# ILI9341 Display support
	ILI9341_CS = 15
    CFLAGS  += -DILI9341_CS=$(ILI9341_CS)
ifdef SWAP45
	ADDR_0 = 4
else
	ADDR_0 = 5
endif
    CFLAGS  += -DADDR_0=$(ADDR_0)

# Display Debug messages via serial
ILI9341_DEBUG = 0 

# Display wireframe earth demo in lower right of display
EARTH = 1

# Spinning Cube demo in upper right of display
WIRECUBE = 1

# Circle demo
CIRCLE = 

# Display voltage - only works if DEBUG_STATS = 1
VOLTAGE_TEST = 1

# Display additional status:
# 	interation count for spinning cube
#   heap and connection count
#   connection and wifi status
#   voltage
DEBUG_STATS = 1

# Vector fonts
# VFONTS = 1
ifdef VFONTS
	CFLAGS += -DVFONTS
endif

# TFT display DEBUG level
ifdef ILI9341_DEBUG
	CFLAGS  += -DILI9341_DEBUG=$(ILI9341_DEBUG)
endif

# ILI9341 Display and FONTS
# Include font specifications - needed with proportional fonts 
	CFLAGS  += -DFONTSPECS 

ifdef DEBUG_STATS
	CFLAGS += -DDEBUG_STATS
endif

ifdef VOLTAGE_TEST
	CFLAGS += -DVOLTAGE_TEST
endif

ifdef XPT2046
	CFLAGS += -DXPT2046
    CFLAGS += -DXPT2046_CS=2
    CFLAGS += -DXPT2046_DEBUG=$(XPT2046_DEBUG)
	MODULES	+= xpt2046
endif

ifdef WIRECUBE
	MODULES	+= wire
	CFLAGS  += -DWIRECUBE
endif

ifdef CIRCLE
	CFLAGS  += -DCIRCLE
endif

ifdef EARTH
ifndef WIRECUBE
	MODULES	+= wire
endif
	CFLAGS  += -DEARTH
endif


endif  # ifdef DISPLAY
# =========================
# =========================

# TELNET serial bridge demo
//TELNET_SERIAL = 1

# =========================
# NETWORK Client demo
NETWORK_TEST = 1
# Network PORT for server and client
# Displays data on TFT display
TCP_PORT = 31415

# =========================
ifdef TELNET_SERIAL
	CFLAGS += -DTELNET_SERIAL
	MODULES	+= bridge
endif

# =========================
ifdef NETWORK_TEST
	MODULES += server
	CFLAGS  += -DNETWORK_TEST -DTCP_PORT=$(TCP_PORT)
endif

# =========================
ifdef WEBSERVER
	CFLAGS += -DWEBSERVER -DWEB_DEBUG=$(WEB_DEBUG) -DMAX_CONNECTIONS=$(MAX_CONNECTIONS)
	MODULES	+= web
endif



# =========================
ifdef YIELD_TASK
	CFLAGS += -DYIELD_TASK
	MODULES	+= yield
endif

# =========================
ifdef ADF4351
	CFLAGS += -DADF4351

	ADF4351_CS=0
	CFLAGS += -DADF4351_CS=$(ADF4351_CS)

	ADF4351_DEBUG = 1
# Debug options can be combined by adding or oring
# 1 = errors
# 2 = calculation detail
# 4 = register dumps
# Example for everything
#  ADF4351_DEBUG = 1+2+4

	MODULES	+= adf4351
ifdef ADF4351_DEBUG
	CFLAGS += -DADF4351_DEBUG=$(ADF4351_DEBUG)
endif
endif

# UART queues
	CFLAGS += -DUART_QUEUED 
	CFLAGS += -DUART_QUEUED_RX
#	CFLAGS += -DUART_QUEUED_TX



# ===============================================================
# select which tools to use as compiler, librarian and linker
CC		:= ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-gcc
NM		:= ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-nm
AR		:= ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-ar
LD		:= ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-gcc
OBJCOPY := ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-objcopy
OBJDUMP := ${XTENSA_TOOLS_ROOT}/xtensa-lx106-elf-objdump

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# V 2 libs
# libcrypto.a
# libssl.a
# libat.a
# libnet80211.a
# libupgrade.a
# libphy.a
# libwpa.a
# libmain.a
# libmesh.a
# libdriver.a
# libespnow.a
# libpp.a
# libairkiss.a
# libjson.a
# libsmartconfig.a
# liblwip_536.a
# libpwm.a
# libgcc.a
# liblwip.a
# libwps.a
# libwpa2.a

LIBS		= gcc hal phy pp net80211 ssl lwip wpa main m

# ===============================================================


compiler.S.cmd=xtensa-lx106-elf-gcc
compiler.S.flags=-c -g -x assembler-with-cpp -MMD 


# ===============================================================

LD_SCRIPT	:= $(addprefix -T$(PROJECT_DIR)/ld/,$(LD_SCRIPT))

# no user configurable options below here
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix ${BUILD_BASE}/,$(MODULES))

SDK_LIBDIR	:= $(addprefix ${SDK_BASE}/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I${SDK_BASE}/,$(SDK_INCDIR))

SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.[cS])) 
C_OBJ		:= $(patsubst %.c,%.o,$(SRC))
S_OBJ		:= $(patsubst %.S,%.o,$(C_OBJ))
OBJ		    := $(patsubst %.o,${BUILD_BASE}/%.o,$(S_OBJ))
LIBS		:= $(addprefix -l,$(LIBS))

APP_AR		:= $(addprefix ${BUILD_BASE}/,$(TARGET).a)
ELF			:= $(addprefix ${BUILD_BASE}/,$(TARGET).elf)


CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DLOCAL_MOD="\"$(LOCAL_MOD)\""

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))
# ===============================================================
FILES := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.[cS])) 
FILES += $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.md)) 
FILES += $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.txt)) 
# Use GIT last modify time if we have it
# GIT_VERSION := $(shell git log -1 2>&1 | grep "^Date:")
# update.last is safer to use, the file is touched by my git commit script
GIT_VERSION := $(shell stat -c%x update.last 2>/dev/null)
LOCAL_MOD := $(shell ls -rt $(FILES) | tail -1 | xargs stat -c%x)
# ===============================================================

ifeq ("$(VERBOSE)","1")
	Q := 
	vecho := @true
else
	Q := @
	vecho := @echo
endif

vpath %.c $(SRC_DIR)
vpath %.S $(SRC_DIR)

define compile-objects
$1/%.o: %.S
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c -g -x assembler-with-cpp -MMD $$< -o $$@
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

# ===============================================================
.PHONY: all checkdirs clean

all: esptool support checkdirs $(FW) send status

.PHONY: status
status:	esptool flash-size
	@echo =============================================
	@echo Note:
	@if [ -n "$(SWAP45)" ]; then echo "GPIO pins 4 and 5 are swapped"; fi
	@echo ADDR_0 pin is GPIO $(ADDR_0)
	@echo
	@echo =============================================
	@echo

# clean
.PHONY: esptool-clean
esptool-clean:
	@echo removing esptool
	rm -rf esptool

#Update esptool
.PHONY: esptool
esptool:	esptool-fetch esptool-patch



.PHONY:	esptool-fetch
esptool-fetch:
	@if [ ! -d esptool ]; then git clone https://github.com/espressif/esptool; else echo esptool already downloaded; fi

.PHONY:	esptool-patch
esptool-patch:	esptool-fetch
	@if ! grep "get_flash_size" esptool/esptool.py >/dev/null 2>&1 ;then echo Patching esptool.py adding get_flash_size option; patch -p0 <esptool.patch.txt; else echo file already patched; fi

# Delete
.PHONY: esptool-update
esptool-update:	esptool-clean esptool-fetch

.PHONY: esptool-ck
esptool-ck:
	#if [ ! -d esptool-ck ]; then git clone https://github.com/igrr/esptool-ck; cd esptool-ck; make; cd ..; else cd esptool-ck; git pull; make; cd .. fi
	@echo =============================================

.PHONY: support
support:
	-@$(MAKE) -C cordic/make_cordic all
	-@$(MAKE) -C earth all
	-@$(MAKE) -C fonts all
ifdef VFONTS
	-@$(MAKE) -C vfonts all
endif

checkdirs: ${BUILD_DIR} $(FW_BASE)


${APP_AR}: $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

$(ELF):	${APP_AR}
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) ${APP_AR} -Wl,--end-group -o $@

size:	$(ELF)
	$(vecho) "Section info:"
	-$(Q) memanalyzer.exe $(OBJDUMP) $(ELF)
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_text_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_text_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_irom0_text_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_irom0_text_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_rodata_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_rodata_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_data_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_data_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_bss_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_bss_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_heap_start"
	@#-@$(NM) -n -S $(ELF) 2>&1 | grep "_heap_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_rodata_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_rodata_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_literal_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_literal_end"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_data_start"
	-@$(NM) -n -S $(ELF) 2>&1 | grep "_dport0_data_end"

$(FW):	$(ELF) size
	$(vecho) "Firmware $@"
	$(ESPTOOL) elf2image $(ELF) -o ${BUILD_BASE}/region-
	$(Q) dd if=$(FILE_IRAM) of=$(FILE_IRAM_PAD) ibs=64K conv=sync 2>&1 >/dev/null
	$(Q) cat $(FILE_IRAM_PAD) $(FILE_IROM) > $(FW)

# =================================================================================================
#  4.1.2. Download Addresses
#  
#  Table 4-2 lists the download addresses for Non-FOTA firmware.
#  
#  Table 4-2. Download Addresses for Non-FOTA Firmware (unit: kB)
#  Binaries 	Download addresses in flash of different capacities
#                                 512                1024                 2048                 4096
#  master_device_key.bin      0x3E000             0x3E000             0x3E0000             0x3E0000
#  esp_init_data_default.bin  0x7C000             0xFC000             0x1FC000             0x3FC000
#  blank.bin                  0x7E000             0xFE000             0x1FE000             0x3FE000
#  eagle.flash.bin            0x00000             0x00000              0x00000              0x00000
#  eagle.irom0text.bin        0x10000             0x10000              0x10000              0x10000
# =================================================================================================

flash: all
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash $(FW_ARGS)  0 $(FW)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash $(FW_ARGS) 0 $(FW)
	#miniterm.py --parity N -e --rts 0 --dtr 0 /dev/ttyUSB0 115200
	miniterm.py --parity N -e --rts 0 --dtr 0 /dev/ttyUSB0 74480

flash-size:	esptool
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) get_flash_size | grep -i "Flash size"

verify: flash
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash $(FW_ARGS) 0 $(FW)

testinit: 
	@echo Disabled untill the correct recovery steps from a blank fash can be determined
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7c000 $(DEF_INIT)
	#$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7e000 $(BLANK_INIT)

init: 
	@echo Disabled untill the correct recovery steps from a blank fash can be determined
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7c000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7d000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7e000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7f000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) write_flash 0x7c000 $(DEF_INIT)

verify-init: init
	@echo Disabled untill the correct recovery steps from a blank fash can be determined
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash 0x7c000 $(DEF_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash 0x7d000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash 0x7e000 $(BLANK_INIT)
	$(ESPTOOL) --port $(ESPPORT)  -b $(BAUD) verify_flash 0x7f000 $(BLANK_INIT)

erase: checkdirs
	@echo Disabled untill the correct recovery steps from a blank fash can be determined
	$(ESPTOOL) --port $(ESPPORT) -b $(BAUD) erase_flash

.PHONY: testflash
testflash:
	gcc testflash.c -o testflash
	-mkdir tmp
	@echo testing first megabyte
	@echo
	@echo Create megabyte size test file 
	./testflash -s 0x080000 -w tmp/test1w.bin
	@echo Write file to ESP8266
	$(ESPTOOL) -p $(ESPPORT) -b $(BAUD) write_flash \
		0x000000 tmp/test1w.bin 
	@echo read flash back from ESP8266
	-$(ESPTOOL) -p $(ESPPORT) -b $(BAUD) read_flash \
		0x000000 0x080000 tmp/test1r.bin 
	@echo Verify data read back matches what we wrote
	./testflash -s 0x080000 -r tmp/test1r.bin

rebuild: clean all

${BUILD_DIR}:
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@
	$(Q) mkdir -p $@/upgrade

# ===============================================================
clean:
	-@$(MAKE) -C printf clean
	-@$(MAKE) -C cordic/make_cordic clean
	-@$(MAKE) -C earth clean
	-@$(MAKE) -C fonts clean
	-@$(MAKE) -C vfonts clean
	rm -f ${APP_AR}
	rm -rf ${BUILD_DIR}
	rm -rf ${BUILD_BASE}
	rm -rf $(FW_BASE)
	rm -f linkmap
	rm -f log
	rm -f eagle.app.*bin
	rm -f send
	rm -f map.txt
	rm -f testflash
	rm -rf doxygen/*

$(foreach bdir,${BUILD_DIR},$(eval $(call compile-objects,$(bdir))))

# ===============================================================
# If makefile changes, update doxygens list
DOCDIRS := . $(MODULES) wire earth fonts vfonts include cordic/make_cordic

# If makefile changes, maybe the list of sources has changed, so update doxygens list
.PHONY: doxyfile.inc
doxyfile.inc:
	echo "INPUT         =  $(DOCDIRS)" > doxyfile.inc
	echo "FILE_PATTERNS =  *.h *.c *.md" >> doxyfile.inc

.PHONY: doxy
doxy:   doxyfile.inc 
	#export PYTHONPATH=$(PYTHONPATH):/share/embedded/testgen-0.11/extras
	doxygen Doxyfile
# ===============================================================

#Network message sending code

send:	send.c
	gcc send.c -DTCP_PORT=$(TCP_PORT) -o send

sendtest:	send
	./send -i $(IPADDR) -m 'testing\nTest3\nscrolling\ntext and even more text\n1\n3'

gcchelp:
	$(CC) --target-help
