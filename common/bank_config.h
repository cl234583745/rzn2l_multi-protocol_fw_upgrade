/*
 * bank_config.h
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 *
 *  Bank Configuration Management - Bank 配置管理模块
 *  用于管理Bank切换和选择逻辑
 */

#ifndef COMMON_BANK_CONFIG_H_
#define COMMON_BANK_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include "flash_config.h"

/*
 * Bank状态信息
 */
typedef struct {
    uint8_t app_id;             // APP ID
    uint8_t current_bank;       // 当前运行的Bank: 0=Bank0, 1=Bank1
    uint8_t target_bank;        // 下次启动的Bank: 0=Bank0, 1=Bank1, 0xFF=自动
    bool bank0_valid;           // Bank0是否有效
    bool bank1_valid;           // Bank1是否有效
} bank_status_t;

/*
 * Bank配置管理API
 */

// 初始化Bank配置管理模块
void BankConfig_Init(void);

// 获取Bank状态信息
bool BankConfig_GetStatus(uint8_t app_id, bank_status_t *status);

// 设置目标Bank
bool BankConfig_SetTargetBank(uint8_t app_id, uint8_t target_bank);

// 获取当前Bank
uint8_t BankConfig_GetCurrentBank(uint8_t app_id);

// 获取目标Bank
uint8_t BankConfig_GetTargetBank(uint8_t app_id);

// 选择下次启动的Bank (根据Bank有效性自动选择)
uint8_t BankConfig_SelectNextBank(uint8_t app_id);

// 检查Bank是否有效
bool BankConfig_IsBankValid(uint8_t app_id, uint8_t bank_id);

// 标记Bank为有效/无效
bool BankConfig_SetBankValid(uint8_t app_id, uint8_t bank_id, bool is_valid);

// Bank切换 (更新current_bank为target_bank)
bool BankConfig_SwitchBank(uint8_t app_id);

// 验证Bank ID是否有效
bool BankConfig_IsValidBankId(uint8_t bank_id);

#endif /* COMMON_BANK_CONFIG_H_ */
