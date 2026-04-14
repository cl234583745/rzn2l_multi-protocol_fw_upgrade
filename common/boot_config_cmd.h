/*
 * boot_config_cmd.h
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 */

#ifndef COMMON_BOOT_CONFIG_CMD_H_
#define COMMON_BOOT_CONFIG_CMD_H_

#include <stdint.h>
#include <stdbool.h>
#include "sbl_boot_params.h"

/*
 * Boot配置命令接口
 * 用于通过EtherCAT FoE修改 SBL Boot Params 配置
 */

// 配置命令魔数
#define BOOT_CONFIG_CMD_MAGIC      "BOOTCFG"

// 配置命令类型
typedef enum {
    BOOT_CONFIG_CMD_DISABLE_VERSION_CHECK = 0x01,  // 禁用版本号检查
    BOOT_CONFIG_CMD_ENABLE_VERSION_CHECK  = 0x02,  // 启用版本号检查
    BOOT_CONFIG_CMD_SET_TARGET_BANK       = 0x03,  // 设置目标Bank
    BOOT_CONFIG_CMD_GET_CURRENT_CONFIG    = 0x04,  // 获取当前配置
} boot_config_cmd_type_t;

// 配置命令结构
#pragma pack(1)
typedef struct {
    uint8_t magic[7];           // "BOOTCFG"
    uint8_t cmd_type;           // 命令类型
    uint8_t param[8];           // 参数
    uint32_t crc32;             // CRC校验
} boot_config_cmd_t;
#pragma pack()

// 处理配置命令
bool BootConfig_ProcessCommand(const boot_config_cmd_t *cmd);

// 禁用版本号检查
bool BootConfig_DisableVersionCheck(void);

// 启用版本号检查
bool BootConfig_EnableVersionCheck(void);

// 设置目标Bank
bool BootConfig_SetTargetBank(uint8_t bank);

// 获取当前配置
bool BootConfig_GetCurrentConfig(sbl_boot_params_t *params);

#endif /* COMMON_BOOT_CONFIG_CMD_H_ */
