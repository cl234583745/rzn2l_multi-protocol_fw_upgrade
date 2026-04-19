/*
 * sbl_boot_params.c
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 *
 *  SBL Boot Params - SBL (Secondary Boot Loader) 启动参数管理
 *  注意: 此模块用于 SBL (二级引导)，区别于 ROM Boot (一级引导)
 */
#include "ecat_foe_data.h"
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
    sbl_boot_params_t boot_params;

    // 1. 读取 SBL Boot Params
    if (!SblBootParams_Read(&boot_params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return;
    }

    // 2. 验证CRC
    if (!SblBootParams_ValidateCRC(&boot_params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");

#if APP1_ENABLE
        // 3. 检查Bank0和Bank1的固件有效性
        bool bank0_valid = false;
        bool bank1_valid = false;

        // 检查Bank0
        uint32_t bank0_addr = FW_UP_BANK0_ADDR - FW_UP_MIRROR_OFFSET;
        app_header_t *header0 = (app_header_t *)bank0_addr;

        if (memcmp(header0->header_app, "APP", 3) == 0)
        {
            uint32_t crcCalRet = CRC_Calculate(&ctx, (char*)bank0_addr, (int)(header0->header_len - 4));

            uint32_t crc_flash;
            memcpy(&crc_flash, (uint8_t *)(bank0_addr + header0->header_len - 4), sizeof(uint32_t));

            if (crcCalRet == crc_flash)
            {
                bank0_valid = true;
                LOG_INFO("Bank0 firmware valid: Version=0x%08X\n", header0->header_version);
            }
            else
            {
                LOG_ERROR("Bank0 CRC check failed!\n");
            }
        }

        // 检查Bank1
        uint32_t bank1_addr = FW_UP_BANK1_ADDR - FW_UP_MIRROR_OFFSET;
        app_header_t *header1 = (app_header_t *)bank1_addr;

        if (memcmp(header1->header_app, "APP", 3) == 0)
        {
            uint32_t crcCalRet = CRC_Calculate(&ctx, (char*)bank1_addr, (int)(header1->header_len - 4));

            uint32_t crc_flash;
            memcpy(&crc_flash, (uint8_t *)(bank1_addr + header1->header_len - 4), sizeof(uint32_t));

            if (crcCalRet == crc_flash)
            {
                bank1_valid = true;
                LOG_INFO("Bank1 firmware valid: Version=0x%08X\n", header1->header_version);
            }
            else
            {
                LOG_ERROR("Bank1 CRC check failed!\n");
            }
        }

        // 4. 选择有效的Bank
        uint8_t boot_bank = BANK_UNKNOWN;
        if (bank0_valid && bank1_valid)
        {
            // 两个Bank都有效，选择版本号更高的
            boot_bank = (header0->header_version >= header1->header_version) ? BANK_0 : BANK_1;
        }
        else if (bank0_valid)
        {
            boot_bank = BANK_0;
        }
        else if (bank1_valid)
        {
            boot_bank = BANK_1;
        }
        else
        {
            LOG_ERROR("APP1 No valid firmware found in Bank0 or Bank1!\n");
            LOG_ERROR("APP1 No valid firmware found in Bank0 or Bank1!\n");
            LOG_ERROR("APP1 No valid firmware found in Bank0 or Bank1!\n");

        }

        if (bank0_valid || bank1_valid)
        {
            // 5. 构造新的 SBL Boot Params
            sbl_boot_params_t new_params;
            memset(&new_params, 0, sizeof(sbl_boot_params_t));

            app_header_t *selected_header = (boot_bank == BANK_0) ? header0 : header1;

            memcpy(new_params.f.header_app, selected_header->header_app, 3);
            new_params.f.header_version = selected_header->header_version;
            new_params.f.target_app = APP1_ID;
            new_params.f.current_bank = boot_bank;
            new_params.f.target_bank = boot_bank;
            new_params.f.version_check_enable = SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE;
            new_params.f.vendor_id = selected_header->dword[0];
            new_params.f.product_code = selected_header->dword[1];
            new_params.f.revision_number = selected_header->dword[2];
            new_params.f.serial_number = selected_header->dword[3];

            SblBootParams_UpdateCRC(&new_params);

            // 6. 写入 SBL Boot Params
            if (!SblBootParams_Write(&new_params))
            {
                LOG_ERROR("Failed to write SBL Boot Params!\n");
                return;
            }

            LOG_INFO("SBL Boot Params reconstructed: Bank=%d, Version=0x%08X, new_params.f.version_check_enable=%d\n",
                     boot_bank, new_params.f.header_version,new_params.f.version_check_enable);
        }
#endif//#if APP1_ENABLE

#if APP2_ENABLE

#endif//#if APP2_ENABLE

#if APP3_ENABLE

#endif//#if APP3_ENABLE

#if APP4_ENABLE

#endif//#if APP4_ENABLE

#if APP5_ENABLE

#endif//#if APP5_ENABLE

    }
    else
    {
        LOG_INFO("SBL Boot Params OK: Bank=%d, Version=0x%08X, new_params.f.version_check_enable=%d\n",
                 boot_params.f.current_bank, boot_params.f.header_version,boot_params.f.version_check_enable);
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

    return (calc_crc == params->f.crc32);
}

/*
 * 更新 SBL Boot Params 的 CRC
 */
void SblBootParams_UpdateCRC(sbl_boot_params_t *params)
{
    if (params == NULL)
        return;

    // 计算CRC (不包括最后的crc32字段)
    params->f.crc32 = CRC_Calculate(&ctx,
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

    return params.f.current_bank;
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

    return params.f.header_version;
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
    if (params.f.version_check_enable == 0)
    {
        LOG_INFO("Version check disabled, allow upgrade.\n");
        return true;
    }

    // 检查新版本号是否大于当前版本号
    if (new_version > params.f.header_version)
    {
        LOG_INFO("Version check passed: new=0x%08X > current=0x%08X\n",
                 new_version, params.f.header_version);
        return true;
    }
    else
    {
        LOG_ERROR("Version check failed: new=0x%08X <= current=0x%08X\n",
                  new_version, params.f.header_version);
        return false;
    }
}
