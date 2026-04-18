/*
 * sbl_params.c
 *
 *  Created on: 2026年2月12日
 *      Author: Jerry.Chen
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sbl_params.h"
#include "sbl_boot_params.h"
#include "bank_detection.h"
#include "flash_config.h"
#include "ecat_foe_data.h"
#include "crc32_table.h"
#include "hal_data.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_DEBUG

extern CRC_Context ctx;

/*
 * 读取 SBL Boot Params，检查CRC和与当前bank是否一致，不一致则恢复当前bank
 */
void checkAndUpdataBootBankParams(void)
{
    sbl_boot_params_t boot_params;
    sbl_boot_params_t new_params;

    // 1. 读取 SBL Boot Params
    if (!SblBootParams_Read(&boot_params))
    {
        LOG_ERROR("Failed to read SBL Boot Params!\n");
        return;
    }

    // 2. 验证CRC
    if (!SblBootParams_ValidateCRC(&boot_params))
    {
        LOG_ERROR("SBL Boot Params CRC check failed! Reconstructing...\n");

        // 3. 构造当前Bank的 SBL Boot Params
        memset(&new_params, 0, sizeof(sbl_boot_params_t));

        // 获取当前Bank
        uint8_t current_bank = BankDetection_GetCurrentBank();
        if (current_bank == BANK_UNKNOWN)
        {
            current_bank = BANK_0;  // 默认Bank0
        }

        // 读取当前Bank的固件头
        uint32_t bank_addr = BankDetection_GetBankAddress(current_bank);
        app_header_t *app_header = (app_header_t *)(bank_addr - FW_UP_MIRROR_OFFSET);

        // 填充Boot Params
        memcpy(new_params.header_app, app_header->header_app, 3);
        new_params.header_version = app_header->header_version;
        new_params.target_app = 1;  // 默认APP1
        new_params.current_bank = current_bank;
        new_params.target_bank = current_bank;
        new_params.version_check_enable = SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE;
        new_params.vendor_id = app_header->dword[0];
        new_params.product_code = app_header->dword[1];
        new_params.revision_number = app_header->dword[2];
        new_params.serial_number = app_header->dword[3];

        // 计算CRC
        SblBootParams_UpdateCRC(&new_params);

        // 4. 写入 SBL Boot Params
        if (!SblBootParams_Write(&new_params))
        {
            LOG_ERROR("Failed to write SBL Boot Params!\n");
            return;
        }

        LOG_INFO("SBL Boot Params reconstructed for Bank%d, Version=0x%08X\n",
                 current_bank, new_params.header_version);
    }
    else
    {
        LOG_INFO("SBL Boot Params OK: Bank=%d, Version=0x%08X\n",
                 boot_params.current_bank, boot_params.header_version);
    }
}

/*
 * SBL启动时检查 SBL Boot Params
 */
void sblCheckBootParams(void)
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
            LOG_ERROR("No valid firmware found in Bank0 or Bank1!\n");
            return;
        }

        // 5. 构造新的 SBL Boot Params
        sbl_boot_params_t new_params;
        memset(&new_params, 0, sizeof(sbl_boot_params_t));

        app_header_t *selected_header = (boot_bank == BANK_0) ? header0 : header1;

        memcpy(new_params.header_app, selected_header->header_app, 3);
        new_params.header_version = selected_header->header_version;
        new_params.target_app = 1;  // 默认APP1
        new_params.current_bank = boot_bank;
        new_params.target_bank = boot_bank;
        new_params.version_check_enable = SBL_BOOT_PARAMS_VERSION_CHECK_ENABLE;
        new_params.vendor_id = selected_header->dword[0];
        new_params.product_code = selected_header->dword[1];
        new_params.revision_number = selected_header->dword[2];
        new_params.serial_number = selected_header->dword[3];

        SblBootParams_UpdateCRC(&new_params);

        // 6. 写入 SBL Boot Params
        if (!SblBootParams_Write(&new_params))
        {
            LOG_ERROR("Failed to write SBL Boot Params!\n");
            return;
        }

        LOG_INFO("SBL Boot Params reconstructed: Bank=%d, Version=0x%08X, new_params.version_check_enable=%d\n",
                 boot_bank, new_params.header_version,new_params.version_check_enable);
#endif
    }
    else
    {
        LOG_INFO("SBL Boot Params OK: Bank=%d, Version=0x%08X, new_params.version_check_enable=%d\n",
                 boot_params.current_bank, boot_params.header_version,boot_params.version_check_enable);
    }
}
