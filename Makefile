##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.3.0] date: [Tue Jan 14 00:41:40 EET 2020] 
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

-include .config

######################################
# target
######################################
TARGET = ledstrip


######################################
# building variables
######################################
# optimization
OPT = $(subst ",,$(CONFIG_C_OPTIMIZATION_LEVEL))


#######################################
# paths
#######################################
# Build path
BUILD_DIR = bin

######################################
# source
######################################
SOURCES_TMP = $(wildcard Application/drivers/Common/*.c)
SOURCES_STD_PERIPH = $(wildcard Libraries/STM32F4xx_StdPeriph_Driver/src/*.c)
SOURCES_CMSIS = $(wildcard Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/*.c)
SOURCES_APP = $(wildcard Application/app/*.c)
SOURCES_DRIVERS = $(wildcard Application/drivers/*.c)
SOURCES_USB = $(wildcard USB/STM32_USB_Device_Library/Core/src/*.c)
SOURCES_USB_CLASS = $(wildcard USB/STM32_USB_Device_Library/Class/audio/src/*.c)
#				$(wildcard USB/STM32_USB_Device_Library/Class/cdc/src/*.c)
SOURCES_USB_APP = $(wildcard Application/usb/audio/src/*.c)
#				$(wildcard Application/usb/virtual_com_port/src/*.c)
SOURCES_USB_OTG = $(wildcard USB/STM32_USB_OTG_Driver/src/*.c)
SOURCES_WS_ADAPTER = $(wildcard Application/ws2812_driver/src/adapter/*.c)
SOURCES_WS_DRIVER = $(wildcard Application/ws2812_driver/src/driver/*.c)
SOURCES_WS_SOURCE = $(wildcard Application/ws2812_driver/src/source/*.c)

C_SOURCES = $(SOURCES_TMP) $(SOURCES_STD_PERIPH) $(SOURCES_CMSIS) $(SOURCES_APP) $(SOURCES_DRIVERS) $(SOURCES_USB) $(SOURCES_USB_CLASS) $(SOURCES_USB_OTG) $(SOURCES_USB_APP) $(SOURCES_WS_ADAPTER) $(SOURCES_WS_DRIVER) $(SOURCES_WS_SOURCE)


# ASM sources
ASM_SOURCES =  \
Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f40xx.s


#######################################
# binaries
#######################################
PREFIX = $(subst ",,$(CONFIG_CROSS_COMPILE))
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DMEDIA_IntFLASH \
-DUSB_AUDIO \
-DUSE_DEVICE_MODE \
-DUSE_USB_OTG_FS \
-DNO_I2S_24B

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I. \
-IApplication/drivers/Common \
-IApplication/app \
-IApplication/drivers \
-ILibraries/CMSIS/Include \
-ILibraries/CMSIS/Device/ST/STM32F4xx/Include \
-ILibraries/STM32F4xx_StdPeriph_Driver/inc \
-IUSB/STM32_USB_OTG_Driver/inc \
-IUSB/STM32_USB_Device_Library/Class/audio/inc \
-IUSB/STM32_USB_Device_Library/Core/inc \
-IApplication/usb/audio/inc \
-IApplication/ws2812_driver/inc


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -g -x assembler-with-cpp -specs=nano.specs

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT)

CFLAGS += -std=$(subst ",,$(CONFIG_C_LANGUAGE_STANDARD))
CFLAGS += $(subst ",,$(CONFIG_C_DATA_SECTIONS)) \
          $(subst ",,$(CONFIG_C_FUNCTION_SECTIONS)) \
          $(subst ",,$(CONFIG_C_STACK_USAGE)) \
          $(subst ",,$(CONFIG_C_WARNING_ALL)) \
          $(subst ",,$(CONFIG_C_WARNING_ERROR)) \
          $(subst ",,$(CONFIG_C_WARNING_EXTRA)) \
          $(subst ",,$(CONFIG_C_WARNING_NO_UNUSED_PARAMETER)) \
          $(subst ",,$(CONFIG_C_WARNING_SWITCH_DEFAULT)) \
          $(subst ",,$(CONFIG_C_WARNING_SWITCH_ENUM)) \
		  $(subst ",,$(CONFIG_C_DEBUG_LEVEL)) -gdwarf-2



# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = Libraries/stm32_flash.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = Libraries/CMSIS/Lib/GCC/libarm_cortexM4lf_math.a
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

all:
	@test -f .config.h || $(MAKE) -f Makefile menuconfig
	@$(MAKE) -f Makefile compile

# default action: build all
compile: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir -p $@		

#######################################
# menuconfig
#######################################
menuconfig:
	$(MAKE) -f scripts/Makefile $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# clean configs
#######################################
clean_config:
	$(MAKE) -f scripts/Makefile clean

#######################################
# clean all
#######################################
clean_all: clean clean_config

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
