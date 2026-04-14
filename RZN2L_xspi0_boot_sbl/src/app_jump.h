/*
 * app_jump.h
 *
 *  Created on: 2026年4月14日
 *      Author: Jerry.Chen
 *
 *  APP Jump Logic - APP 跳转逻辑模块
 *  用于处理APP的跳转和启动 (仅SBL使用)
 */

#ifndef RZN2L_XSPI0_BOOT_SBL_SRC_APP_JUMP_H_
#define RZN2L_XSPI0_BOOT_SBL_SRC_APP_JUMP_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * APP跳转结果
 */
typedef enum {
    APP_JUMP_SUCCESS = 0,
    APP_JUMP_FAIL_INVALID_APP,
    APP_JUMP_FAIL_INVALID_BANK,
    APP_JUMP_FAIL_NOT_ENABLED,
    APP_JUMP_FAIL_LOAD_ERROR,
    APP_JUMP_FAIL_VALIDATION_ERROR,
    APP_JUMP_FAIL_NO_VALID_BANK
} app_jump_result_t;

/*
 * APP跳转信息
 */
typedef struct {
    uint8_t app_id;             // APP ID
    uint8_t bank_id;            // Bank ID
    uint32_t src_addr;          // Flash源地址
    uint32_t dst_addr;          // SRAM目标地址
    uint32_t entry_point;       // 入口点地址
    uint32_t image_size;        // 镜像大小
    bool is_valid;              // 是否有效
} app_jump_info_t;

/*
 * APP Jump API
 */

// 初始化APP跳转模块
void AppJump_Init(void);

// 跳转到指定APP的指定Bank
app_jump_result_t AppJump_ToApp(uint8_t app_id, uint8_t bank_id);

// 跳转到指定APP (自动选择Bank)
app_jump_result_t AppJump_ToAppAuto(uint8_t app_id);

// 跳转到下一个APP (用于多APP轮转)
app_jump_result_t AppJump_ToNextApp(void);

// 获取APP跳转信息
bool AppJump_GetInfo(uint8_t app_id, uint8_t bank_id, app_jump_info_t *info);

// 验证APP镜像
bool AppJump_ValidateImage(uint8_t app_id, uint8_t bank_id);

// 获取跳转结果描述字符串
const char* AppJump_GetResultString(app_jump_result_t result);

#endif /* RZN2L_XSPI0_BOOT_SBL_SRC_APP_JUMP_H_ */
