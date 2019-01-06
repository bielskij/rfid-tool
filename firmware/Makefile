CROSS_COMPILE=avr-

CC      := $(CROSS_COMPILE)gcc
AS      := $(CROSS_COMPILE)gcc -x assembler-with-cpp
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE    := $(CROSS_COMPILE)size

ifneq ($(V),1)
CC      := @$(CC)
AS      := @$(AS)
OBJCOPY := @$(OBJCOPY)
OBJDUMP := @$(OBJDUMP)
SIZE    := @$(SIZE)
endif

MKDIR := @mkdir
RM    := @rm

DIR_OUT := out
DIR_INC := inc
DIR_SRC := src

F_CPU := 16500000
MCU   := attiny85

CFLAGS := -std=c99 
CFLAGS += -Wall -Wundef -Warray-bounds -Wformat -Wmissing-braces -Wreturn-type
CFLAGS += -fshort-enums -fpack-struct  -fno-split-wide-types -ffunction-sections -fdata-sections
CFLAGS += -mmcu=$(MCU) -DF_CPU=$(F_CPU) -mrelax -O2
# VUSB config
CFLAGS += -D__CODEVISIONAVR__=0 -DDEBUG_LEVEL=0 -DUSB_CFG_DRIVER_FLASH_PAGE=0
CFLAGS +=  -DUSB_CFG_DEVICE_NAME_LEN=0 -DUSB_CFG_SERIAL_NUMBER_LEN=0 -DUSB_CFG_USE_SWITCH_STATEMENT=0
CFLAGS += -I$(DIR_SRC) -I$(DIR_SRC)/usbdrv -I$(DIR_SRC)/../../common/inc

LDFLAGS += -Wl,--gc-sections

APP_NAME := firmware

# common src
SRC := \
	$(DIR_SRC)/usbdrv/usbdrvasm.S \
	$(DIR_SRC)/usbdrv/oddebug.c \
	$(DIR_SRC)/main.c
	
TMP := $(foreach file, $(SRC), $(shell echo $(file) | sed -e 's|$(DIR_SRC)\/|$(DIR_OUT)\/|'))
OBJ := $(foreach file, $(TMP), $(shell echo $(file) | sed -e 's|\.c$$|.c.o|' | sed -e 's|\.S$$|.S.o|')) 

init:
	$(MKDIR) -p $(DIR_OUT)

all: init \
	$(DIR_OUT)/$(APP_NAME).elf \
	$(DIR_OUT)/$(APP_NAME).lss \
	$(DIR_OUT)/$(APP_NAME).hex \
	$(DIR_OUT)/$(APP_NAME).eep \
	$(DIR_OUT)/$(APP_NAME).bin

	$(SIZE) --format=avr $(DIR_OUT)/$(APP_NAME).elf

burn:
	micronucleus --timeout 10 --run --type intel-hex $(DIR_OUT)/$(APP_NAME).hex

clean:
	$(RM) -rf $(DIR_OUT)

$(DIR_OUT)/%.c.o: $(DIR_SRC)/%.c
	@echo "Building `basename $@`"
	$(MKDIR) -p `dirname $@`
	$(CC) -c $(CFLAGS) -o $@ $<

$(DIR_OUT)/%.S.o: $(DIR_SRC)/%.S
	@echo "Building `basename $@`"
	$(MKDIR) -p `dirname $@`
	$(AS) -c $(CFLAGS) -o $@  $< 

$(DIR_OUT)/%.elf: $(OBJ)
	@echo "Building binary... $(APP_NAME).elf"
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)
	
$(DIR_OUT)/%.lss: $(DIR_OUT)/%.elf
	@echo "Creating Extended Listing... $@"
	$(OBJDUMP) -h -S $< > $@
	
$(DIR_OUT)/%.hex: $(DIR_OUT)/%.elf
	@echo "Create Flash image (ihex format)... $@"
	$(OBJCOPY) -R .eeprom -O ihex $< $@

$(DIR_OUT)/%.bin: $(DIR_OUT)/%.elf
	@echo "Create Flash image (binary format)... $@"
	$(OBJCOPY) $(OBJCOPY_BIN_FLAGS) -R .eeprom -O binary $< $@
	
$(DIR_OUT)/%.eep: $(DIR_OUT)/%.elf
	@echo "Create eeprom image (ihex format)... $@"
	$(OBJCOPY) -j .eeprom -O ihex $< $@