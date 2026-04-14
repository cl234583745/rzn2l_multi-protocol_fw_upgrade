/*
 * sbl_boot_params.c
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 *
 *  SBL Boot Params - SBL (Secondary Boot Loader) 启动参数管理
 *  注意: 此模块用于 SBL (二级引导)，区别于 ROM Boot (一级引导)
 */

#include "sbl_boot_params.h"
#include "flash_config.h"
#include "crc32_table.h"
#include "hal_data.h"
#include "log.h"
#include <string.h>

#define CURRENT_LOG_LEVEL   LOG_LEVEL_DEBUG

// CRC上下文
extern CRC_Context ctx;

/*
 * 初始化 SBL Boot Params
 */
void SblBootParams_Init(void)
{
    sbl_boot_params_t params;

    // 尝试读取现有的 SBL Boot Params
    if (SblBootParams_Read(&params))
    {
        // 验证CRC
        if (SblBootParams_ValidateCRC(&params))
        {
            LOG_INFO("SBL Boot Params already initialized and valid\n");
            return;
        }
    }

    // SBL Boot Params 无效或不存在，初始化默认值
    LOG_INFO("Initializing SBL Boot Params with default values\n");

    // 清零结构
    memset(&params, 0, sizeof(sbl_boot_params_t));

    // 设置默认值
    params.header_app[0] = 'A';
    params.header_app[1] = 'P';
    params.header_app[2] = 'P';
    params.header_version = 0x00010000;  // 版本 1.0.0
    params.target_app = 1;               // 默认跳转到 APP1
    params.current_bank = BANK_0;        // 当前运行 Bank0
    params.target_bank = 0xFF;           // 自动选择 Bank
    params.version_check_enable = SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE;

    // Identify 信息 (默认值)
    params.vendor_id = 0;
    params.product_code = 0;
    params.revision_number = 0;
    params.serial_number = 0;

    // 计算并设置 CRC
    SblBootParams_UpdateCRC(&params);

    // 写入 Flash
    if (SblBootParams_Write(&params))
    {
        LOG_INFO("SBL Boot Params initialized successfully\n");
    }
    else
    {
        LOG_ERROR("Failed to initialize SBL Boot Params\n");
    }
}

/*
 * 读取 SBL Boot Params
 */
bool SblBootParams_Read(sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    // 从Flash读取 SBL Boot Params (地址: FW_UP_BANK0_ADDR - FW_UP_BOOT_PARAMS_SIZE)
    memcpy(params,
           (uint8_t *)(FW_UP_BANK0_ADDR - FW_UP_MIRROR_OFFSET - FW_UP_BOOT_PARAMS_SIZE),
           sizeof(sbl_boot_params_t));

    return true;
}

/*
 * 写入 SBL Boot Params
 */
bool SblBootParams_Write(const sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    spi_flash_status_t status_erase;

    // 1. 擦除 SBL Boot Params 区域
    R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                      (uint8_t *)(FW_UP_BANK0_ADDR - FW_UP_BOOT_PARAMS_SIZE),
                      FW_UP_BOOT_PARAMS_SIZE);

    // 等待擦除完成
    do {
        R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
    } while (status_erase.write_in_progress);

    // 2. 写入 SBL Boot Params (分多次写入，每次64字节)
    uint8_t *data = (uint8_t *)params;
    uint32_t total_size = sizeof(sbl_boot_params_t);
    uint32_t offset = 0;

    while (offset < total_size)
    {
        uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                              FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

        R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                          data + offset,
                          (uint8_t *)(FW_UP_BANK0_ADDR - FW_UP_BOOT_PARAMS_SIZE + offset),
                          write_size);

        // 等待写入完成
        do {
            R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
        } while (status_erase.write_in_progress);

        offset += write_size;
    }

    return true;
}

/*
 * 验证 SBL Boot Params 的 CRC
 */
bool SblBootParams_ValidateCRC(const sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    // 计算CRC (不包括最后的crc32字段)
    uint32_t calc_crc = CRC_Calculate(&ctx,
                                      (char *)params,
                                      sizeof(sbl_boot_params_t) - 4);

    return (calc_crc == params->crc32);
}

/*
 * 更新 SBL Boot Params 的 CRC
 */
void SblBootParams_UpdateCRC(sbl_boot_params_t *params)
{
    if (params == NULL)
        return;

    // 计算CRC (不包括最后的crc32字段)
    params->crc32 = CRC_Calculate(&ctx,
                                  (char *)params,
                                  sizeof(sbl_boot_params_t) - 4);
}

/*
 * 获取当前运行的Bank
 */
uint8_t SblBootParams_GetCurrentBank(void)
{
    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
        return BANK_UNKNOWN;

    if (!SblBootParams_ValidateCRC(&params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return BANK_UNKNOWN;
    }

    return params.current_bank;
}

/*
 * 获取目标APP
 */
uint8_t SblBootParams_GetTargetApp(void)
{
    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
        return 0;

    if (!SblBootParams_ValidateCRC(&params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return 0;
    }

    return params.target_app;
}

/*
 * 设置目标APP
 */
bool SblBootParams_SetTargetApp(uint8_t app_id)
{
    // 验证APP ID
    if (app_id < 1 || app_id > 5)
    {
        LOG_ERROR("Invalid APP ID: %d\n", app_id);
        return false;
    }

    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
    {
        LOG_ERROR("Failed to read SBL Boot Params\n");
        return false;
    }

    // 设置目标APP
    params.target_app = app_id;

    // 更新CRC
    SblBootParams_UpdateCRC(&params);

    // 写回
    if (!SblBootParams_Write(&params))
    {
        LOG_ERROR("Failed to write SBL Boot Params\n");
        return false;
    }

    LOG_INFO("Target APP set to: APP%d\n", app_id);
    return true;
}

/*
 * 获取当前固件版本号
 */
uint32_t SblBootParams_GetCurrentVersion(void)
{
    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
        return 0;

    if (!SblBootParams_ValidateCRC(&params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return 0;
    }

    return params.header_version;
}

/*
 * 检查版本号是否允许升级
 * 返回: true=允许升级, false=不允许升级
 */
bool SblBootParams_CheckVersionUpgrade(uint32_t new_version)
{
    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
        return false;

    if (!SblBootParams_ValidateCRC(&params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return false;
    }

    // 如果版本号检查被禁用，则允许升级
    if (params.version_check_enable == 0)
    {
        LOG_INFO("Version check disabled, allow upgrade.\n");
        return true;
    }

    // 检查新版本号是否大于当前版本号
    if (new_version > params.header_version)
    {
        LOG_INFO("Version check passed: new=0x%08X > current=0x%08X\n",
                 new_version, params.header_version);
        return true;
    }
    else
    {
        LOG_ERROR("Version check failed: new=0x%08X <= current=0x%08X\n",
                  new_version, params.header_version);
        return false;
    }
}
