/*
 * flash_config.h
 *
 *  Created on: 2026年2月5日
 *      Author: Jerry.Chen
 */
#ifndef FLASH_CONFIG_H_
#define FLASH_CONFIG_H_

#include "bsp_api.h"

/*      SBL和APP的使能配置
 * 通过宏定义控制是否启用SBL和各个APP，以及APP是单bank还是双bank模式
 * 这些宏可以在编译时通过编译器命令行定义覆盖，或者直接在代码中修改默认值
 */  
#ifndef SBL_ENABLE
#define SBL_ENABLE              1   // 默认启用 SBL
#endif

#ifndef APP1_ENABLE
#define APP1_ENABLE             1   // 默认启用 app1
#endif

#ifndef APP2_ENABLE
#define APP2_ENABLE             0   // 默认禁用 app2
#endif

#ifndef APP3_ENABLE
#define APP3_ENABLE             0   // 默认禁用 app3
#endif

#ifndef APP4_ENABLE
#define APP4_ENABLE             0   // 默认禁用 app4
#endif

#ifndef APP5_ENABLE
#define APP5_ENABLE             0   // 默认禁用 app5
#endif
/*
 * APP Bank 模式配置
 * 控制各 APP 是单 bank 还是双 bank 模式
 */
#ifndef APP1_DUAL_BANK
#define APP1_DUAL_BANK          1   // app1 默认双 bank
#endif

#ifndef APP2_DUAL_BANK
#define APP2_DUAL_BANK          0   // app2 默认单 bank
#endif

#ifndef APP3_DUAL_BANK
#define APP3_DUAL_BANK          0   // app3 默认单 bank
#endif

#ifndef APP4_DUAL_BANK
#define APP4_DUAL_BANK          0   // app4 默认单 bank
#endif

#ifndef APP5_DUAL_BANK
#define APP5_DUAL_BANK          0   // app5 默认单 bank
#endif

/*  APP ID 定义
 * 每个 APP 对应一个唯一的 ID，方便在代码中识别和区分不同的 APP
 * 这些 ID 也可以用于 SBL Boot Params 中，指示当前启动的是哪个 APP
 */
#define APP1_ID     1
#define APP2_ID     2
#define APP3_ID     3
#define APP4_ID     4
#define APP5_ID     5

#define BANK0_ID    0
#define BANK1_ID    1

extern uint32_t APP1_BANK0_BASE_ADDR[];
extern uint32_t APP1_BANK1_BASE_ADDR[];

/* APP Flash地址和大小配置
 * 由链接器脚本定义的符号提供，编译时链接器会将这些符号替换为实际的地址和大小
 * 通过宏定义映射到loader_table.c中使用
 */
#if SBL_ENABLE

/* loader_table_t definition (256 bytes per entry) */
//APP1_HEADER_LENS = 0x00000074;          /*116B*/
//APP1_IDENTIFY_LENS = 0x00000010;        /*16B*/
//因为ECAT FOE单次传输长度是116，#define FW_UP_PACKAGE_SIZE      (116)// package send size
//所以固件头部最长100，方便第一包就解析数据，与FW_UP_PACKAGE_SIZE相同
#define ECAT_FOE_PACKAGE_LENS      	100
#define LOADER_TABLE_SIZE  			ECAT_FOE_PACKAGE_LENS
#define SBL_BOOT_PARAMS_SIZE  		ECAT_FOE_PACKAGE_LENS
// 也可以直接定义版本号，不使用字符串解析
#define APP1_HEADER_BUF_LENS  		ECAT_FOE_PACKAGE_LENS
#define APP1_IDENTIFY_BUF_LENS  	16/sizeof(uint32_t) // 4个32位字段



extern uint32_t SBL_BOOT_PARAMS_ADDR[];
extern uint32_t SBL_BOOT_PARAMS_ADDR_BACKUP[];

extern uint32_t APP1_HEADER_LENS[];         /*256B*/
extern uint32_t APP1_IDENTIFY_LENS[];       /*16B*/
#define APP1_FW_OFFSET_HEADER   ( (uint32_t)APP1_HEADER_LENS + (uint32_t)APP1_IDENTIFY_LENS) // 256+4x4
 
#endif//#if SBL_ENABLE


/*********************************************************************************** 
 * APP Flash地址和大小配置
 * 由链接器脚本定义的符号提供，编译时链接器会将这些符号替换为实际的地址和大小
 * 通过宏定义映射到loader_table.c中使用
 */
#if APP1_ENABLE

/* APP1版本号定义
 * 版本号由4个8位字段组成：主版本号、次版本号、补丁号和修订号
 * 通过宏定义设置默认版本号，也可以在编译时覆盖
 */
#define APP1_STR "APP1"		//固定格式不要修改：APP1 APP2

#define APP1_STR_1 ((APP1_STR)[0])  // 'A'
#define APP1_STR_2 ((APP1_STR)[1])  // 'P'
#define APP1_STR_3 ((APP1_STR)[2])  // 'P'
#define APP1_STR_4 ((APP1_STR)[3])  // '1'

/* 版本号定义
 修改下方 VERSION_MAJOR/MINOR/PATCH/REVISION 四个值来设置版本号
 字符串 APP1_VERSION_STR 会自动同步更新
 */
#define APP1_VERSION_MAJOR      1
#define APP1_VERSION_MINOR      2
#define APP1_VERSION_PATCH      3
#define APP1_VERSION_REVISION   11
#define APP1_VERSION ((APP1_VERSION_MAJOR << 24) | (APP1_VERSION_MINOR << 16) | (APP1_VERSION_PATCH << 8) | APP1_VERSION_REVISION)

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define APP1_VERSION_STR STRINGIFY(APP1_VERSION_MAJOR) "." STRINGIFY(APP1_VERSION_MINOR) "." STRINGIFY(APP1_VERSION_PATCH) "." STRINGIFY(APP1_VERSION_REVISION)



/* Bank类型定义 */
#define BANK_UNKNOWN            0xFF
#define BANK_0                  0
#define BANK_1                  1

/* SBL Boot Params 配置选项 */
#define SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE    0   // 默认禁用版本号检查

#define FW_UP_BOOT_PARAMS_SIZE          (4 * 1024)      /* Sector size = 4 KB   */
#define FW_UP_BOOT_PARAMS_BACKUP_SIZE   (4 * 1024)      /* Boot Params backup 4KB */
#define FW_UP_SECTOR_SIZE               (64 * 1024)     /* Sector size = 64 KB   */
#define FW_UP_TOTAL_SIZE                (128 * 1024)    /* Sector size = 128 KB   */

/* Flash driver WRITE API restriction */
#define FW_UP_WRITE_ATONCE_SIZE (64) // Byte
#define FW_UP_PAGE_SIZE         (256)// page program

#define FW_UP_MIRROR_OFFSET     (0x20000000)    /* xSPI0 Mirror space minus offset  */

#if defined(__ICCARM__)
#elif defined(__GNUC__)
extern uint32_t APP1_BANK0_IMAGE_APP_FLASH_section_start;
extern uint32_t APP1_BANK0_IMAGE_APP_RAM_start;
extern uint32_t APP1_BANK0_IMAGE_APP_FLASH_section_size;

#define app1_bank0_prg_flash_addr (&APP1_BANK0_IMAGE_APP_FLASH_section_start)
#define app1_bank0_prg_start_addr (&APP1_BANK0_IMAGE_APP_RAM_start)
#define app1_bank0_prg_size       (&APP1_BANK0_IMAGE_APP_FLASH_section_size)

extern uint32_t APP1_BANK1_IMAGE_APP_FLASH_section_start;
extern uint32_t APP1_BANK1_IMAGE_APP_RAM_start;
extern uint32_t APP1_BANK1_IMAGE_APP_FLASH_section_size;

#define app1_bank1_prg_flash_addr (&APP1_BANK1_IMAGE_APP_FLASH_section_start)
#define app1_bank1_prg_start_addr (&APP1_BANK1_IMAGE_APP_RAM_start)
#define app1_bank1_prg_size       (&APP1_BANK1_IMAGE_APP_FLASH_section_size)

#endif//#if defined(__GNUC__)  

#endif//#if APP1_ENABLE

#if APP2_ENABLE
#define APP2_ID     2

#endif//#if APP2_ENABLE

#if APP3_ENABLE
#define APP3_ID     3

#endif//#if APP3_ENABLE

#if APP4_ENABLE
#define APP4_ID     4

#endif//#if APP4_ENABLE

#if APP5_ENABLE
#define APP5_ID     5

#endif//#if APP5_ENABLE

#endif /* FLASH_CONFIG_H_ */
