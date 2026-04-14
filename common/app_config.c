/*
 * app_config.c
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 */

#include "app_config.h"
#include "log.h"
#include <string.h>

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO

/*
 * APP配置表 (静态配置)
 */
static const app_config_info_t app_config_table[] = {
#if APP1_ENABLE
    {
        .app_id = 1,
        .is_enabled = true,
        .is_dual_bank = APP1_DUAL_BANK,
        .bank0_addr = 0x60100000,
        .bank1_addr = APP1_DUAL_BANK ? 0x60200000 : 0,
        .app_size = 0x00100000  // 1MB
    },
#endif

#if APP2_ENABLE
    {
        .app_id = 2,
        .is_enabled = true,
        .is_dual_bank = APP2_DUAL_BANK,
        .bank0_addr = 0x60300000,
        .bank1_addr = APP2_DUAL_BANK ? 0x60400000 : 0,
        .app_size = 0x00100000
    },
#endif

#if APP3_ENABLE
    {
        .app_id = 3,
        .is_enabled = true,
        .is_dual_bank = APP3_DUAL_BANK,
        .bank0_addr = 0x60500000,
        .bank1_addr = APP3_DUAL_BANK ? 0x60600000 : 0,
        .app_size = 0x00100000
    },
#endif

#if APP4_ENABLE
    {
        .app_id = 4,
        .is_enabled = true,
        .is_dual_bank = APP4_DUAL_BANK,
        .bank0_addr = 0x60700000,
        .bank1_addr = APP4_DUAL_BANK ? 0x60800000 : 0,
        .app_size = 0x00100000
    },
#endif

#if APP5_ENABLE
    {
        .app_id = 5,
        .is_enabled = true,
        .is_dual_bank = APP5_DUAL_BANK,
        .bank0_addr = 0x60900000,
        .bank1_addr = APP5_DUAL_BANK ? 0x60A00000 : 0,
        .app_size = 0x00100000
    },
#endif
};

#define APP_CONFIG_TABLE_SIZE  (sizeof(app_config_table) / sizeof(app_config_info_t))

/*
 * 初始化APP配置管理模块
 */
void AppConfig_Init(void)
{
    LOG_INFO("APP Config Module Initialized\n");
    LOG_INFO("  Enabled APPs: %d\n", AppConfig_GetEnabledCount());
}

/*
 * 获取APP配置信息
 */
bool AppConfig_GetInfo(uint8_t app_id, app_config_info_t *config)
{
    if (config == NULL || !AppConfig_IsValidAppId(app_id))
    {
        return false;
    }

    for (uint8_t i = 0; i < APP_CONFIG_TABLE_SIZE; i++)
    {
        if (app_config_table[i].app_id == app_id)
        {
            memcpy(config, &app_config_table[i], sizeof(app_config_info_t));
            return true;
        }
    }

    return false;
}

/*
 * 检查APP是否启用
 */
bool AppConfig_IsEnabled(uint8_t app_id)
{
    for (uint8_t i = 0; i < APP_CONFIG_TABLE_SIZE; i++)
    {
        if (app_config_table[i].app_id == app_id)
        {
            return app_config_table[i].is_enabled;
        }
    }

    return false;
}

/*
 * 检查APP是否为双Bank模式
 */
bool AppConfig_IsDualBank(uint8_t app_id)
{
    app_config_info_t config;

    if (AppConfig_GetInfo(app_id, &config))
    {
        return config.is_dual_bank;
    }

    return false;
}

/*
 * 获取APP的Bank数量 (1或2)
 */
uint8_t AppConfig_GetBankCount(uint8_t app_id)
{
    if (AppConfig_IsDualBank(app_id))
    {
        return 2;
    }

    return AppConfig_IsEnabled(app_id) ? 1 : 0;
}

/*
 * 获取APP的Bank地址
 */
uint32_t AppConfig_GetBankAddress(uint8_t app_id, uint8_t bank_id)
{
    app_config_info_t config;

    if (!AppConfig_GetInfo(app_id, &config))
    {
        return 0;
    }

    if (bank_id == 0)
    {
        return config.bank0_addr;
    }
    else if (bank_id == 1 && config.is_dual_bank)
    {
        return config.bank1_addr;
    }

    return 0;
}

/*
 * 获取启用的APP数量
 */
uint8_t AppConfig_GetEnabledCount(void)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < APP_CONFIG_TABLE_SIZE; i++)
    {
        if (app_config_table[i].is_enabled)
        {
            count++;
        }
    }

    return count;
}

/*
 * 获取下一个启用的APP ID (用于循环遍历)
 */
uint8_t AppConfig_GetNextEnabledApp(uint8_t current_app_id)
{
    if (current_app_id == 0 || current_app_id > 5)
    {
        // 从第一个APP开始查找
        for (uint8_t i = 0; i < APP_CONFIG_TABLE_SIZE; i++)
        {
            if (app_config_table[i].is_enabled)
            {
                return app_config_table[i].app_id;
            }
        }
        return 0;
    }

    // 查找当前APP在表中的位置
    uint8_t current_index = 0xFF;
    for (uint8_t i = 0; i < APP_CONFIG_TABLE_SIZE; i++)
    {
        if (app_config_table[i].app_id == current_app_id)
        {
            current_index = i;
            break;
        }
    }

    if (current_index == 0xFF)
    {
        return 0;
    }

    // 从下一个位置开始查找
    for (uint8_t i = current_index + 1; i < APP_CONFIG_TABLE_SIZE; i++)
    {
        if (app_config_table[i].is_enabled)
        {
            return app_config_table[i].app_id;
        }
    }

    // 循环回到开头
    for (uint8_t i = 0; i <= current_index; i++)
    {
        if (app_config_table[i].is_enabled)
        {
            return app_config_table[i].app_id;
        }
    }

    return 0;
}

/*
 * 验证APP ID是否有效
 */
bool AppConfig_IsValidAppId(uint8_t app_id)
{
    return (app_id >= 1 && app_id <= 5);
}
