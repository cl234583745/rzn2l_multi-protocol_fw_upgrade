################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ethercat/beckhoff/Src/bootmode.c \
../src/ethercat/beckhoff/Src/cia402appl.c \
../src/ethercat/beckhoff/Src/coeappl.c \
../src/ethercat/beckhoff/Src/ecatappl.c \
../src/ethercat/beckhoff/Src/ecatcoe.c \
../src/ethercat/beckhoff/Src/ecatfoe.c \
../src/ethercat/beckhoff/Src/ecatslv.c \
../src/ethercat/beckhoff/Src/foeappl.c \
../src/ethercat/beckhoff/Src/mailbox.c \
../src/ethercat/beckhoff/Src/objdef.c \
../src/ethercat/beckhoff/Src/sdoserv.c 

LST += \
bootmode.lst \
cia402appl.lst \
coeappl.lst \
ecatappl.lst \
ecatcoe.lst \
ecatfoe.lst \
ecatslv.lst \
foeappl.lst \
mailbox.lst \
objdef.lst \
sdoserv.lst 

C_DEPS += \
./src/ethercat/beckhoff/Src/bootmode.d \
./src/ethercat/beckhoff/Src/cia402appl.d \
./src/ethercat/beckhoff/Src/coeappl.d \
./src/ethercat/beckhoff/Src/ecatappl.d \
./src/ethercat/beckhoff/Src/ecatcoe.d \
./src/ethercat/beckhoff/Src/ecatfoe.d \
./src/ethercat/beckhoff/Src/ecatslv.d \
./src/ethercat/beckhoff/Src/foeappl.d \
./src/ethercat/beckhoff/Src/mailbox.d \
./src/ethercat/beckhoff/Src/objdef.d \
./src/ethercat/beckhoff/Src/sdoserv.d 

OBJS += \
./src/ethercat/beckhoff/Src/bootmode.o \
./src/ethercat/beckhoff/Src/cia402appl.o \
./src/ethercat/beckhoff/Src/coeappl.o \
./src/ethercat/beckhoff/Src/ecatappl.o \
./src/ethercat/beckhoff/Src/ecatcoe.o \
./src/ethercat/beckhoff/Src/ecatfoe.o \
./src/ethercat/beckhoff/Src/ecatslv.o \
./src/ethercat/beckhoff/Src/foeappl.o \
./src/ethercat/beckhoff/Src/mailbox.o \
./src/ethercat/beckhoff/Src/objdef.o \
./src/ethercat/beckhoff/Src/sdoserv.o 

MAP += \
RZN2L_xspi0_app1.map 


# Each subdirectory must supply rules for building sources it contributes
src/ethercat/beckhoff/Src/%.o: ../src/ethercat/beckhoff/Src/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-r52 -mthumb -mfloat-abi=hard -mfpu=neon-fp-armv8 -fdiagnostics-parseable-fixits -O1 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wnull-dereference -g -D_RENESAS_RZN_ -D_RZN_CORE=CR52_0 -D_RZN_ORDINAL=1 -DREVISION_NUMBER=0x00000300 -DETHERCAT_SSC_PORT_GMAC_MDIO_SUPPORT=1 -DxFW_PARSE_SREC -DAPP1_BANK=0 -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_gen" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\renesas" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\beckhoff" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat\\beckhoff\\Src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\src\\ethercat" -I"." -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\fsp_cfg\\bsp" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\fsp_cfg" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc\\api" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\inc\\instances" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\src\\bsp\\mcu\\all\\cr" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\arm\\CMSIS_5\\CMSIS\\Core_R\\Include" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn_cfg\\driver" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\RZN2L_xspi0_app1\\rzn\\fsp\\src\\rm_ethercat_ssc_port" -I"E:\\RS_workspace\\RZN2L_Multi-protocol_FW_Upgrade\\common" -std=c99 -Wno-format-truncation -Wno-stringop-overflow --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@arm-none-eabi-gcc @"$@.in"

