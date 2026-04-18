/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
* https://www.beckhoff.com/media/downloads/slave-stack-code/ethercat_ssc_license.pdf
*/

/**
\addtogroup ESM EtherCAT State Machine
@{
*/

/**
\file bootmode.c
\author EthercatSSC@beckhoff.com
\brief Implementation

\version 5.12

<br>Changes to version V4.20:<br>
V5.12 BOOT2: call BL_Start() from Init to Boot<br>
<br>Changes to version - :<br>
V4.20: File created
*/

/*--------------------------------------------------------------------------------------
------
------    Includes
------
--------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include "ecatfoe.h"
#include "ecat_def.h"
#include "ecatslv.h"
#include "mailbox.h"
#include "ecatappl.h"
#include "foeappl.h"
#include "sampleappl.h"
#include "renesashw.h"
#include "bootmode.h"




#include "flash_config.h"
#include "circular_queue.h"
#include "crc32_table.h"
#include "bsp_r52_global_counter.h"
#include "sbl_params.h"
#include "ecat_foe_data.h"
#include "sbl_boot_params.h"
#include "bank_detection.h"
#include "progress.h"
#include "log.h"

#define CURRENT_LOG_LEVEL   LOG_LEVEL_DEBUG

#define    DEBUG                    1



static ota_handle_t ota_handle = {0};
static volatile uint64_t beginTime = 0;
static volatile uint64_t endTime = 0;
CRC_Context ctx;
static BOOL   bReBoot;

// ============================================================================
// APP1固件版本号定义 (易读、易修改、易比较)
// ============================================================================
// 格式: Major.Minor.Patch (语义化版本规范)
// 例如: 1.2.3 表示主版本1，次版本2，补丁版本3
// 每个字段范围: 0-255，足够使用
// 
// 修改方法: 直接修改下面的数字即可
// 例如: 发布v2.1.0版本，修改为:
//       #define APP1_VERSION_MAJOR  2
//       #define APP1_VERSION_MINOR  1
//       #define APP1_VERSION_PATCH  0
// ============================================================================
#ifndef APP1_VERSION_MAJOR
#define APP1_VERSION_MAJOR  1
#endif

#ifndef APP1_VERSION_MINOR
#define APP1_VERSION_MINOR  0
#endif

#ifndef APP1_VERSION_PATCH
#define APP1_VERSION_PATCH  0
#endif

// 自动组合版本号 (无需手动修改)
// 格式: 0x00MMmmpp (MM=Major, mm=Minor, pp=Patch)
// 例如: v1.2.3 -> 0x00010203
#define APP1_VERSION ((APP1_VERSION_MAJOR << 16) | (APP1_VERSION_MINOR << 8) | APP1_VERSION_PATCH)

BSP_DONT_REMOVE const uint8_t g_header[HEADER_PARAMS_LENS] BSP_PLACE_IN_SECTION(".header") = {
    'A', 'P', 'P',  // "APP"
    (APP1_VERSION >> 0) & 0xFF,  // Patch
    (APP1_VERSION >> 8) & 0xFF,  // Minor
    (APP1_VERSION >> 16) & 0xFF, // Major
    (APP1_VERSION >> 24) & 0xFF  // 保留(0x00)
};

BSP_DONT_REMOVE const uint32_t g_identify[4] BSP_PLACE_IN_SECTION(".identify") = {(VENDOR_ID), (PRODUCT_CODE), (REVISION_NUMBER), (SERIAL_NUMBER)};


static void norFlashPageProgram(uint8_t* addr, uint8_t* data, uint16_t len);
/*
 * page program 256Byte
 *
 */
static void norFlashPageProgram(uint8_t* addr, uint8_t* data, uint16_t len)
{
    (void)len;

    spi_flash_status_t status_erase;
    for ( uint8_t i = 0; i < FW_UP_PAGE_SIZE / FW_UP_WRITE_ATONCE_SIZE ; i++)
    {
        R_XSPI_QSPI_Write(&g_qspi0_ctrl, &data[0 + (i * FW_UP_WRITE_ATONCE_SIZE)],
                          (uint8_t *)(addr + (i * FW_UP_WRITE_ATONCE_SIZE) - FW_UP_MIRROR_OFFSET),
                          (uint32_t)FW_UP_WRITE_ATONCE_SIZE);
        do
        {
            (void) R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
        } while (true == status_erase.write_in_progress);
    }
}

void test_byte_array(void);
void test_byte_array(void) {
//    printf("Test 5 - Byte array {0x00, 0x01, 0x02, 0x03, 0x04}:\n");
//    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
//    //uint32_t crc = calculate_crc32(data, sizeof(data), 0xFFFFFFFF, 0xFFFFFFFF);
//    printf("  CRC32 = 0x%08X\n", crc);
//    printf("  Expected: 0xB63CFBCD\n");
//    printf("  Result: %s\n\n", (crc == 0xB63CFBCD) ? "PASS" : "FAIL");
}

void BL_Start( UINT8 State)
{
    (void)State;

    CRC_Init(&ctx, 0xFFFFFFFF, 0xEDB88320);  // 标准CRC32参数

    // 初始化Bank检测
    BankDetection_Init();

    // 初始化Boot Params
    SblBootParams_Init();

#if 0//test crc32
    LOG_DEBUG("=== CRC32 Test ===\n\n");

    // 测试数据
    const char* test_str = "123456789";
    const uint8_t test_bytes[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    LOG_DEBUG("1. Using CRC_Context:\n");
    uint32_t crc1 = CRC_Calculate(&ctx, test_str, strlen(test_str));
    LOG_DEBUG("   CRC32 of \"123456789\": 0x%08X (Expected: 0xCBF43926)\n", crc1);
#endif

    memset(&ota_handle, 0 , sizeof(ota_handle_t));
    Queue_Init((Circular_queue_t*)&Circular_queue);

    // 设置版本号检查选项 (从Boot Params读取)
    sbl_boot_params_t sbl_boot_params;
    if (SblBootParams_Read(&sbl_boot_params) && SblBootParams_ValidateCRC(&sbl_boot_params))
    {
        ota_handle.version_check_enabled = (sbl_boot_params.version_check_enable != 0);
    }
    else
    {
        // 默认启用版本号检查
        ota_handle.version_check_enabled = true;
    }

#if DEBUG
    LOG_INFO("%s State=%d, VersionCheck=%d\n", __FUNCTION__, State, ota_handle.version_check_enabled);
#endif
#if 0
    char buffer[] = "BL_Start\n";
    R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t*) &buffer, 9);
#endif
}

void BL_Stop(void)
{
#if DEBUG
    LOG_INFO("%s\n", __FUNCTION__);
#endif
}



void BL_StartDownload(UINT32 password)
{
    (void)password;

    beginTime =  getGlobalCounter();

    // 获取当前Bank
    uint8_t current_bank = BankDetection_GetCurrentBank();
    if (current_bank == BANK_UNKNOWN)
    {
        LOG_ERROR("BL_StartDownload: Unknown current bank!\n");
        return;
    }

    // 确定目标Bank (将在BL_Data中根据固件头确定)
    ota_handle.target_bank = BANK_UNKNOWN;

    LOG_INFO("BL_StartDownload: Current Bank=%d, waiting for firmware header...\n", current_bank);

    // 暂不擦除Flash，等收到固件头并验证后再擦除
}

void Progress_Init_Callback(uint32_t total_size)
{
    Progress_Init(total_size);
}

UINT16 BL_Data(UINT16 *pData,UINT16 Size)
{
    UINT16 ErrorCode = 0;
    uint8_t current_bank = BankDetection_GetCurrentBank();

    // 首次接收: 解析固件头并验证
    if(ota_handle.recv_offset == 0)
    {
        app_header_t *fw_header = (app_header_t *)pData;

        // 1. 验证固件魔数 "APP"
        if (memcmp(fw_header->header_app, "APP", 3) != 0)
        {
            LOG_ERROR("Invalid firmware magic! Expected 'APP', got '%c%c%c'\n",
                      fw_header->header_app[0], fw_header->header_app[1], fw_header->header_app[2]);
            return FOE_ERROR;
        }

        // 2. 保存固件头
        memcpy(&ota_handle.current_header, fw_header, sizeof(app_header_t));

        LOG_INFO("Firmware Header: Version=0x%08X, TargetBank=%d, Len=%ld\n",
                 fw_header->header_version,
                 fw_header->header_target_bank,
                 fw_header->header_len);

        // 3. 版本号检查
        if (ota_handle.version_check_enabled)
        {
            if (!SblBootParams_CheckVersionUpgrade(fw_header->header_version))
            {
                LOG_ERROR("Version check failed! Upgrade rejected.\n");
                return FOE_ERROR;
            }
        }
        else
        {
            LOG_INFO("Version check disabled, skip version validation.\n");
        }

        // 4. 确定目标Bank
        if (fw_header->header_target_bank == BANK_UNKNOWN)
        {
            // 自动模式: 写入另一个Bank
            ota_handle.target_bank = BankDetection_GetTargetBank(current_bank);
        }
        else
        {
            // 强制模式: 检查有效性
            if (!BankDetection_IsTargetBankValid(fw_header->header_target_bank, current_bank))
            {
                LOG_ERROR("Invalid target bank! current=%d, target=%d\n",
                          current_bank, fw_header->header_target_bank);
                return FOE_ERROR;
            }
            ota_handle.target_bank = fw_header->header_target_bank;
        }

        LOG_INFO("Firmware upgrade: Bank%d -> Bank%d\n", current_bank, ota_handle.target_bank);

        // 5. 擦除目标Bank
        uint32_t target_addr = BankDetection_GetBankAddress(ota_handle.target_bank);
        LOG_INFO("Erasing Bank%d at 0x%08X...\n", ota_handle.target_bank, target_addr);

        for(uint8_t i = 0; i < FW_UP_TOTAL_SIZE/FW_UP_SECTOR_SIZE; i++)
        {
            R_XSPI_QSPI_Erase(&g_qspi0_ctrl,
                              (uint8_t *)(target_addr + (i * FW_UP_SECTOR_SIZE)),
                              FW_UP_SECTOR_SIZE);

            spi_flash_status_t status_erase;
            volatile uint16_t dummy16;
            do
            {
                R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status_erase);
                HW_EscReadWord(dummy16, ESC_EEPROM_CONFIG_OFFSET);
                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
                (void)dummy16;
            } while (status_erase.write_in_progress);
        }

        LOG_INFO("Bank%d erased successfully!\n", ota_handle.target_bank);

        // 初始化进度条
        Progress_Init(ota_handle.current_header.header_len);
    }

    // 写入数据到环形队列
    Queue_Wirte(&Circular_queue, (uint8_t*) ((uint8_t*)pData), Size);

    // 队列满一页时写入Flash
    if(Queue_HadUse(&Circular_queue) >= FW_UP_PAGE_SIZE)
    {
        Queue_Read(&Circular_queue, ota_handle.Read_Buffer, FW_UP_PAGE_SIZE);

        uint32_t target_addr = BankDetection_GetBankAddress(ota_handle.target_bank);
        norFlashPageProgram((uint8_t *)(target_addr + ota_handle.write_offset),
                            ota_handle.Read_Buffer,
                            FW_UP_PAGE_SIZE);
        ota_handle.write_offset += FW_UP_PAGE_SIZE;
    }

    // 1 to n-1 times: 更新进度条
    if((ota_handle.recv_offset > 0) && (ota_handle.current_header.header_len - ota_handle.recv_offset > FW_UP_PACKAGE_SIZE))
    {
        //LOG_INFO("P:%ld%% ", ota_handle.write_offset*100/ota_handle.current_header.header_len);
    }
    // n last time and lastDataLen < 256
    else if((ota_handle.recv_offset > 0) && (ota_handle.current_header.header_len - ota_handle.recv_offset <= FW_UP_PACKAGE_SIZE))
    {
        uint16_t lastDataLen = Queue_HadUse(&Circular_queue);

        Queue_Read(&Circular_queue, ota_handle.Read_Buffer, lastDataLen);

        uint32_t target_addr = BankDetection_GetBankAddress(ota_handle.target_bank);
        norFlashPageProgram((uint8_t *)(target_addr + ota_handle.write_offset),
                            ota_handle.Read_Buffer,
                            lastDataLen);
        ota_handle.write_offset += lastDataLen;

        //LOG_INFO("Last P:%ld%%\n", ota_handle.write_offset*100/ota_handle.current_header.header_len);
        //LOG_INFO("Write flash finished!!!\n");
    }

    // 更新进度条
    Progress_Update(ota_handle.write_offset);

    // 升级完成: CRC校验 + 更新SII + 更新Boot Params
    if(ota_handle.write_offset >= ota_handle.current_header.header_len)
    {
        // 1. CRC校验
        uint32_t target_addr = BankDetection_GetBankAddress(ota_handle.target_bank);
        uint32_t mirror_addr = target_addr - FW_UP_MIRROR_OFFSET;

        uint32_t crcCalRet = CRC_Calculate(&ctx, (char*)mirror_addr, (int)(ota_handle.current_header.header_len - 4));

        uint32_t crc_flash;
        memcpy(&crc_flash, (uint8_t *)(mirror_addr + ota_handle.current_header.header_len - 4), sizeof(uint32_t));

        if(crcCalRet != crc_flash)
        {
            LOG_ERROR("CRC check failed! calc=0x%lX, flash=0x%lX\n", crcCalRet, crc_flash);
            return FOE_ERROR;
        }
        else
        {
            LOG_INFO("CRC check passed!\n");
        }

        // 2. 更新SII (Slave Information Interface)
        EEPBUFFER Buffer;
        for (uint8_t i = 0; i < 4; i++)
        {
            Buffer.dword[i] = ota_handle.current_header.dword[i];
        }
        ESC_EepromAccess(SII_EEP_IDENTIFY_OFFSET + SII_EEP_REVESIONNO, 2, &Buffer.word[SII_EEP_REVESIONNO], ESC_WR);
        LOG_INFO("SII updated: Revision=0x%04X\n", Buffer.word[SII_EEP_REVESIONNO]);

        // 3. 更新Boot Params
        sbl_boot_params_t sbl_boot_params;
        memset(&sbl_boot_params, 0, sizeof(sbl_boot_params_t));

        // 填充Boot Params
        memcpy(sbl_boot_params.header_app, "APP", 3);
        sbl_boot_params.header_version = ota_handle.current_header.header_version;
        sbl_boot_params.target_app = 1;  // APP1
        sbl_boot_params.current_bank = ota_handle.target_bank;  // 下次启动的Bank
        sbl_boot_params.target_bank = ota_handle.target_bank;
        sbl_boot_params.version_check_enable = ota_handle.version_check_enabled ? 1 : 0;
        sbl_boot_params.vendor_id = ota_handle.current_header.dword[0];
        sbl_boot_params.product_code = ota_handle.current_header.dword[1];
        sbl_boot_params.revision_number = ota_handle.current_header.dword[2];
        sbl_boot_params.serial_number = ota_handle.current_header.dword[3];

        // 计算并设置CRC
        SblBootParams_UpdateCRC(&sbl_boot_params);

        // 写入Boot Params
        if (!SblBootParams_Write(&sbl_boot_params))
        {
            LOG_ERROR("Failed to write Boot Params!\n");
            return FOE_ERROR;
        }

        LOG_INFO("Boot Params updated: Bank=%d, Version=0x%08X\n",
                 sbl_boot_params.current_bank, sbl_boot_params.header_version);

        // 4. 完成进度条
        Progress_Finish();

        // 5. 设置重启标志
        BL_SetRebootFlag(TRUE);

        // 6. 记录升级时间
        endTime = getGlobalCounter();
        LOG_INFO("Time: %ldms\n",
                 (uint32_t)((endTime - beginTime) * 1000 / BSP_GLOBAL_SYSTEM_COUNTER_CLOCK_HZ));

        return 0;
    }

    ota_handle.recv_offset += Size;
    return(ErrorCode);
}

void BL_SetRebootFlag(BOOL Flag)
{
    bReBoot = Flag;
}

BOOL BL_CheckRebootFlag(void)
{
    return(bReBoot);
}


void BL_Reboot(void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_RESET);
    R_BSP_SystemReset();    // System Software Reset
    while(1)
    {
        /* Do nothing */
    };
}


