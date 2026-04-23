/*
 * qspi_utils.h
 *
 *  Created on: 2026年4月23日
 *      Author: Jerry.Chen
 *
 *  QSPI Flash工具函数 - 带超时保护
 */
#ifndef COMMON_QSPI_UTILS_H_
#define COMMON_QSPI_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal_data.h"
#include "log.h"

#define QSPI_ERASE_TIMEOUT_US    1000000   // 擦除超时：1秒
#define QSPI_WRITE_TIMEOUT_US   50000     // 写入超时：50ms

extern uint64_t getGlobalCounter(void);

bool QSPI_WaitEraseComplete(uint32_t timeout_us);
bool QSPI_WaitWriteComplete(uint32_t timeout_us);

#endif /* COMMON_QSPI_UTILS_H_ */