/*
 * flash_config.h
 *
 *  Created on: 2026年2月5日
 *      Author: Jerry.Chen
 */
#include "bsp_api.h"

#ifndef FLASH_CONFIG_H_
#define FLASH_CONFIG_H_


#ifndef SBL_ENABLE
#define SBL_ENABLE              1   // 默认启用 SBL
#endif

/*
 * APP 启用配置
 * 控制哪些 APP 被启用
 */
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










#if SBL_ENABLE
//fsp_xspi0_boot_loader.ld中定义的符号，链接时由ld文件生成            
extern uint32_t LOADER_PARAM_start;
extern uint32_t LOADER_PARAM_end;

extern uint32_t SBL_LOADER_TABLE_start;
extern uint32_t SBL_LOADER_TABLE_end;

extern uint32_t SBL_BOOT_PARAMS_BACKUP_start;
extern uint32_t SBL_BOOT_PARAMS_BACKUP_end;

extern uint32_t SBL_BOOT_PARAMS_start;
extern uint32_t SBL_BOOT_PARAMS_end;    

/////////////////////////////////////////////////////////////////////////

#if defined(__ICCARM__)
    #pragma section="LOADER_PARAM"
    #pragma section="CPU0_LOADER_TABLE"
    #pragma section="SBL_BOOT_PARAMS_BACKUP"
    #pragma section="SBL_BOOT_PARAMS"
    
    #define loader_param_start_addr   (__section_begin("LOADER_PARAM"))
    #define loader_param_end_addr     (__section_end("LOADER_PARAM"))
    
    #define sbl_loader_table_start_addr (__section_begin("CPU0_LOADER_TABLE"))
    #define sbl_loader_table_end_addr   (__section_end("CPU0_LOADER_TABLE"))
    
    #define sbl_boot_params_backup_start_addr (__section_begin("SBL_BOOT_PARAMS_BACKUP"))
    #define sbl_boot_params_backup_end_addr   (__section_end("SBL_BOOT_PARAMS_BACKUP"))
    
    #define sbl_boot_params_start_addr (__section_begin("SBL_BOOT_PARAMS"))
    #define sbl_boot_params_end_addr   (__section_end("SBL_BOOT_PARAMS"))  

#elif defined(__GNUC__)

    #define loader_param_start_addr   (&LOADER_PARAM_start)
    #define loader_param_end_addr     (&LOADER_PARAM_end)

    #define sbl_loader_table_start_addr (&SBL_LOADER_TABLE_start)
    #define sbl_loader_table_end_addr   (&SBL_LOADER_TABLE_end)

    #define sbl_boot_params_backup_start_addr (&SBL_BOOT_PARAMS_BACKUP_start)           
    #define sbl_boot_params_backup_end_addr   (&SBL_BOOT_PARAMS_BACKUP_end)     

    #define sbl_boot_params_start_addr (&SBL_BOOT_PARAMS_start)                 
    #define sbl_boot_params_end_addr   (&SBL_BOOT_PARAMS_end)   

#endif  
#endif//#if SBL_ENABLE


#if APP1_ENABLE
//fsp_xspi0_boot_loader.ld中定义的符号，链接时由ld文件生成
extern uint32_t APP1_PARAMS_LENS;
extern uint32_t APP1_PARAMS_OFFSET;

extern uint32_t APP1_BANK0_ADDR;
extern uint32_t APP1_BANK0_IMAGE_APP_RAM_start;
extern uint32_t APP1_BANK0_IMAGE_APP_FLASH_section_start;   //0x60100000
extern uint32_t APP1_BANK0_IMAGE_APP_FLASH_section_size;

extern uint32_t APP1_BANK1_ADDR;
extern uint32_t APP1_BANK1_IMAGE_APP_RAM_start;
extern uint32_t APP1_BANK1_IMAGE_APP_FLASH_section_start;   //0x60200000
extern uint32_t APP1_BANK1_IMAGE_APP_FLASH_section_size;

/////////////////////////////////////////////////////////////////
#if defined(__ICCARM__)
     #pragma section="APPLICATION_PRG_RBLOCK"
     #pragma section="APPLICATION_PRG_WBLOCK"

     #define application_prg_flash_addr (__section_begin("APPLICATION_PRG_RBLOCK"))
     #define application_prg_start_addr (__section_begin("APPLICATION_PRG_WBLOCK"))
     #define application_prg_size       (__section_size("APPLICATION_PRG_RBLOCK"))
#elif defined(__GNUC__)
    /* 链接脚本定义的符号已在loader_table.h中声明 */
    #define app1_bank0_prg_flash_addr ((uint32_t)&APP1_BANK0_IMAGE_APP_FLASH_section_start)
    //#define app1_bank0_prg_flash_addr ((uint32_t*)((uint8_t*)&APP1_BANK0_IMAGE_APP_FLASH_section_start + app1_bank0_offset))
    #define app1_bank0_prg_start_addr (&APP1_BANK0_IMAGE_APP_RAM_start)
    #define app1_bank0_prg_size       (&APP1_BANK0_IMAGE_APP_FLASH_section_size)


    #define app1_bank1_prg_flash_addr ((uint32_t)&APP1_BANK1_IMAGE_APP_FLASH_section_start)
    //#define app1_bank1_prg_flash_addr ((uint32_t*)((uint8_t*)&APP1_BANK1_IMAGE_APP_FLASH_section_start + app1_bank1_offset))
    #define app1_bank1_prg_start_addr (&APP1_BANK1_IMAGE_APP_RAM_start)
    #define app1_bank1_prg_size       (&APP1_BANK1_IMAGE_APP_FLASH_section_size)

#endif


/* Bank类型定义 */
#define BANK_UNKNOWN            0xFF
#define BANK_0                  0
#define BANK_1                  1

/* SBL Boot Params 配置选项 */
#define SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE    0   // 默认禁用版本号检查

/* SBL参数区域地址 (每个4KB, 从APP1 Bank0向低地址依次排列) */
#define SBL_LOADER_TABLE_ADDR      SBL_BOOT_PARAMS_BACKUP_start     // Loader Table 4KB (0x60100000 - 0x3000)
#define SBL_BACKUP_PARAMS_ADDR     SBL_BOOT_PARAMS_BACKUP_start     // 备份区 4KB (0x60100000 - 0x2000)
#define SBL_MAIN_PARAMS_ADDR       SBL_BOOT_PARAMS_start            // 主区 4KB (0x60100000 - 0x1000)



#define FW_UP_BANK0_ADDR        APP1_BANK0_ADDR    /* BANK0 user application */
#define FW_UP_BANK1_ADDR        APP1_BANK1_ADDR    /* BANK1 user application */

#define FW_UP_BOOT_PARAMS_SIZE          (4 * 1024)      /* Sector size = 4 KB   */
#define FW_UP_BOOT_PARAMS_BACKUP_SIZE   (4 * 1024)      /* Boot Params backup 4KB */
#define FW_UP_SECTOR_SIZE               (64 * 1024)     /* Sector size = 64 KB   */
#define FW_UP_TOTAL_SIZE                (128 * 1024)    /* Sector size = 128 KB   */

/* Flash driver WRITE API restriction */
#define FW_UP_WRITE_ATONCE_SIZE (64) // Byte
#define FW_UP_PAGE_SIZE         (256)// page program

#define FW_UP_MIRROR_OFFSET     (0x20000000)    /* xSPI0 Mirror space minus offset  */

#define FW_UP_PACKAGE_SIZE      (116)// package send size

#define APP1_HDR                9//APP1BANK0

#define SBL_BOOT_PARAMS_LENS    29// (9+16+4)

#define APP1_FW_OFFSET_HEADER   APP1_PARAMS_LENS + APP1_PARAMS_OFFSET// 0x4c+4x4
#define SBL_BOOT_PARAMS_LENS    29


#endif//#if APP1_ENABLE

#if APP2_ENABLE

#endif//#if APP2_ENABLE

#if APP3_ENABLE

#endif//#if APP3_ENABLE

#if APP4_ENABLE

#endif//#if APP4_ENABLE

#if APP5_ENABLE

#endif//#if APP5_ENABLE

#endif /* FLASH_CONFIG_H_ */
