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
 * 读取 SBL Boot Params (带备份恢复)
 * 策略: 先读主区域，无效则读备份区
 */
bool SblBootParams_Read(sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    // 使用flash_config.h中定义的地址
    #define MAIN_PARAMS_ADDR   SBL_MAIN_PARAMS_ADDR
    #define BACKUP_PARAMS_ADDR SBL_BACKUP_PARAMS_ADDR

    // 1. 先读取主区域
    memcpy(params, (uint8_t *)MAIN_PARAMS_ADDR, sizeof(sbl_boot_params_t));

    // 2. 验证 CRC
    if (SblBootParams_ValidateCRC(params))
    {
        return true;
    }

    // 3. 主区域无效，尝试读取备份区
    LOG_WARN("SBL Boot Params main region invalid, trying backup...\n");
    memcpy(params, (uint8_t *)BACKUP_PARAMS_ADDR, sizeof(sbl_boot_params_t));

    if (SblBootParams_ValidateCRC(params))
    {
        LOG_INFO("SBL Boot Params restored from backup!\n");
        return true;
    }

    // 4. 都无效
    LOG_ERROR("SBL Boot Params both main and backup invalid!\n");
    return false;
}

/*
 * 写入 SBL Boot Params (带备份机制)
 *
 * 策略:
 * 1. 先读取现有 Boot Params 作为备份 (从备份区)
 * 2. 擦除主区域
 * 3. 写入新的 Boot Params 到主区域
 * 4. 验证写入
 * 5. 写入备份区
 */
bool SblBootParams_Write(const sbl_boot_params_t *params)
{
    if (params == NULL)
        return false;

    spi_flash_status_t status_erase;

    // 使用flash_config.h中定义的地址
    #define MAIN_PARAMS_ADDR   SBL_MAIN_PARAMS_ADDR
    #define BACKUP_PARAMS_ADDR SBL_BACKUP_PARAMS_ADDR

    // 1. 先读取现有 Boot Params (用于失败恢复)
    sbl_boot_params_t old_params;
    bool has_backup = false;
    memcpy(&old_params, (uint8_t *)MAIN_PARAMS_ADDR, sizeof(sbl_boot_params_t));
    if (SblBootParams_ValidateCRC(&old_params))
    {
        has_backup = true;
    }

    // 2. 擦除主区域
    R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                      (uint8_t *)MAIN_PARAMS_ADDR,
                      FW_UP_BOOT_PARAMS_SIZE);

    do {
        R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
    } while (status_erase.write_in_progress);

    // 3. 写入新的 Boot Params 到主区域 (分多次写入，每次64字节)
    uint8_t *data = (uint8_t *)params;
    uint32_t total_size = sizeof(sbl_boot_params_t);
    uint32_t offset = 0;

    while (offset < total_size)
    {
        uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                              FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

        R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                          data + offset,
                          (uint8_t *)(MAIN_PARAMS_ADDR + offset),
                          write_size);

        do {
            R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
        } while (status_erase.write_in_progress);

        offset += write_size;
    }

    // 4. 验证写入
    sbl_boot_params_t verify_params;
    memcpy(&verify_params, (uint8_t *)MAIN_PARAMS_ADDR, sizeof(sbl_boot_params_t));
    if (!SblBootParams_ValidateCRC(&verify_params))
    {
        LOG_ERROR("SblBootParams_Write: Verify failed!\n");

        // 尝试恢复旧数据
        if (has_backup)
        {
            LOG_INFO("SblBootParams_Write: Restoring old params...\n");
            offset = 0;
            data = (uint8_t *)&old_params;

            R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                              (uint8_t *)MAIN_PARAMS_ADDR,
                              FW_UP_BOOT_PARAMS_SIZE);

            do {
                R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
            } while (status_erase.write_in_progress);

            while (offset < total_size)
            {
                uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                                      FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

                R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                                  data + offset,
                                  (uint8_t *)(MAIN_PARAMS_ADDR + offset),
                                  write_size);

                do {
                    R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
                } while (status_erase.write_in_progress);

                offset += write_size;
            }
        }

        return false;
    }

    // 5. 写入备份区
    R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                      (uint8_t *)BACKUP_PARAMS_ADDR,
                      FW_UP_BOOT_PARAMS_SIZE);

    do {
        R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
    } while (status_erase.write_in_progress);

    offset = 0;
    data = (uint8_t *)params;

    while (offset < total_size)
    {
        uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                              FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

        R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                          data + offset,
                          (uint8_t *)(BACKUP_PARAMS_ADDR + offset),
                          write_size);

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
