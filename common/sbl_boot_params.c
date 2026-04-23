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
#include "qspi_utils.h"
#include <string.h>

#define CURRENT_LOG_LEVEL   LOG_LEVEL_DEBUG

// CRC上下文
extern CRC_Context ctx;

/*
 * 初始化 SBL Boot Params
 * 启动决策逻辑:
 * 1. 读取并校验SBL Boot Params CRC
 * 2. 校验APP1 Bank0和Bank1的完整性
 * 3. 根据校验结果决定:
 *    - SBL有效 + 当前Bank有效 -> 正常启动
 *    - SBL有效 + 当前Bank无效 + 另一Bank有效 -> 回滚到另一Bank
 *    - SBL无效 + 有Bank有效 -> 重建SBL Boot Params
 *    - SBL无效 + 无Bank有效 -> 报错停止
 */
void SblBootParams_Init(void)
{
    sbl_boot_params_t boot_params;
    bool sbl_valid = false;
    bool bank0_valid = false;
    bool bank1_valid = false;
    app_header_t *header0 = NULL;
    app_header_t *header1 = NULL;

#if APP1_ENABLE
    // 1. 检查Bank0
    uint32_t bank0_addr = (uint32_t)APP1_BANK0_BASE_ADDR - FW_UP_MIRROR_OFFSET;
    header0 = (app_header_t *)bank0_addr;

    LOG_DEBUG("APP1_BANK0_BASE_ADDR=%08X %08X %08X\n", APP1_BANK0_BASE_ADDR, FW_UP_MIRROR_OFFSET, bank0_addr);

    if (memcmp(header0->header_app, APP1_STR, strlen(APP1_STR)) == 0)
    {
        if (header0->header_len > 4 && header0->header_len < 0x100000)
        {
            uint32_t crcCalRet = CRC_Calculate(&ctx, (char*)bank0_addr, (int)(header0->header_len - 4));
            uint32_t crc_flash;
            memcpy(&crc_flash, (uint8_t *)(bank0_addr + header0->header_len - 4), sizeof(uint32_t));

            LOG_DEBUG("Bank0: crcCalRet=%08X crc_flash=%08X calc_len=%d\n", crcCalRet, crc_flash, (int)(header0->header_len - 4));

            if (crcCalRet == crc_flash)
            {
                bank0_valid = true;
                LOG_INFO("Bank0 firmware valid: Version=0x%08X\n", header0->header_version);
            }
            else
            {
                LOG_ERROR("Bank0 CRC check failed! crcCalRet=%08X crc_flash=%08X\n", crcCalRet, crc_flash);
            }
        }
        else
        {
            LOG_ERROR("Bank0 header_len invalid: %d\n", header0->header_len);
        }
    }
    else
    {
        LOG_DEBUG("Bank0 header_app mismatch\n");
    }

    // 2. 检查Bank1
    uint32_t bank1_addr = (uint32_t)APP1_BANK1_BASE_ADDR - FW_UP_MIRROR_OFFSET;
    header1 = (app_header_t *)bank1_addr;

    LOG_DEBUG("APP1_BANK1_BASE_ADDR=%08X %08X %08X\n", APP1_BANK1_BASE_ADDR, FW_UP_MIRROR_OFFSET, bank1_addr);

    if (memcmp(header1->header_app, APP1_STR, strlen(APP1_STR)) == 0)
    {
        if (header1->header_len > 4 && header1->header_len < 0x100000)
        {   
            uint32_t crcCalRet = CRC_Calculate(&ctx, (char*)bank1_addr, (int)(header1->header_len - 4));
            uint32_t crc_flash;
            memcpy(&crc_flash, (uint8_t *)(bank1_addr + header1->header_len - sizeof(uint32_t)), sizeof(uint32_t));

            LOG_DEBUG("Bank1: crcCalRet=%08X crc_flash=%08X calc_len=%d\n", crcCalRet, crc_flash, (int)(header1->header_len - 4));

            if (crcCalRet == crc_flash)
            {
                bank1_valid = true;
                LOG_INFO("Bank1 firmware valid: Version=0x%08X\n", header1->header_version);
            }
            else
            {
                LOG_ERROR("Bank1 CRC check failed! crcCalRet=%08X crc_flash=%08X\n", crcCalRet, crc_flash);
            }
        }
        else
        {
            LOG_ERROR("Bank1 header_len invalid: %d\n", header1->header_len);
        }   
    }
    else
    {
        LOG_DEBUG("Bank1 header_app mismatch\n");   
    }
#endif

    // 3. 读取并校验SBL Boot Params
    if (SblBootParams_Read(&boot_params))
    {
        if (SblBootParams_ValidateCRC(&boot_params))
        {
            sbl_valid = true;
            LOG_INFO("SBL Boot Params CRC OK. Current Bank=%d\n", boot_params.f.current_bank);
        }
        else
        {
            LOG_ERROR("SBL Boot Params CRC invalid!\n");
        }
    }
    else
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
    }

    // 4. 根据SBL和APP状态决定启动策略
#if APP1_ENABLE
    uint8_t boot_bank = BANK_UNKNOWN;

    if (sbl_valid)
    {
        // SBL有效，根据当前Bank状态决定
        if ((boot_params.f.current_bank == BANK_0 && bank0_valid) ||
            (boot_params.f.current_bank == BANK_1 && bank1_valid))
        {
            // 当前Bank有效，正常启动
            LOG_INFO("Normal boot: Bank=%d valid\n", boot_params.f.current_bank);
        }
        else if (boot_params.f.current_bank == BANK_0 && !bank0_valid && bank1_valid)
        {
            // Bank0无效，尝试回滚到Bank1
            LOG_WARN("Bank0 invalid, rolling back to Bank1...\n");
            boot_params.f.current_bank = BANK_1;
            boot_params.f.target_bank = BANK_1;
            SblBootParams_UpdateCRC(&boot_params);
            SblBootParams_Write(&boot_params);
            LOG_INFO("Rollback to Bank1 complete\n");
        }
        else if (boot_params.f.current_bank == BANK_1 && !bank1_valid && bank0_valid)
        {
            // Bank1无效，尝试回滚到Bank0
            LOG_WARN("Bank1 invalid, rolling back to Bank0...\n");
            boot_params.f.current_bank = BANK_0;
            boot_params.f.target_bank = BANK_0;
            SblBootParams_UpdateCRC(&boot_params);
            SblBootParams_Write(&boot_params);
            LOG_INFO("Rollback to Bank0 complete\n");
        }
        else
        {
            // 当前Bank无效，另一Bank也无效
            LOG_ERROR("Both banks invalid! Cannot boot!\n");
            while(1) { }
            //TODO: 可以考虑进入DFU模式等待固件更新，或者重试机制
        }
    }
    else
    {
        // SBL无效，尝试从有效的Bank重建
        if (bank0_valid && bank1_valid)
        {
            // 两个Bank都有效，选择版本号更高的
            boot_bank = (header0->header_version >= header1->header_version) ? BANK_0 : BANK_1;
            LOG_INFO("SBL invalid, selecting higher version bank: Bank=%d\n", boot_bank);
        }
        else if (bank0_valid)
        {
            boot_bank = BANK_0;
            LOG_INFO("SBL invalid, selecting Bank0\n");
        }
        else if (bank1_valid)
        {
            boot_bank = BANK_1;
            LOG_INFO("SBL invalid, selecting Bank1\n");
        }
        else
        {
            // 没有有效的Bank
            LOG_ERROR("No valid firmware found in any bank!\n");
            while(1) { }
            //TODO: 可以考虑进入DFU模式等待固件更新，或者重试机制
        }

        // 重建SBL Boot Params
        sbl_boot_params_t new_params;
        memset(&new_params, 0, sizeof(sbl_boot_params_t));

        app_header_t *selected_header = (boot_bank == BANK_0) ? header0 : header1;

        memcpy(new_params.f.header_app, selected_header->header_app, strlen(APP1_STR));
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

        if (!SblBootParams_Write(&new_params))
        {
            LOG_ERROR("Failed to write SBL Boot Params!\n");
            while(1) { }
            //TODO: 可以考虑进入DFU模式等待固件更新，或者重试机制
        }

        LOG_INFO("SBL Boot Params reconstructed: Bank=%d, Version=0x%08X\n",
                    boot_bank, new_params.f.header_version);

        // 使用重建的参数
        memcpy(&boot_params, &new_params, sizeof(sbl_boot_params_t));
    }

    // 5. 输出最终启动信息
    LOG_INFO("=== Boot Info ===\n");
    LOG_INFO("  Target APP: APP%d\n", boot_params.f.target_app);
    LOG_INFO("  Current Bank: %d\n", boot_params.f.current_bank);
    LOG_INFO("  Target Bank: %d\n", boot_params.f.target_bank);
    LOG_INFO("  Firmware Version: 0x%08X\n", boot_params.f.header_version);
    LOG_INFO("  Version Check: %s\n", boot_params.f.version_check_enable ? "Enabled" : "Disabled");
    LOG_INFO("==================\n");

#else
    if (!sbl_valid)
    {
        LOG_ERROR("SBL Boot Params invalid and APP1 not enabled!\n");
        while(1) { }
        //TODO: 可以考虑进入DFU模式等待固件更新，或者重试机制
    }
#endif
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
	uint32_t sblBootParamsMirror = (uint32_t)SBL_BOOT_PARAMS_ADDR - FW_UP_MIRROR_OFFSET;

    LOG_DEBUG("SBL_BOOT_PARAMS_ADDR=%08X %08X %08X\n", SBL_BOOT_PARAMS_ADDR, FW_UP_MIRROR_OFFSET, sblBootParamsMirror);
    // 1. 先读取主区域
    memcpy(params, (void *)(sblBootParamsMirror), sizeof(sbl_boot_params_t));

    // 2. 验证 CRC
    if (SblBootParams_ValidateCRC(params))
    {
        return true;
    }

    // 3. 主区域无效，尝试读取备份区
    LOG_WARN("SBL Boot Params main region invalid, trying backup...\n");

    // 使用flash_config.h中定义的地址
	uint32_t sblBootParamsBackupMirror = (uint32_t)SBL_BOOT_PARAMS_ADDR_BACKUP - FW_UP_MIRROR_OFFSET;

	LOG_DEBUG("SBL_BOOT_PARAMS_ADDR_BACKUP=%08X %08X %08X\n", SBL_BOOT_PARAMS_ADDR_BACKUP, FW_UP_MIRROR_OFFSET, sblBootParamsBackupMirror);
	// 1. 先读取主区域
	memcpy(params, (void *)(sblBootParamsBackupMirror), sizeof(sbl_boot_params_t));

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

    // 使用flash_config.h中定义的地址
    uint32_t sblBootParams = (uint32_t)SBL_BOOT_PARAMS_ADDR;
	uint32_t sblBootParamsMirror = (uint32_t)SBL_BOOT_PARAMS_ADDR - FW_UP_MIRROR_OFFSET;

	LOG_DEBUG("SBL_BOOT_PARAMS_ADDR=%08X %08X %08X\n", SBL_BOOT_PARAMS_ADDR, FW_UP_MIRROR_OFFSET, sblBootParamsMirror);

    // 1. 先读取现有 Boot Params (用于失败恢复)
    sbl_boot_params_t old_params;
    bool has_backup = false;
    memcpy(&old_params, (void *)(sblBootParamsMirror), sizeof(sbl_boot_params_t));
    if (SblBootParams_ValidateCRC(&old_params))
    {
        has_backup = true;
    }

    // 2. 擦除主区域
    R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                      (uint8_t *)sblBootParams,
                      FW_UP_BOOT_PARAMS_SIZE);

    if (!QSPI_WaitEraseComplete(QSPI_ERASE_TIMEOUT_US))
    {
        return false;
    }

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
                          (uint8_t *)(sblBootParams + offset),
                          write_size);

        if (!QSPI_WaitWriteComplete(QSPI_WRITE_TIMEOUT_US))
        {
            return false;
        }

        offset += write_size;

        LOG_DEBUG("total_size=%08X offset=%08X write_size=%08X\n", total_size, offset, write_size);
    }

    // 4. 验证写入
    sbl_boot_params_t verify_params;
    memcpy(&verify_params, (void *)(sblBootParamsMirror), sizeof(sbl_boot_params_t));
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
                              (uint8_t *)sblBootParams,
                              FW_UP_BOOT_PARAMS_SIZE);

            if (!QSPI_WaitEraseComplete(QSPI_ERASE_TIMEOUT_US))
            {
                return false;
            }

            while (offset < total_size)
            {
                uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                                      FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

                R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                                  data + offset,
                                  (uint8_t *)(sblBootParams + offset),
                                  write_size);

                if (!QSPI_WaitWriteComplete(QSPI_WRITE_TIMEOUT_US))
                {
                    return false;
                }

                offset += write_size;
            }
        }

        return false;
    }

    // 5. 写入备份区
    // 使用flash_config.h中定义的地址
    uint32_t sblBootParamsBackup = (uint32_t)SBL_BOOT_PARAMS_ADDR_BACKUP;

    LOG_DEBUG("SBL_BOOT_PARAMS_ADDR_BACKUP=%08X %08X\n", SBL_BOOT_PARAMS_ADDR_BACKUP, sblBootParamsBackup);

    R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                      (uint8_t *)sblBootParamsBackup,
                      FW_UP_BOOT_PARAMS_SIZE);

    if (!QSPI_WaitEraseComplete(QSPI_ERASE_TIMEOUT_US))
    {
        return false;
    }

    offset = 0;
    data = (uint8_t *)params;

    while (offset < total_size)
    {
        uint32_t write_size = (total_size - offset) > FW_UP_WRITE_ATONCE_SIZE ?
                              FW_UP_WRITE_ATONCE_SIZE : (total_size - offset);

        R_XSPI_QSPI_Write(&g_qspi0_ctrl,
                          data + offset,
                          (uint8_t *)(sblBootParamsBackup + offset),
                          write_size);

        if (!QSPI_WaitWriteComplete(QSPI_WRITE_TIMEOUT_US))
        {
            return false;
        }

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
                                      sizeof(sbl_boot_params_fields_t) -sizeof(uint32_t));

    LOG_DEBUG("calc_crc=%08X calc_len=%d params->f.crc32==%08X \n", calc_crc, sizeof(sbl_boot_params_fields_t) -sizeof(uint32_t), params->f.crc32);

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
                                  sizeof(sbl_boot_params_fields_t) -sizeof(uint32_t));
}

/*
 * 获取目标启动的APP
 */
uint8_t SblBootParams_GetCurrentApp(void)
{
    sbl_boot_params_t params;

    if (!SblBootParams_Read(&params))
        return BANK_UNKNOWN;

    if (!SblBootParams_ValidateCRC(&params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed!\n");
        return BANK_UNKNOWN;
    }

    return params.f.target_app;
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
