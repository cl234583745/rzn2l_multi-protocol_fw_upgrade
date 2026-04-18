/*
 * progress.c
 *
 *  Created on: 2026年4月18日
 *      Author: Jerry.Chen
 */

#include "progress.h"
#include "log.h"

#define PROGRESS_BAR_LENGTH  50

static uint32_t g_total_size = 0;
static uint32_t g_last_percent = 0xFFFFFFFF;

void Progress_Init(uint32_t total_size)
{
    g_total_size = total_size;
    g_last_percent = 0xFFFFFFFF;

    LOG_RAW("\r\n[");
    for (uint8_t i = 0; i < PROGRESS_BAR_LENGTH; i++)
    {
        LOG_RAW("-");
    }
    LOG_RAW("] 0%%\r");
}

void Progress_Update(uint32_t current_size)
{
    if (g_total_size == 0)
        return;

    uint32_t percent = (current_size * 100) / g_total_size;

    if (percent == g_last_percent)
        return;

    g_last_percent = percent;

    uint8_t bar_len = (uint8_t)((percent * PROGRESS_BAR_LENGTH) / 100);
    uint8_t empty_len = PROGRESS_BAR_LENGTH - bar_len;

    LOG_RAW("\r[");
    for (uint8_t i = 0; i < bar_len; i++)
    {
        LOG_RAW("=");
    }
    for (uint8_t i = 0; i < empty_len; i++)
    {
        LOG_RAW("-");
    }
    LOG_RAW("] %lu%%", percent);
}

void Progress_Finish(void)
{
    g_last_percent = 100;
    LOG_RAW("\r[");
    for (uint8_t i = 0; i < PROGRESS_BAR_LENGTH; i++)
    {
        LOG_RAW("=");
    }
    LOG_RAW("] 100%%\n");
    LOG_RAW("Upgrade completed!\n");
}

void Progress_Print(uint32_t current, uint32_t total)
{
    if (total == 0)
        return;

    uint32_t percent = (current * 100) / total;

    if (percent == g_last_percent)
        return;

    g_last_percent = percent;

    uint8_t bar_len = (uint8_t)((percent * PROGRESS_BAR_LENGTH) / 100);
    uint8_t empty_len = PROGRESS_BAR_LENGTH - bar_len;

    LOG_RAW("\r[");
    for (uint8_t i = 0; i < bar_len; i++)
    {
        LOG_RAW("=");
    }
    for (uint8_t i = 0; i < empty_len; i++)
    {
        LOG_RAW("-");
    }
    LOG_RAW("] %lu%%", percent);

    if (percent >= 100)
    {
        LOG_RAW("\n");
    }
}