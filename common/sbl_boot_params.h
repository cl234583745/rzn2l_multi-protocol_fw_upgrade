/*
 * sbl_boot_params.h
 *
 *  Created on: 2026年4月13日
 *      Author: Jerry.Chen
 *
 *  SBL Boot Params - SBL (Secondary Boot Loader) 启动参数
 *  注意: 此参数用于 SBL (二级引导)，区别于 ROM Boot (一级引导) 参数
 */

#ifndef COMMON_SBL_BOOT_PARAMS_H_
#define COMMON_SBL_BOOT_PARAMS_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * SBL Boot Params结构 (存储在0x600FF000, 大小4KB)
 * 用于记录 SBL 启动配置和升级选项
 * 注意: 此结构用于 SBL (二级引导)，区别于 ROM Boot (一级引导) 参数
 */
#pragma pack(1)
typedef struct
{
    // 固件标识 (9字节)
    uint8_t header_app[3];          // "APP"
    uint32_t header_version;        // 当前运行固件版本号

    // APP信息 (1字节) - 新增字段
    uint8_t target_app;             // 目标启动的APP: 1=app1, 2=app2, ..., 5=app5

    // Bank信息 (2字节)
    uint8_t current_bank;           // 当前运行的Bank: 0=Bank0, 1=Bank1
    uint8_t target_bank;            // 下次启动的Bank: 0=Bank0, 1=Bank1, 0xFF=自动

    // 配置选项 (1字节)
    uint8_t version_check_enable;   // 版本号检查使能: 1=启用, 0=禁用

    // Identify信息 (16字节)
    uint32_t vendor_id;             // Vendor ID
    uint32_t product_code;          // Product Code
    uint32_t revision_number;       // Revision Number
    uint32_t serial_number;         // Serial Number

    // CRC校验 (4字节)
    uint32_t crc32;                 // 整个结构的CRC32校验值

} sbl_boot_params_t;
#pragma pack()

// SBL Boot Params大小检查
#define SBL_BOOT_PARAMS_SIZE    (3 + 4 + 1 + 1 + 1 + 1 + 4*4 + 4)  // = 33字节

// SBL Boot Params API
void SblBootParams_Init(void);
bool SblBootParams_Read(sbl_boot_params_t *params);
bool SblBootParams_Write(const sbl_boot_params_t *params);
bool SblBootParams_ValidateCRC(const sbl_boot_params_t *params);
void SblBootParams_UpdateCRC(sbl_boot_params_t *params);

// 获取当前Bank
uint8_t SblBootParams_GetCurrentBank(void);

// 获取目标APP
uint8_t SblBootParams_GetTargetApp(void);

// 设置目标APP
bool SblBootParams_SetTargetApp(uint8_t app_id);

// 获取当前固件版本号
uint32_t SblBootParams_GetCurrentVersion(void);

// 检查版本号是否允许升级
bool SblBootParams_CheckVersionUpgrade(uint32_t new_version);

#endif /* COMMON_SBL_BOOT_PARAMS_H_ */
