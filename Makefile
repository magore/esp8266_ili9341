#############################################################
#
# Root Level Makefile
#
# (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

BUILD_BASE	= build
FW_BASE		= firmware

SOFTCS = 
ILI9341_DEBUG = 

# wireframe earth
EARTH = 
# rotaing Cube
WIRECUBE = 1
# NETWORK Client test
NETWORK_TEST = 1
# SPI FIFO CODE
USE_FIFO =  1

# Netwokr PORT for listener
TCP_PORT = 31415

ROOT_DIR=/opt/Espressif/esp-open-sdk
# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(ROOT_DIR)/xtensa-lx106-elf/bin
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ROOT_DIR)/sdk

# esptool path and port
SDK_TOOLS	?= $(SDK_BASE)/tools
ESPTOOL		?= $(SDK_TOOLS)/esptool.py
ESPPORT		?= /dev/ttyUSB0

# BOOT = none
# BOOT = old - boot_v1.1
# BOOT = new - boot_v1.3+
BOOT?=new
# APP = 0 - eagle.flash.bin + eagle.irom0text.bin
# APP = 1 - user1.bin
# APP = 2 - user2.bin
APP?=0
# SPI_SPEED = 20MHz, 26.7MHz, 40MHz, 80MHz
SPI_SPEED?=40
# SPI_MODE: QIO, QOUT, DIO, DOUT
SPI_MODE?=QIO
# SPI_SIZE: 256KB, 512KB, 1024KB, 2048KB, 4096KB
SPI_SIZE?=4096

ifeq ($(BOOT), new)
    boot = new
else
    ifeq ($(BOOT), old)
        boot = old
    else
        boot = none
    endif
endif

ifeq ($(APP), 1)
    app = 1
else
    ifeq ($(APP), 2)
        app = 2
    else
        app = 0
    endif
endif

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

# flash larger than 1024KB only use 1024KB to storage user1.bin and user2.bin
ifeq ($(SPI_SIZE), 256)
    size = 1
    flash = 256
	flashimageoptions += -fs 2m
else
    ifeq ($(SPI_SIZE), 1024)
        size = 2
        flash = 1024
		flashimageoptions += -fs 8m
    else
        ifeq ($(SPI_SIZE), 2048)
            size = 3
            flash = 1024
			flashimageoptions += -fs 16m
        else
            ifeq ($(SPI_SIZE), 4096)
                size = 4
                flash = 1024
				flashimageoptions += -fs 32m
            else
                size = 0
                flash = 512
				flashimageoptions += -fs 4m
            endif
        endif
    endif
endif

ifeq ($(flash), 512)
  ifeq ($(app), 1)
    addr = 0x01000
  else
    ifeq ($(app), 2)
      addr = 0x41000
    endif
  endif
else
  ifeq ($(flash), 1024)
    ifeq ($(app), 1)
      addr = 0x01000
    else
      ifeq ($(app), 2)
        addr = 0x81000
      endif
    endif
  endif
endif

# name for the target project
TARGET		= app

EXTRA_INCDIR    = user include $(SDK_BASE)/../include 

# libraries used in this project, mainly provided by the SDK
#LIBS		= gcc hal phy pp net80211 lwip wpa main m
LIBS		= cirom gcc hal phy pp net80211 ssc ssl lwip wpa main m


# compiler flags using during compilation of source files
CFLAGS	= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL \
    -fno-inline-functions \
	-nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH \
	-ffunction-sections 

MODULES	= driver user printf cordic display
# which modules (subdirectories) of the project to include in compiling

# Include font specifications - used by proportional fonts 
	CFLAGS  += -DFONTSPECS 

ifdef USE_FIFO
	CFLAGS  += -DUSE_FIFO
endif

ifdef ILI9341_DEBUG
	CFLAGS  += -DILI9341_DEBUG
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

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static \
-Wl,-gc-sections -Wl,-Map=map.txt  \
-Wl,--undefined=_xtos_set_exception_handler -Wl,--wrap=_xtos_set_exception_handler

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

ifneq ($(boot), none)
ifneq ($(app),0)
	LD_SCRIPT = eagle.app.v6.$(boot).$(flash).app$(app).ld
	BIN_NAME = user$(app).$(flash).$(boot)
endif
else
    app = 0
endif

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

# no user configurable options below here
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

#/opt/Espressif/esp-open-sdk/ESP8266_SDK/ld/eagle.app.v6.ld
#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
LD_SCRIPT	:= $(addprefix -T/opt/Espressif/projects/mike/ld/,$(LD_SCRIPT))
#LD_SCRIPT   := $(addprefix -T/opt/Espressif/CHERTS/esp8266-devkit/Espressif/examples/esp8266_ili9341/ld/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

V ?= $(VERBOSE)
ifeq ("$(V)","1")
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

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) send

$(TARGET_OUT): $(APP_AR)
	$(vecho) "$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@"

	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	echo $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $@
	$(Q) $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	echo memanalyzer.exe $(OBJDUMP) $@
	$(Q) memanalyzer.exe $(OBJDUMP) $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Run objcopy, please wait..."
	$(Q) $(OBJCOPY) --only-section .text -O binary $@ eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $@ eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $@ eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $@ eagle.app.v6.irom0text.bin
	$(vecho) "objcopy done"
	$(vecho) "Run gen_appbin.py"
ifeq ($(app), 0)
	$(Q) $(SDK_TOOLS)/gen_appbin.py $@ 0 $(mode) $(freqdiv) $(size)
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
	$(Q) rm eagle.app.v6.*
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->0x40000"
else
    ifeq ($(boot), new)
		# $(Q) $(SDK_TOOLS)/gen_appbin.exe $@ 2 $(mode) $(freqdiv) $(size)
		$(Q) $(SDK_TOOLS)/gen_appbin.py $@ 2 $(mode) $(freqdiv) $(size)
		$(vecho) "Support boot_v1.3 and +"
    else
		#$(Q) $(SDK_TOOLS)/gen_appbin.exe $@ 1 $(mode) $(freqdiv) $(size)
		$(Q) $(SDK_TOOLS)/gen_appbin.py $@ 1 $(mode) $(freqdiv) $(size)
		$(vecho) "Support boot_v1.1 and +"
    endif
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/upgrade/$(BIN_NAME).bin
	$(Q) rm eagle.app.v6.*
	$(vecho) "Generate $(BIN_NAME).bin successully in folder $(FW_BASE)/upgrade."
	$(vecho) "boot_v1.x.bin------->0x00000"
	$(vecho) "$(BIN_NAME).bin--->$(addr)"
endif
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

flashonefile:   all
	$(vecho) "No boot needed."
	$(ESPTOOL) elf2image $(TARGET_OUT) -ofirmware/ -ff 40m -fm qio -fs 32m
	$(vecho) "Generate eagle.app.flash.bin successully in folder firmware."
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin

flashboot: all flashinit
ifeq ($(boot), new)
	$(vecho) "Flash boot_v1.3 and +"
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash $(flashimageoptions) 0x00000 $(SDK_BASE)/bin/boot_v1.3\(b3\).bin
endif
ifeq ($(boot), old)
	$(vecho) "Flash boot_v1.1 and +"
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash $(flashimageoptions) 0x00000 $(SDK_BASE)/bin/boot_v1.1.bin
endif
ifeq ($(boot), none)
	$(vecho) "No boot needed."
endif

flash: all
ifeq ($(app), 0) 
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash $(flashimageoptions) 0x00000 $(FW_BASE)/eagle.flash.bin 0x40000 $(FW_BASE)/eagle.irom0text.bin
else
ifeq ($(boot), none)
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash $(flashimageoptions) 0x00000 $(FW_BASE)/eagle.flash.bin 0x40000 $(FW_BASE)/eagle.irom0text.bin
else
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash $(flashimageoptions) $(addr) $(FW_BASE)/upgrade/$(BIN_NAME).bin
endif
endif

flashinit:
	$(vecho) "Flash init data:"
	$(vecho) "Clear old settings (EEP area):"
	$(vecho) "clear_eep.bin-------->0x79000"
	$(vecho) "Default config (Clear SDK settings):"
	$(vecho) "blank.bin-------->0x7E000"
	$(vecho) "esp_init_data_default.bin-------->0x7C000"
	$(ESPTOOL) -p $(ESPPORT) write_flash $(flashimageoptions) \
		0x79000 $(SDK_BASE)/bin/clear_eep.bin \
		0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin \
		0x7e000 $(SDK_BASE)/bin/blank.bin

rebuild: clean all

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

# If makefile changes, maybe the list of sources has changed, so update doxygens list
DOCDIRS = $(MODULES) cube wire earth fonts include cordic/make_cordic earth/make_wireframe 

# If makefile changes, maybe the list of sources has changed, so update doxygens list
doxyfile.inc:
	echo "INPUT         =  $(SRCDIRS)" > doxyfile.inc
	echo "FILE_PATTERNS =  *.h *.h" >> doxyfile.inc

.PHONY: doxy
doxy:   doxyfile.inc $(SRCS)
	#export PYTHONPATH=$(PYTHONPATH):/share/embedded/testgen-0.11/extras
	doxygen Doxyfile

#Network message sending code

send:	send.c
	gcc send.c -DTCP_PORT=$(TCP_PORT) -o send
