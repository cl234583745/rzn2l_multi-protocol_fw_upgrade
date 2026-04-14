/*
 * bank_detection.h
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 */

#ifndef COMMON_BANK_DETECTION_H_
#define COMMON_BANK_DETECTION_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Bank检测模块
 * 用于运行时检测当前运行的Bank，并确定升级目标Bank
 */

// 初始化Bank检测
void BankDetection_Init(void);

// 获取当前运行的Bank
uint8_t BankDetection_GetCurrentBank(void);

// 获取目标写入Bank (另一个Bank)
uint8_t BankDetection_GetTargetBank(uint8_t current_bank);

// 检查固件头的目标Bank是否有效
bool BankDetection_IsTargetBankValid(uint8_t target_bank, uint8_t current_bank);

// 获取当前Bank的起始地址
uint32_t BankDetection_GetBankAddress(uint8_t bank);

#endif /* COMMON_BANK_DETECTION_H_ */
