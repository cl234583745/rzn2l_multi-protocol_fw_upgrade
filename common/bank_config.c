/*
 * bank_config.c
 *
 *  Bank配置管理 - 简化版
 *  配置在 flash_config.h 中修改
 */

#include "bank_config.h"
#include "flash_config.h"
#include "sbl_boot_params.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO

/*
 * 内部配置查询 (简化版)
 */
static bool app_is_enabled(uint8_t app_id)
{
    switch (app_id)
    {
        case 1: return (APP1_ENABLE == 1);
        case 2: return (APP2_ENABLE == 1);
        case 3: return (APP3_ENABLE == 1);
        case 4: return (APP4_ENABLE == 1);
        case 5: return (APP5_ENABLE == 1);
        default: return false;
    }
}

static bool app_is_dual_bank(uint8_t app_id)
{
    switch (app_id)
    {
        case 1: return (APP1_DUAL_BANK == 1);
        case 2: return (APP2_DUAL_BANK == 1);
        case 3: return (APP3_DUAL_BANK == 1);
        case 4: return (APP4_DUAL_BANK == 1);
        case 5: return (APP5_DUAL_BANK == 1);
        default: return false;
    }
}

static bool app_is_valid(uint8_t app_id)
{
    return (app_id >= 1 && app_id <= 5);
}

/*
 * 初始化Bank配置管理模块
 */
void BankConfig_Init(void)
{
    LOG_INFO("Bank Config Module Initialized\n");

    for (uint8_t app_id = 1; app_id <= 5; app_id++)
    {
        if (app_is_enabled(app_id))
        {
            LOG_INFO("  APP%d: %s mode\n",
                     app_id,
                     app_is_dual_bank(app_id) ? "Dual-Bank" : "Single-Bank");
        }
    }
}

/*
 * 获取Bank状态信息
 */
bool BankConfig_GetStatus(uint8_t app_id, bank_status_t *status)
{
    if (status == NULL || !app_is_valid(app_id))
    {
        return false;
    }

    if (!app_is_enabled(app_id))
    {
        return false;
    }

    status->app_id = app_id;
    status->current_bank = BankConfig_GetCurrentBank(app_id);
    status->target_bank = BankConfig_GetTargetBank(app_id);

    status->bank0_valid = true;
    status->bank1_valid = app_is_dual_bank(app_id);

    return true;
}

/*
 * 设置目标Bank
 */
bool BankConfig_SetTargetBank(uint8_t app_id, uint8_t target_bank)
{
    if (!app_is_enabled(app_id) || !BankConfig_IsValidBankId(target_bank))
    {
        return false;
    }

    if (!app_is_dual_bank(app_id) && target_bank == 1)
    {
        LOG_ERROR("APP%d is single-bank mode, cannot set target to Bank1\n", app_id);
        return false;
    }

    if (!BankConfig_IsBankValid(app_id, target_bank))
    {
        LOG_ERROR("APP%d Bank%d is not valid\n", app_id, target_bank);
        return false;
    }

    sbl_boot_params_t params;
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params\n");
        return false;
    }

    params.f.target_bank = target_bank;
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
    if (!app_is_enabled(app_id))
    {
        return 0xFF;
    }

    return SblBootParams_GetCurrentBank();
}

/*
 * 获取目标Bank
 */
uint8_t BankConfig_GetTargetBank(uint8_t app_id)
{
    if (!app_is_enabled(app_id))
    {
        return 0xFF;
    }

    sbl_boot_params_t params;
    if (SblBootParams_Read(&params))
    {
        return params.f.target_bank;
    }

    return 0xFF;  // 自动选择
}

/*
 * 选择下次启动的Bank (根据Bank有效性自动选择)
 */
uint8_t BankConfig_SelectNextBank(uint8_t app_id)
{
    if (!app_is_enabled(app_id))
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
    if (app_is_dual_bank(app_id))
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
    if (!app_is_valid(app_id) || !BankConfig_IsValidBankId(bank_id))
    {
        return false;
    }

    if (!app_is_enabled(app_id))
    {
        return false;
    }

    if (bank_id == 1 && !app_is_dual_bank(app_id))
    {
        return false;
    }

    return true;
}

/*
 * 标记Bank为有效/无效
 */
bool BankConfig_SetBankValid(uint8_t app_id, uint8_t bank_id, bool is_valid)
{
    if (!app_is_enabled(app_id) || !BankConfig_IsValidBankId(bank_id))
    {
        return false;
    }

    if (!app_is_dual_bank(app_id) && bank_id == 1)
    {
        return false;
    }

    LOG_INFO("APP%d Bank%d marked as %s (read-only, use loader_table)\n",
             app_id, bank_id, is_valid ? "Valid" : "Invalid");

    return true;
}

/*
 * Bank切换 (更新current_bank为target_bank)
 */
bool BankConfig_SwitchBank(uint8_t app_id)
{
    if (!app_is_enabled(app_id))
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

    params.f.current_bank = target_bank;
    params.f.target_bank = 0xFF;  // 重置为自动选择
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
    return (bank_id == BANK0_ID || bank_id == BANK1_ID);
}
