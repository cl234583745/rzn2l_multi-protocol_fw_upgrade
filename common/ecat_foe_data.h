/*
 * ecat_foe_data.h
 *
 *  Created on: 2026年2月12日
 *      Author: Jerry.Chen
 */
#include "flash_config.h"

#ifndef COMMON_ECAT_FOE_DATA_H_
#define COMMON_ECAT_FOE_DATA_H_

#define FW_UP_PACKAGE_SIZE      (116)// package send size

#define    HEADER_PARAMS_APP        4   // "APP1"
#define    HEADER_PARAMS_VERSION    4   // 4字节版本号

#pragma pack(1)
typedef struct
{
    uint8_t header_app[HEADER_PARAMS_APP];      // "APP"
    uint32_t header_version;                    // 固件版本号 (例如: 0x00010002 = v1.0.2)
    uint32_t header_len;                        // 固件总长度
    uint8_t header_reserved[SBL_BOOT_PARAMS_SIZE - HEADER_PARAMS_APP - HEADER_PARAMS_VERSION - 4];

    union
    {
        uint32_t  dword[4];                     // Vendor ID, Product Code, Revision, Serial
        uint16_t  word[8];
        uint8_t   byte[16];
    };

} app_header_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
    app_header_t current_header;
    uint8_t target_bank;            // 目标Bank (运行时确定)
    bool version_check_enabled;     // 版本号检查是否启用

    uint32_t current_addr;
    uint32_t current_crc;
    uint32_t recv_offset;
    uint32_t write_offset;
    uint8_t Read_Buffer[FW_UP_PAGE_SIZE];

} ota_handle_t;
#pragma pack()


#endif /* COMMON_ECAT_FOE_DATA_H_ */
