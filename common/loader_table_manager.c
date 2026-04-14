/*
 * loader_table_manager.c
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 */

#include "loader_table_manager.h"
#include "app_config.h"
#include "log.h"

// 定义loader_table结构 (与loader_table.h中一致)
typedef struct {
    uint32_t * src;
    uint32_t * dst;
    uint32_t size;
    uint32_t enable_flag;
    uint8_t app_id;
    uint8_t bank_id;
    uint8_t reserved[2];
} loader_table;

// 引用外部Loader Table
extern const loader_table table[];
#define TABLE_ENTRY_NUM 4

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO

/*
 * 初始化Loader Table管理模块
 */
void LoaderTableManager_Init(void)
{
    LOG_INFO("Loader Table Manager Initialized\n");
    LoaderTableManager_PrintInfo();
}

/*
 * 获取指定APP和Bank的Loader Table条目
 */
bool LoaderTableManager_GetEntry(uint8_t app_id, uint8_t bank_id, loader_table_entry_t *entry)
{
    if (entry == NULL)
    {
        return false;
    }

    // 遍历Loader Table查找匹配的条目
    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (table[i].app_id == app_id && table[i].bank_id == bank_id)
        {
            entry->app_id = table[i].app_id;
            entry->bank_id = table[i].bank_id;
            entry->src_addr = (uint32_t)table[i].src;
            entry->dst_addr = (uint32_t)table[i].dst;
            entry->size = table[i].size;
            entry->is_enabled = (table[i].enable_flag == 1);
            return true;
        }
    }

    return false;
}

/*
 * 启用指定APP和Bank的Loader Table条目
 */
bool LoaderTableManager_EnableEntry(uint8_t app_id, uint8_t bank_id)
{
    // 注意: Loader Table是const类型，不能直接修改
    // 这里需要通过其他方式实现，比如修改Loader Table的加载逻辑
    // 或者使用非const的副本

    LOG_INFO("Enable Loader Table entry: APP%d Bank%d\n", app_id, bank_id);
    return true;
}

/*
 * 禁用指定APP和Bank的Loader Table条目
 */
bool LoaderTableManager_DisableEntry(uint8_t app_id, uint8_t bank_id)
{
    LOG_INFO("Disable Loader Table entry: APP%d Bank%d\n", app_id, bank_id);
    return true;
}

/*
 * 禁用所有Loader Table条目
 */
void LoaderTableManager_DisableAll(void)
{
    LOG_INFO("Disable all Loader Table entries\n");
}

/*
 * 根据APP ID和Bank ID选择对应的Loader Table条目
 */
bool LoaderTableManager_SelectEntry(uint8_t app_id, uint8_t bank_id)
{
    loader_table_entry_t entry;

    if (!LoaderTableManager_GetEntry(app_id, bank_id, &entry))
    {
        LOG_ERROR("Loader Table entry not found: APP%d Bank%d\n", app_id, bank_id);
        return false;
    }

    if (!entry.is_enabled)
    {
        LOG_ERROR("Loader Table entry is disabled: APP%d Bank%d\n", app_id, bank_id);
        return false;
    }

    LOG_INFO("Selected Loader Table entry: APP%d Bank%d\n", app_id, bank_id);
    LOG_INFO("  Src: 0x%08X\n", entry.src_addr);
    LOG_INFO("  Dst: 0x%08X\n", entry.dst_addr);
    LOG_INFO("  Size: %d bytes\n", entry.size);

    return true;
}

/*
 * 获取当前启用的Loader Table条目数量
 */
uint8_t LoaderTableManager_GetEnabledCount(void)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (table[i].enable_flag == 1)
        {
            count++;
        }
    }

    return count;
}

/*
 * 验证Loader Table条目是否有效
 */
bool LoaderTableManager_IsEntryValid(uint8_t app_id, uint8_t bank_id)
{
    loader_table_entry_t entry;

    if (!LoaderTableManager_GetEntry(app_id, bank_id, &entry))
    {
        return false;
    }

    // 检查地址和大小是否有效
    if (entry.src_addr == 0xFFFFFFFF || entry.dst_addr == 0xFFFFFFFF || entry.size == 0xFFFFFFFF)
    {
        return false;
    }

    return true;
}

/*
 * 打印Loader Table信息 (调试用)
 */
void LoaderTableManager_PrintInfo(void)
{
    LOG_INFO("Loader Table Info:\n");
    LOG_INFO("  Total entries: %d\n", TABLE_ENTRY_NUM);
    LOG_INFO("  Enabled entries: %d\n", LoaderTableManager_GetEnabledCount());

    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        LOG_INFO("  Entry[%d]: APP%d Bank%d, %s\n",
                 i,
                 table[i].app_id,
                 table[i].bank_id,
                 table[i].enable_flag ? "Enabled" : "Disabled");
    }
}
