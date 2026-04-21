/*
 * flash_layout.ld - 共享的 Flash 分区布局定义
 * 放在 common/ 目录，供 SBL 和 APP1 工程共用
 *
 * 地址分配规则 (修改 flash_config.h 配置后重新编译):
 * - 单 Bank: 1MB (0x00100000)
 * - 双 Bank: 2MB (0x00200000)
 *
 * 默认配置 (单 Bank):
 *   APP1: 0x60100000 (1MB)
 */



/* ========================================
 * SBL Flash 区域
 * ======================================== */
LOADER_PARAM_ADDR           = 0x60000000;
LOADER_PARAMS_OFFSET        = 0x0000004C;
FLASH_CONTENTS_ADDR         = LOADER_PARAM_ADDR + LOADER_PARAMS_OFFSET;

SBL_LOADER_TABLE_ADDR       = 0x600FD000;
SBL_BOOT_PARAMS_ADDR_BACKUP = 0x600FE000;
SBL_BOOT_PARAMS_ADDR        = 0x600FF000;


/* ========================================
 * APP Flash 区域 (固定 1MB 单 Bank)
 * 如需双 Bank 修改: flash_config.h 中 APPx_DUAL_BANK=1
 * ======================================== */
SINGLE_BANK_SIZE = 0x00100000;
APP1_BANK0_BASE_ADDR = 0x60100000;
APP1_BANK1_BASE_ADDR = 0x60200000;

/* APP1: 1MB */
APP1_BANK0_ADDR = APP1_BANK0_BASE_ADDR;
APP1_BANK1_ADDR = APP1_BANK1_BASE_ADDR + SINGLE_BANK_SIZE;


/* ========================================
 * APP header 参数
 * ======================================== */
APP1_HEADER_ADDR = APP1_BANK0_BASE_ADDR;
APP1_HEADER_LENS = 0x00000100;          /*256B*/
APP1_IDENTIFY_LENS = 0x00000010;        /*16B*/

APP1_IDENTIFY_ADDR = APP1_HEADER_ADDR + APP1_HEADER_LENS;
APP1_CODE_ADDR = APP1_HEADER_ADDR + APP1_HEADER_LENS + APP1_IDENTIFY_LENS;


/* ========================================
 * APP RAM 区域
 * ======================================== */
APP_RAM_START = 0x10010000;