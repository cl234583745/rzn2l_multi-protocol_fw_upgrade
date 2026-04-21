/*
 * bank_detection.c
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 */

#include "bank_detection.h"
#include "sbl_boot_params.h"
#include "flash_config.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_DEBUG

// 当前Bank缓存
static uint8_t g_current_bank = BANK_UNKNOWN;

/*
 * 初始化Bank检测
 */
void BankDetection_Init(void)
{
    // 方法1: 从Boot Params读取
    g_current_bank = SblBootParams_GetCurrentBank();

    if (g_current_bank != BANK_UNKNOWN)
    {
        LOG_INFO("BankDetection: Current Bank=%d (from Boot Params)\n", g_current_bank);
        return;
    }

    // 方法2: 通过链接符号判断 (备用方案)
    // extern uint32_t __image_start;
    // uint32_t image_start = (uint32_t)&__image_start;
    //
    // if (image_start >= FW_UP_BANK1_ADDR && image_start < (FW_UP_BANK1_ADDR + FW_UP_TOTAL_SIZE))
    //     g_current_bank = BANK_1;
    // else if (image_start >= FW_UP_BANK0_ADDR && image_start < (FW_UP_BANK0_ADDR + FW_UP_TOTAL_SIZE))
    //     g_current_bank = BANK_0;
    // else
    //     g_current_bank = BANK_UNKNOWN;

    // 方法3: 默认Bank0 (最后的备用方案)
    if (g_current_bank == BANK_UNKNOWN)
    {
        g_current_bank = BANK_0;
        LOG_WARN("BankDetection: Cannot detect Bank, default to Bank0\n");
    }

    LOG_INFO("BankDetection: Current Bank=%d\n", g_current_bank);
}

/*
 * 获取当前运行的Bank
 */
uint8_t BankDetection_GetCurrentBank(void)
{
    return g_current_bank;
}

/*
 * 获取目标写入Bank (另一个Bank)
 */
uint8_t BankDetection_GetTargetBank(uint8_t current_bank)
{
    if (current_bank == BANK_0)
        return BANK_1;
    else if (current_bank == BANK_1)
        return BANK_0;
    else
        return BANK_UNKNOWN;
}

/*
 * 检查固件头的目标Bank是否有效
 */
bool BankDetection_IsTargetBankValid(uint8_t target_bank, uint8_t current_bank)
{
    // 自动模式: 总是写入另一个Bank
    if (target_bank == BANK_UNKNOWN)
        return true;

    // 强制模式: 不能写入当前Bank
    if (target_bank == current_bank)
    {
        LOG_ERROR("Cannot upgrade to current Bank! current=%d, target=%d\n",
                  current_bank, target_bank);
        return false;
    }

    // 有效
    return true;
}

/*
 * 获取当前Bank的起始地址
 */
uint32_t BankDetection_GetBankAddress(uint8_t bank)
{
	uint32_t addr = 0;

    if (bank == BANK_0)
    {
    	addr = (uint32_t)APP1_BANK0_BASE_ADDR;
        return addr;
    }
    else if (bank == BANK_1)
    {
		addr = (uint32_t)APP1_BANK1_BASE_ADDR;
		return addr;
	}
    else
        return 0;
}
