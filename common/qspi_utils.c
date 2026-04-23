/*
 * qspi_utils.c
 *
 *  Created on: 2026年4月23日
 *      Author: Jerry.Chen
 *
 *  QSPI Flash工具函数 - 带超时保护
 */
#include "qspi_utils.h"

extern uint64_t getGlobalCounter(void);

bool QSPI_WaitEraseComplete(uint32_t timeout_us)
{
    spi_flash_status_t status;
    uint64_t start_time = getGlobalCounter();

    do {
        R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status);

        if ((getGlobalCounter() - start_time) > timeout_us)
        {
            LOG_ERROR("Flash erase timeout!\n");
            return false;
        }
    } while (status.write_in_progress);

    return true;
}

bool QSPI_WaitWriteComplete(uint32_t timeout_us)
{
    spi_flash_status_t status;
    uint64_t start_time = getGlobalCounter();

    do {
        R_XSPI_QSPI_StatusGet(&g_qspi0_ctrl, &status);

        if ((getGlobalCounter() - start_time) > timeout_us)
        {
            LOG_ERROR("Flash write timeout!\n");
            return false;
        }
    } while (status.write_in_progress);

    return true;
}