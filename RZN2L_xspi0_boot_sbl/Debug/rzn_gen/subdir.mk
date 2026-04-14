################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rzn_gen/common_data.c \
../rzn_gen/hal_data.c \
../rzn_gen/main.c \
../rzn_gen/pin_data.c \
../rzn_gen/vector_data.c 

LST += \
common_data.lst \
hal_data.lst \
main.lst \
pin_data.lst \
vector_data.lst 

C_DEPS += \
./rzn_gen/common_data.d \
./rzn_gen/hal_data.d \
./rzn_gen/main.d \
./rzn_gen/pin_data.d \
./rzn_gen/vector_data.d 

OBJS += \
./rzn_gen/common_data.o \
./rzn_gen/hal_data.o \
./rzn_gen/main.o \
./rzn_gen/pin_data.o \
./rzn_gen/vector_data.o 

MAP += \
RZN2L_xspi0_boot_sbl.map 


# Each subdirectory must supply rules for building sources it contributes
rzn_gen/%.o: ../rzn_gen/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-r52 -mthumb -mfloat-abi=hard -mfpu=neon-fp-armv8 -fdiagnostics-parseable-fixits -O1 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wnull-dereference -g -D_RENESAS_RZN_ -D_RZN_CORE=CR52_0 -D_RZN_ORDINAL=1 -DSBL_ENABLE=1 -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_gen" -I"." -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\fsp_cfg\\bsp" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\fsp_cfg" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc\\api" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc\\instances" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\src\\bsp\\mcu\\all\\cr" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\arm\\CMSIS_5\\CMSIS\\Core_R\\Include" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\driver" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\common" -std=c99 -Wno-format-truncation -Wno-stringop-overflow --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@arm-none-eabi-gcc @"$@.in"

