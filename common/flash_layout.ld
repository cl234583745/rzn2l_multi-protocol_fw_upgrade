/*
 * flash_layout.ld - 共享的 Flash 分区布局定义
 * 放在 common/ 目录，供 SBL 和 APP1 工程共用
 * 修改此文件即可同时更新两个工程
 */

/* ========================================
 * SBL Flash 区域
 * ======================================== */

/* Loader 参数 */
LOADER_PARAM_ADDRESS = 0x60000000;
LOADER_PARAMS_OFFSET = 0x0000004C;
FLASH_CONTENTS_ADDRESS = LOADER_PARAM_ADDRESS + LOADER_PARAMS_OFFSET;

/* SBL Boot Params (每个 4KB，从高地址往低地址排列) */
SBL_LOADER_TABLE_ADDR    = 0x600FD000;
SBL_BOOT_PARAMS_BACKUP_ADDR = 0x600FE000;
SBL_BOOT_PARAMS_ADDR       = 0x600FF000;

/* SBL Boot Params 符号 (别名，供代码使用) */
SBL_BOOT_PARAMS_start = SBL_BOOT_PARAMS_ADDR;
SBL_BOOT_PARAMS_BACKUP_start = SBL_BOOT_PARAMS_BACKUP_ADDR;

/* ========================================
 * APP1 Flash 区域
 * ======================================== */

/* APP1 Bank 地址 */
APP1_BANK0_ADDR = 0x60100000;
APP1_BANK1_ADDR = 0x60200000;



/* APP1 header 参数 (在 Bank0 内偏移) */
APP1_HEADER_PARAM_ADDR = APP1_BANK0_ADDR;
APP1_PARAMS_LENS = 0x0000004C;
APP1_IDENTIFY_PARAM_ADDR = APP1_HEADER_PARAM_ADDR + APP1_PARAMS_LENS;
APP1_PARAMS_OFFSET = 0x00000010;
APP1_CODE_ADDR = APP1_IDENTIFY_PARAM_ADDR + APP1_PARAMS_OFFSET;