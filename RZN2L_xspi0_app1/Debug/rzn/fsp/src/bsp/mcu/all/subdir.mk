################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rzn/fsp/src/bsp/mcu/all/bsp_cache.c \
../rzn/fsp/src/bsp/mcu/all/bsp_clocks.c \
../rzn/fsp/src/bsp/mcu/all/bsp_common.c \
../rzn/fsp/src/bsp/mcu/all/bsp_delay.c \
../rzn/fsp/src/bsp/mcu/all/bsp_io.c \
../rzn/fsp/src/bsp/mcu/all/bsp_irq.c \
../rzn/fsp/src/bsp/mcu/all/bsp_register_protection.c \
../rzn/fsp/src/bsp/mcu/all/bsp_reset.c \
../rzn/fsp/src/bsp/mcu/all/bsp_sbrk.c 

LST += \
bsp_cache.lst \
bsp_clocks.lst \
bsp_common.lst \
bsp_delay.lst \
bsp_io.lst \
bsp_irq.lst \
bsp_register_protection.lst \
bsp_reset.lst \
bsp_sbrk.lst 

C_DEPS += \
./rzn/fsp/src/bsp/mcu/all/bsp_cache.d \
./rzn/fsp/src/bsp/mcu/all/bsp_clocks.d \
./rzn/fsp/src/bsp/mcu/all/bsp_common.d \
./rzn/fsp/src/bsp/mcu/all/bsp_delay.d \
./rzn/fsp/src/bsp/mcu/all/bsp_io.d \
./rzn/fsp/src/bsp/mcu/all/bsp_irq.d \
./rzn/fsp/src/bsp/mcu/all/bsp_register_protection.d \
./rzn/fsp/src/bsp/mcu/all/bsp_reset.d \
./rzn/fsp/src/bsp/mcu/all/bsp_sbrk.d 

OBJS += \
./rzn/fsp/src/bsp/mcu/all/bsp_cache.o \
./rzn/fsp/src/bsp/mcu/all/bsp_clocks.o \
./rzn/fsp/src/bsp/mcu/all/bsp_common.o \
./rzn/fsp/src/bsp/mcu/all/bsp_delay.o \
./rzn/fsp/src/bsp/mcu/all/bsp_io.o \
./rzn/fsp/src/bsp/mcu/all/bsp_irq.o \
./rzn/fsp/src/bsp/mcu/all/bsp_register_protection.o \
./rzn/fsp/src/bsp/mcu/all/bsp_reset.o \
./rzn/fsp/src/bsp/mcu/all/bsp_sbrk.o 

MAP += \
RZN2L_xspi0_app1.map 


# Each subdirectory must supply rules for building sources it contributes
rzn/fsp/src/bsp/mcu/all/%.o: ../rzn/fsp/src/bsp/mcu/all/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-r52 -mthumb -mfloat-abi=hard -mfpu=neon-fp-armv8 -fdiagnostics-parseable-fixits -O1 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wnull-dereference -g -D_RENESAS_RZN_ -D_RZN_CORE=CR52_0 -D_RZN_ORDINAL=1 -DREVISION_NUMBER=0x00000300 -DETHERCAT_SSC_PORT_GMAC_MDIO_SUPPORT=1 -DxFW_PARSE_SREC -DAPP1_BANK=0 -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_gen" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\renesas" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\beckhoff" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\beckhoff\\Src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat" -I"." -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\fsp_cfg\\bsp" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\fsp_cfg" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc\\api" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc\\instances" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\src\\bsp\\mcu\\all\\cr" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\arm\\CMSIS_5\\CMSIS\\Core_R\\Include" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\driver" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\src\\rm_ethercat_ssc_port" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\common" -std=c99 -Wno-format-truncation -Wno-stringop-overflow --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@arm-none-eabi-gcc @"$@.in"

