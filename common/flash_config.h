/*
 * flash_config.h
 *
 *  Created on: 2026年2月5日
 *      Author: Jerry.Chen
 */

#ifndef FLASH_CONFIG_H_
#define FLASH_CONFIG_H_

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

#if APP1_ENABLE
/* Bank类型定义 */
#define BANK_UNKNOWN            0xFF
#define BANK_0                  0
#define BANK_1                  1

/* SBL Boot Params 配置选项 */
#define SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE    0   // 默认禁用版本号检查


#define FW_UP_BANK0_ADDR        (0x60100000)    /* BANK0 user applicaiotn */
#define FW_UP_BANK1_ADDR        (0x60200000)    /* BANK1 user applicaiotn */

#define FW_UP_BOOT_PARAMS_SIZE  (4 * 1024)      /* Sector size = 4 KB   */
#define FW_UP_SECTOR_SIZE       (64 * 1024)     /* Sector size = 64 KB   */
#define FW_UP_TOTAL_SIZE        (128 * 1024)    /* Sector size = 128 KB   */

/* Flash driver WRITE API restriction */
#define FW_UP_WRITE_ATONCE_SIZE (64) // Byte
#define FW_UP_PAGE_SIZE         (256)// page program

#define FW_UP_MIRROR_OFFSET     (0x20000000)    /* xSPI0 Mirror space minus offset  */

#define FW_UP_PACKAGE_SIZE      (116)// package send size

#define APP1_HDR                9//APP1BANK0


#define SBL_BOOT_PARAMS_LENS    29// (9+16+4)

#define APP1_FW_OFFSET_HEADER       0x0000005c// 0x4c+4x4


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
