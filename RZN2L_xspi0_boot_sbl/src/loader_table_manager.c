/*
 * loader_table_manager.c
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 *
 *  统一Loader Table管理 - 替代app_config
 *  提供统一的APP配置查询API
 */

#include "loader_table_manager.h"
#include "loader_table.h"
#include "flash_config.h"
#include "log.h"



// 引用外部Loader Table
extern const loader_table_t loader_table[];


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
        if (loader_table[i].f.app_id == app_id && loader_table[i].f.bank_id == bank_id)
        {
            entry->app_id = loader_table[i].f.app_id;
            entry->bank_id = loader_table[i].f.bank_id;
            entry->src_addr = (uint32_t)loader_table[i].f.src;
            entry->dst_addr = (uint32_t)loader_table[i].f.dst;
            entry->size = loader_table[i].f.size;
            entry->is_enabled = (loader_table[i].f.enable_flag == 1);

            // is_dual_bank 存储在 bank_id=0 的条目中
            entry->is_dual_bank = false;
            for (uint8_t j = 0; j < TABLE_ENTRY_NUM; j++)
            {
                if (loader_table[j].f.app_id == app_id && loader_table[j].f.bank_id == 0)
                {
                    entry->is_dual_bank = (loader_table[j].f.is_dual_bank == 1);
                    break;
                }
            }
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
        if (loader_table[i].f.enable_flag == 1)
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
                 loader_table[i].f.app_id,
                 loader_table[i].f.bank_id,
                 loader_table[i].f.enable_flag ? "Enabled" : "Disabled");
    }
}

/*
 * AppConfig兼容API - 验证APP ID是否有效
 */
bool LoaderTableManager_IsValidAppId(uint8_t app_id)
{
    if (app_id < 1 || app_id > 5)
    {
        return false;
    }

    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (loader_table[i].f.app_id == app_id && loader_table[i].f.bank_id != 0xFF)
        {
            return true;
        }
    }

    return false;
}

/*
 * AppConfig兼容API - 检查APP是否启用
 */
bool LoaderTableManager_IsEnabled(uint8_t app_id)
{
    for (uint8_t bank_id = 0; bank_id < 2; bank_id++)
    {
        loader_table_entry_t entry;
        if (LoaderTableManager_GetEntry(app_id, bank_id, &entry))
        {
            if (entry.is_enabled)
            {
                return true;
            }
        }
    }
    return false;
}

/*
 * AppConfig兼容API - 检查APP是否为双Bank模式
 */
bool LoaderTableManager_IsDualBank(uint8_t app_id)
{
    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (loader_table[i].f.app_id == app_id && loader_table[i].f.bank_id == 0)
        {
            return (loader_table[i].f.is_dual_bank == 1);
        }
    }
    return false;
}

/*
 * AppConfig兼容API - 获取APP的Bank数量
 */
uint8_t LoaderTableManager_GetBankCount(uint8_t app_id)
{
    uint8_t count = 0;

    for (uint8_t bank_id = 0; bank_id < 2; bank_id++)
    {
        loader_table_entry_t entry;
        if (LoaderTableManager_GetEntry(app_id, bank_id, &entry))
        {
            if (entry.is_enabled)
            {
                count++;
            }
        }
    }

    return count;
}

/*
 * AppConfig兼容API - 获取APP的Bank地址
 */
uint32_t LoaderTableManager_GetBankAddress(uint8_t app_id, uint8_t bank_id)
{
    loader_table_entry_t entry;

    if (!LoaderTableManager_GetEntry(app_id, bank_id, &entry))
    {
        return 0;
    }

    if (!entry.is_enabled)
    {
        return 0;
    }

    return entry.dst_addr;
}

/*
 * AppConfig兼容API - 获取下一个启用的APP ID
 */
uint8_t LoaderTableManager_GetNextEnabledApp(uint8_t current_app_id)
{
    if (current_app_id == 0 || current_app_id > 5)
    {
        for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
        {
            if (loader_table[i].f.bank_id == 0 && loader_table[i].f.enable_flag == TABLE_ENABLE)
            {
                return loader_table[i].f.app_id;
            }
        }
        return 0;
    }

    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (loader_table[i].f.app_id == current_app_id && loader_table[i].f.bank_id == 0)
        {
            for (uint8_t j = i + 1; j < TABLE_ENTRY_NUM; j++)
            {
                if (loader_table[j].f.bank_id == 0 && loader_table[j].f.enable_flag == TABLE_ENABLE)
                {
                    return loader_table[j].f.app_id;
                }
            }
            break;
        }
    }

    for (uint8_t i = 0; i < TABLE_ENTRY_NUM; i++)
    {
        if (loader_table[i].f.bank_id == 0 && loader_table[i].f.enable_flag == TABLE_ENABLE)
        {
            return loader_table[i].f.app_id;
        }
    }

    return 0;
}
