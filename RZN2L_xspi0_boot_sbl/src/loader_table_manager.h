/*
 * loader_table_manager.h
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 *
 *  Loader Table Manager - Loader Table 管理模块
 *  用于管理多个APP的Loader Table配置
 */

#ifndef COMMON_LOADER_TABLE_MANAGER_H_
#define COMMON_LOADER_TABLE_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Loader Table 条目信息
 */
typedef struct {
    uint8_t app_id;             // APP ID: 1-5
    uint8_t bank_id;            // Bank ID: 0=Bank0, 1=Bank1
    uint32_t src_addr;          // 源地址 (Flash)
    uint32_t dst_addr;          // 目标地址 (RAM)
    uint32_t size;              // 大小
    bool is_enabled;            // 是否启用
} loader_table_entry_t;

/*
 * Loader Table Manager API
 */

// 初始化Loader Table管理模块
void LoaderTableManager_Init(void);

// 获取指定APP和Bank的Loader Table条目
bool LoaderTableManager_GetEntry(uint8_t app_id, uint8_t bank_id, loader_table_entry_t *entry);

// 启用指定APP和Bank的Loader Table条目
bool LoaderTableManager_EnableEntry(uint8_t app_id, uint8_t bank_id);

// 禁用指定APP和Bank的Loader Table条目
bool LoaderTableManager_DisableEntry(uint8_t app_id, uint8_t bank_id);

// 禁用所有Loader Table条目
void LoaderTableManager_DisableAll(void);

// 根据APP ID和Bank ID选择对应的Loader Table条目
bool LoaderTableManager_SelectEntry(uint8_t app_id, uint8_t bank_id);

// 获取当前启用的Loader Table条目数量
uint8_t LoaderTableManager_GetEnabledCount(void);

// 验证Loader Table条目是否有效
bool LoaderTableManager_IsEntryValid(uint8_t app_id, uint8_t bank_id);

// 打印Loader Table信息 (调试用)
void LoaderTableManager_PrintInfo(void);

#endif /* COMMON_LOADER_TABLE_MANAGER_H_ */
