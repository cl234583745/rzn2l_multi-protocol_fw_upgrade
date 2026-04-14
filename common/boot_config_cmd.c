/*
 * boot_config_cmd.c
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 */

#include "boot_config_cmd.h"
#include "sbl_boot_params.h"
#include "crc32_table.h"
#include "log.h"
#include <string.h>

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO

extern CRC_Context ctx;

/*
 * 禁用版本号检查
 */
bool BootConfig_DisableVersionCheck(void)
{
    sbl_boot_params_t params;

    // 读取当前 SBL Boot Params
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return false;
    }

    // 禁用版本号检查
    params.version_check_enable = 0;

    // 更新CRC
    SblBootParams_UpdateCRC(&params);

    // 写回 SBL Boot Params
    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params!\n");
        return false;
    }

    LOG_INFO("Version check DISABLED!\n");
    LOG_INFO("Now you can downgrade firmware.\n");
    return true;
}

/*
 * 启用版本号检查
 */
bool BootConfig_EnableVersionCheck(void)
{
    sbl_boot_params_t params;

    // 读取当前 SBL Boot Params
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return false;
    }

    // 启用版本号检查
    params.version_check_enable = 1;

    // 更新CRC
    SblBootParams_UpdateCRC(&params);

    // 写回 SBL Boot Params
    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params!\n");
        return false;
    }

    LOG_INFO("Version check ENABLED!\n");
    LOG_INFO("Firmware upgrade requires higher version.\n");
    return true;
}

/*
 * 设置目标Bank
 */
bool BootConfig_SetTargetBank(uint8_t bank)
{
    if (bank != 0 && bank != 1)
    {
        LOG_ERROR("Invalid bank: %d\n", bank);
        return false;
    }

    sbl_boot_params_t params;

    // 读取当前 SBL Boot Params
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return false;
    }

    // 设置目标Bank
    params.target_bank = bank;

    // 更新CRC
    SblBootParams_UpdateCRC(&params);

    // 写回 SBL Boot Params
    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params!\n");
        return false;
    }

    LOG_INFO("Target Bank set to: Bank%d\n", bank);
    return true;
}

/*
 * 获取当前配置
 */
bool BootConfig_GetCurrentConfig(sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    if (!SblBootParams_Read(params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return false;
    }

    if (!SblBootParams_ValidateCRC(params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return false;
    }

    LOG_INFO("Current Config:\n");
    LOG_INFO("  Version: %d.%d.%d\n",
             (params->header_version >> 16) & 0xFF,
             (params->header_version >> 8) & 0xFF,
             params->header_version & 0xFF);
    LOG_INFO("  Current Bank: %d\n", params->current_bank);
    LOG_INFO("  Target Bank: %d\n", params->target_bank);
    LOG_INFO("  Version Check: %s\n",
             params->version_check_enable ? "Enabled" : "Disabled");

    return true;
}

/*
 * 处理配置命令
 */
bool BootConfig_ProcessCommand(const boot_config_cmd_t *cmd)
{
    if (cmd == NULL)
        return false;

    // 验证魔数
    if (memcmp(cmd->magic, BOOT_CONFIG_CMD_MAGIC, 7) != 0)
    {
        LOG_ERROR("Invalid config command magic!\n");
        return false;
    }

    // 验证CRC
    uint32_t calc_crc = CRC_Calculate(&ctx, (char *)cmd, sizeof(boot_config_cmd_t) - 4);
    if (calc_crc != cmd->crc32)
    {
        LOG_ERROR("Config command CRC check failed!\n");
        return false;
    }

    // 处理命令
    switch (cmd->cmd_type)
    {
    case BOOT_CONFIG_CMD_DISABLE_VERSION_CHECK:
        return BootConfig_DisableVersionCheck();

    case BOOT_CONFIG_CMD_ENABLE_VERSION_CHECK:
        return BootConfig_EnableVersionCheck();

    case BOOT_CONFIG_CMD_SET_TARGET_BANK:
        return BootConfig_SetTargetBank(cmd->param[0]);

    case BOOT_CONFIG_CMD_GET_CURRENT_CONFIG:
        {
            sbl_boot_params_t params;
            return BootConfig_GetCurrentConfig(&params);
        }

    default:
        LOG_ERROR("Unknown config command: 0x%02X\n", cmd->cmd_type);
        return false;
    }
}
