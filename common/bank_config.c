/*
 * bank_config.c
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 */

#include "bank_config.h"
#include "app_config.h"
#include "sbl_boot_params.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO

/*
 * Bank有效性标志 (运行时状态)
 */
static bool bank_valid_table[5][2] = {
    {true, true},   // APP1: Bank0, Bank1
    {true, false},  // APP2: Bank0 only
    {true, false},  // APP3: Bank0 only
    {true, false},  // APP4: Bank0 only
    {true, false}   // APP5: Bank0 only
};

/*
 * 初始化Bank配置管理模块
 */
void BankConfig_Init(void)
{
    LOG_INFO("Bank Config Module Initialized\n");

    // 根据APP配置初始化Bank有效性
    for (uint8_t app_id = 1; app_id <= 5; app_id++)
    {
        if (AppConfig_IsEnabled(app_id))
        {
            bool is_dual_bank = AppConfig_IsDualBank(app_id);
            bank_valid_table[app_id - 1][0] = true;  // Bank0总是有效
            bank_valid_table[app_id - 1][1] = is_dual_bank;  // Bank1根据双Bank配置

            LOG_INFO("  APP%d: %s mode, Bank0=%s, Bank1=%s\n",
                     app_id,
                     is_dual_bank ? "Dual-Bank" : "Single-Bank",
                     bank_valid_table[app_id - 1][0] ? "Valid" : "Invalid",
                     bank_valid_table[app_id - 1][1] ? "Valid" : "Invalid");
        }
    }
}

/*
 * 获取Bank状态信息
 */
bool BankConfig_GetStatus(uint8_t app_id, bank_status_t *status)
{
    if (status == NULL || !AppConfig_IsValidAppId(app_id))
    {
        return false;
    }

    if (!AppConfig_IsEnabled(app_id))
    {
        return false;
    }

    status->app_id = app_id;
    status->current_bank = BankConfig_GetCurrentBank(app_id);
    status->target_bank = BankConfig_GetTargetBank(app_id);
    status->bank0_valid = bank_valid_table[app_id - 1][0];
    status->bank1_valid = bank_valid_table[app_id - 1][1];

    return true;
}

/*
 * 设置目标Bank
 */
bool BankConfig_SetTargetBank(uint8_t app_id, uint8_t target_bank)
{
    if (!AppConfig_IsEnabled(app_id) || !BankConfig_IsValidBankId(target_bank))
    {
        return false;
    }

    // 检查是否为双Bank模式
    if (!AppConfig_IsDualBank(app_id) && target_bank == 1)
    {
        LOG_ERROR("APP%d is single-bank mode, cannot set target to Bank1\n", app_id);
        return false;
    }

    // 检查目标Bank是否有效
    if (!BankConfig_IsBankValid(app_id, target_bank))
    {
        LOG_ERROR("APP%d Bank%d is not valid\n", app_id, target_bank);
        return false;
    }

    // 更新SBL Boot Params
    sbl_boot_params_t params;
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params\n");
        return false;
    }

    params.target_bank = target_bank;
    SblBootParams_UpdateCRC(&params);

    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params\n");
        return false;
    }

    LOG_INFO("APP%d target bank set to Bank%d\n", app_id, target_bank);
    return true;
}

/*
 * 获取当前Bank
 */
uint8_t BankConfig_GetCurrentBank(uint8_t app_id)
{
    if (!AppConfig_IsEnabled(app_id))
    {
        return 0xFF;
    }

    // 从SBL Boot Params获取当前Bank
    return SblBootParams_GetCurrentBank();
}

/*
 * 获取目标Bank
 */
uint8_t BankConfig_GetTargetBank(uint8_t app_id)
{
    if (!AppConfig_IsEnabled(app_id))
    {
        return 0xFF;
    }

    // 从SBL Boot Params获取目标Bank
    sbl_boot_params_t params;
    if (SblBootParams_Read(&params))
    {
        return params.target_bank;
    }

    return 0xFF;  // 自动选择
}

/*
 * 选择下次启动的Bank (根据Bank有效性自动选择)
 */
uint8_t BankConfig_SelectNextBank(uint8_t app_id)
{
    if (!AppConfig_IsEnabled(app_id))
    {
        return 0xFF;
    }

    // 获取目标Bank
    uint8_t target_bank = BankConfig_GetTargetBank(app_id);

    // 如果目标Bank有效，使用目标Bank
    if (target_bank != 0xFF && BankConfig_IsBankValid(app_id, target_bank))
    {
        return target_bank;
    }

    // 否则使用当前Bank
    uint8_t current_bank = BankConfig_GetCurrentBank(app_id);
    if (BankConfig_IsBankValid(app_id, current_bank))
    {
        return current_bank;
    }

    // 如果当前Bank也无效，尝试另一个Bank (双Bank模式)
    if (AppConfig_IsDualBank(app_id))
    {
        uint8_t other_bank = (current_bank == 0) ? 1 : 0;
        if (BankConfig_IsBankValid(app_id, other_bank))
        {
            return other_bank;
        }
    }

    // 所有Bank都无效
    LOG_ERROR("APP%d: No valid bank available!\n", app_id);
    return 0xFF;
}

/*
 * 检查Bank是否有效
 */
bool BankConfig_IsBankValid(uint8_t app_id, uint8_t bank_id)
{
    if (!AppConfig_IsValidAppId(app_id) || !BankConfig_IsValidBankId(bank_id))
    {
        return false;
    }

    return bank_valid_table[app_id - 1][bank_id];
}

/*
 * 标记Bank为有效/无效
 */
bool BankConfig_SetBankValid(uint8_t app_id, uint8_t bank_id, bool is_valid)
{
    if (!AppConfig_IsEnabled(app_id) || !BankConfig_IsValidBankId(bank_id))
    {
        return false;
    }

    // 单Bank模式下不能设置Bank1
    if (!AppConfig_IsDualBank(app_id) && bank_id == 1)
    {
        return false;
    }

    bank_valid_table[app_id - 1][bank_id] = is_valid;

    LOG_INFO("APP%d Bank%d marked as %s\n",
             app_id, bank_id, is_valid ? "Valid" : "Invalid");

    return true;
}

/*
 * Bank切换 (更新current_bank为target_bank)
 */
bool BankConfig_SwitchBank(uint8_t app_id)
{
    if (!AppConfig_IsEnabled(app_id))
    {
        return false;
    }

    uint8_t target_bank = BankConfig_SelectNextBank(app_id);

    if (target_bank == 0xFF)
    {
        LOG_ERROR("APP%d: Cannot switch bank, no valid bank available\n", app_id);
        return false;
    }

    // 更新SBL Boot Params
    sbl_boot_params_t params;
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params\n");
        return false;
    }

    params.current_bank = target_bank;
    params.target_bank = 0xFF;  // 重置为自动选择
    SblBootParams_UpdateCRC(&params);

    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params\n");
        return false;
    }

    LOG_INFO("APP%d switched to Bank%d\n", app_id, target_bank);
    return true;
}

/*
 * 验证Bank ID是否有效
 */
bool BankConfig_IsValidBankId(uint8_t bank_id)
{
    return (bank_id == 0 || bank_id == 1);
}
