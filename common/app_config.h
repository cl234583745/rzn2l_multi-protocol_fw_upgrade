/*
 * app_config.h
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 *
 *  APP Configuration Management - APP 配置管理模块
 *  用于管理多个APP的配置信息
 */

#ifndef COMMON_APP_CONFIG_H_
#define COMMON_APP_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include "flash_config.h"

/*
 * APP 配置信息结构
 */
typedef struct {
    uint8_t app_id;             // APP ID: 1-5
    bool is_enabled;            // APP是否启用
    bool is_dual_bank;          // 是否双Bank模式
    uint32_t bank0_addr;        // Bank0起始地址
    uint32_t bank1_addr;        // Bank1起始地址 (单Bank时为0)
    uint32_t app_size;          // APP大小
} app_config_info_t;

/*
 * APP配置管理API
 */

// 初始化APP配置管理模块
void AppConfig_Init(void);

// 获取APP配置信息
bool AppConfig_GetInfo(uint8_t app_id, app_config_info_t *config);

// 检查APP是否启用
bool AppConfig_IsEnabled(uint8_t app_id);

// 检查APP是否为双Bank模式
bool AppConfig_IsDualBank(uint8_t app_id);

// 获取APP的Bank数量 (1或2)
uint8_t AppConfig_GetBankCount(uint8_t app_id);

// 获取APP的Bank地址
uint32_t AppConfig_GetBankAddress(uint8_t app_id, uint8_t bank_id);

// 获取启用的APP数量
uint8_t AppConfig_GetEnabledCount(void);

// 获取下一个启用的APP ID (用于循环遍历)
uint8_t AppConfig_GetNextEnabledApp(uint8_t current_app_id);

// 验证APP ID是否有效
bool AppConfig_IsValidAppId(uint8_t app_id);

#endif /* COMMON_APP_CONFIG_H_ */
