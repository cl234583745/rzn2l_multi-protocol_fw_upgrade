/*
 * app_jump.c
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 */

#include "app_jump.h"
#include "app_config.h"
#include "bank_config.h"
#include "loader_table_manager.h"
#include "sbl_boot_params.h"
#include "flash_config.h"
#include "hal_data.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_INFO




// PRINTF宏定义 (与hal_entry.c一致)
#define PRINTF  1

/*
 * 跳转结果描述字符串
 */
static const char* jump_result_strings[] = {
    "Success",
    "Fail: Invalid APP ID",
    "Fail: Invalid Bank ID",
    "Fail: APP not enabled",
    "Fail: Load error",
    "Fail: Validation error",
    "Fail: No valid bank"
};


static void sbl_copy_multibyte (uintptr_t * src, uintptr_t * dst, uintptr_t bytesize)
{
    uintptr_t i;
    uintptr_t cnt;

    uintptr_t src_mod;
    uint8_t * src_single_byte;
    uint8_t * dst_single_byte;

    if (0 != bytesize)
    {
        /* Copy Count in single byte unit */
        src_mod = (uintptr_t) src % sizeof(uintptr_t);

        if (0 != src_mod)
        {
            src_single_byte = (uint8_t *) src;
            dst_single_byte = (uint8_t *) dst;

            for (i = 0; i < src_mod; i++)
            {
                *dst_single_byte++ = *src_single_byte++;
            }

            dst       = (uintptr_t *) dst_single_byte;
            src       = (uintptr_t *) src_single_byte;
            bytesize -= src_mod;
        }
        else
        {
            /* Do nothing */
        }

        /* Copy Count in multi byte unit */
        cnt = (bytesize + (sizeof(uintptr_t) - 1)) / sizeof(uintptr_t);

        for (i = 0; i < cnt; i++)
        {
            *dst++ = *src++;
        }

        /* Ensuring data-changing */
        __asm volatile ("DSB SY");
    }
    else
    {
        /* Do nothing */
    }
}

/*
 * 初始化APP跳转模块
 */
void AppJump_Init(void)
{
    LOG_INFO("APP Jump Module Initialized\n");
}

/*
 * 跳转到指定APP的指定Bank
 */
app_jump_result_t AppJump_ToApp(uint8_t app_id, uint8_t bank_id)
{
    LOG_INFO("Jump to APP%d Bank%d\n", app_id, bank_id);

    // 验证APP ID
    if (!AppConfig_IsValidAppId(app_id))
    {
        return APP_JUMP_FAIL_INVALID_APP;
    }

    // 验证Bank ID
    if (!BankConfig_IsValidBankId(bank_id))
    {
        return APP_JUMP_FAIL_INVALID_BANK;
    }

    // 检查APP是否启用
    if (!AppConfig_IsEnabled(app_id))
    {
        return APP_JUMP_FAIL_NOT_ENABLED;
    }

    // 检查Bank是否有效
    if (!BankConfig_IsBankValid(app_id, bank_id))
    {
        return APP_JUMP_FAIL_INVALID_BANK;
    }

    // 选择Loader Table条目
    if (!LoaderTableManager_SelectEntry(app_id, bank_id))
    {
        return APP_JUMP_FAIL_LOAD_ERROR;
    }

    // 验证APP镜像
    if (!AppJump_ValidateImage(app_id, bank_id))
    {
        return APP_JUMP_FAIL_VALIDATION_ERROR;
    }

    // 获取跳转信息
    app_jump_info_t jump_info;
    if (!AppJump_GetInfo(app_id, bank_id, &jump_info))
    {
        return APP_JUMP_FAIL_LOAD_ERROR;
    }

    // 注意: Flash源地址已经在loader_table中包含了APP1_FW_OFFSET_HEADER偏移量
    // SRAM目标地址不需要偏移，直接使用即可
    //jump_info.src_addr += APP1_FW_OFFSET_HEADER;

    // 复制APP从Flash到SRAM
    LOG_INFO("Copying APP%d Bank%d from Flash to SRAM...\n", app_id, bank_id);
    LOG_DEBUG("  Source:      0x%08X\n", jump_info.src_addr);
    LOG_DEBUG("  Destination: 0x%08X\n", jump_info.dst_addr);
    LOG_DEBUG("  Size:        %d bytes\n", jump_info.image_size);

    sbl_copy_multibyte((uintptr_t *)jump_info.src_addr,
                       (uintptr_t *)jump_info.dst_addr,
                       jump_info.image_size);

    LOG_INFO("Copy completed\n");

    // 执行跳转 (CR52内核)
    LOG_INFO("Jumping to APP%d Bank%d at 0x%08X\n", app_id, bank_id, jump_info.entry_point);

    // 跳转前准备
    __asm volatile("dsb");  // 确保数据同步

#if PRINTF
    g_uart0.p_api->close(g_uart0.p_ctrl);
    __disable_irq();
#endif

    // 设置跳转地址并跳转 (CR52内核方式)
    void (*app_prg)(void);
    app_prg = (void(*)(void))jump_info.entry_point;

    // 跳转到应用程序
    app_prg();

    // 不应该执行到这里
    return APP_JUMP_SUCCESS;
}

/*
 * 跳转到指定APP (自动选择Bank)
 */
app_jump_result_t AppJump_ToAppAuto(uint8_t app_id)
{
    LOG_INFO("Jump to APP%d (auto select bank)\n", app_id);

    // 验证APP ID
    if (!AppConfig_IsValidAppId(app_id))
    {
        return APP_JUMP_FAIL_INVALID_APP;
    }

    // 检查APP是否启用
    if (!AppConfig_IsEnabled(app_id))
    {
        return APP_JUMP_FAIL_NOT_ENABLED;
    }

    // 自动选择Bank
    uint8_t bank_id = BankConfig_SelectNextBank(app_id);

    if (bank_id == 0xFF)
    {
        return APP_JUMP_FAIL_NO_VALID_BANK;
    }

    return AppJump_ToApp(app_id, bank_id);
}

/*
 * 跳转到下一个APP (用于多APP轮转)
 */
app_jump_result_t AppJump_ToNextApp(void)
{
    // 从SBL Boot Params获取目标APP
    sbl_boot_params_t params;
    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params\n");
        return APP_JUMP_FAIL_INVALID_APP;
    }

    uint8_t target_app = params.target_app;

    // 如果没有指定目标APP，使用当前APP
    if (target_app == 0 || target_app == 0xFF)
    {
        // 获取第一个启用的APP
        target_app = AppConfig_GetNextEnabledApp(0);
    }

    if (target_app == 0)
    {
        LOG_ERROR("No enabled APP found\n");
        return APP_JUMP_FAIL_NO_VALID_BANK;
    }

    return AppJump_ToAppAuto(target_app);
}

/*
 * 获取APP跳转信息
 */
bool AppJump_GetInfo(uint8_t app_id, uint8_t bank_id, app_jump_info_t *info)
{
    if (info == NULL)
    {
        return false;
    }

    loader_table_entry_t entry;
    if (!LoaderTableManager_GetEntry(app_id, bank_id, &entry))
    {
        return false;
    }

    info->app_id = app_id;
    info->bank_id = bank_id;
    info->src_addr = entry.src_addr;  // Flash源地址
    info->dst_addr = entry.dst_addr;  // SRAM目标地址
    info->entry_point = entry.dst_addr;  // 入口点 = SRAM目标地址
    info->image_size = entry.size;
    info->is_valid = entry.is_enabled;

    // 注意: Flash源地址已经在loader_table中包含了APP1_FW_OFFSET_HEADER偏移量
    // SRAM目标地址不需要偏移，直接使用即可

    return true;
}

/*
 * 验证APP镜像
 */
bool AppJump_ValidateImage(uint8_t app_id, uint8_t bank_id)
{
    // TODO: 实现镜像验证逻辑
    // 1. 检查镜像头
    // 2. 验证CRC
    // 3. 检查版本号

    LOG_INFO("Validate APP%d Bank%d image\n", app_id, bank_id);
    return true;
}

/*
 * 获取跳转结果描述字符串
 */
const char* AppJump_GetResultString(app_jump_result_t result)
{
    if (result >= 0 && result < sizeof(jump_result_strings) / sizeof(char*))
    {
        return jump_result_strings[result];
    }

    return "Unknown error";
}
