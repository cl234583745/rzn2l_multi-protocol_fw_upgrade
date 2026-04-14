################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rzn/aws/amazon-freertos/freertos_kernel/event_groups.c \
../rzn/aws/amazon-freertos/freertos_kernel/list.c \
../rzn/aws/amazon-freertos/freertos_kernel/queue.c \
../rzn/aws/amazon-freertos/freertos_kernel/stream_buffer.c \
../rzn/aws/amazon-freertos/freertos_kernel/tasks.c \
../rzn/aws/amazon-freertos/freertos_kernel/timers.c 

LST += \
event_groups.lst \
list.lst \
queue.lst \
stream_buffer.lst \
tasks.lst \
timers.lst 

C_DEPS += \
./rzn/aws/amazon-freertos/freertos_kernel/event_groups.d \
./rzn/aws/amazon-freertos/freertos_kernel/list.d \
./rzn/aws/amazon-freertos/freertos_kernel/queue.d \
./rzn/aws/amazon-freertos/freertos_kernel/stream_buffer.d \
./rzn/aws/amazon-freertos/freertos_kernel/tasks.d \
./rzn/aws/amazon-freertos/freertos_kernel/timers.d 

OBJS += \
./rzn/aws/amazon-freertos/freertos_kernel/event_groups.o \
./rzn/aws/amazon-freertos/freertos_kernel/list.o \
./rzn/aws/amazon-freertos/freertos_kernel/queue.o \
./rzn/aws/amazon-freertos/freertos_kernel/stream_buffer.o \
./rzn/aws/amazon-freertos/freertos_kernel/tasks.o \
./rzn/aws/amazon-freertos/freertos_kernel/timers.o 

MAP += \
RZN2L_xspi0_boot_sbl.map 


# Each subdirectory must supply rules for building sources it contributes
rzn/aws/amazon-freertos/freertos_kernel/%.o: ../rzn/aws/amazon-freertos/freertos_kernel/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-r52 -mthumb -mfloat-abi=hard -mfpu=neon-fp-armv8 -fdiagnostics-parseable-fixits -O1 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wnull-dereference -g -D_RENESAS_RZN_ -D_RZN_CORE=CR52_0 -D_RZN_ORDINAL=1 -DSBL_ENABLE=1 -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_gen" -I"." -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\fsp_cfg\\bsp" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\fsp_cfg" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc\\api" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\inc\\instances" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\src\\bsp\\mcu\\all\\cr" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\arm\\CMSIS_5\\CMSIS\\Core_R\\Include" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn_cfg\\driver" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\common" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\fsp\\src\\rm_freertos_port\\cr" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_boot_sbl\\rzn\\aws\\amazon-freertos\\freertos_kernel\\include" -std=c99 -Wno-format-truncation -Wno-stringop-overflow --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@arm-none-eabi-gcc @"$@.in"

