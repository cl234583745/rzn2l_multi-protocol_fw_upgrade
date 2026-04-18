/*
 * progress.h
 *
 *  Created on: 2026年4月18日
 *      Author: Jerry.Chen
 */

#ifndef COMMON_PROGRESS_H_
#define COMMON_PROGRESS_H_

#include <stdint.h>
#include <stdbool.h>

void Progress_Init(uint32_t total_size);

void Progress_Update(uint32_t current_size);

void Progress_Finish(void);

void Progress_Print(uint32_t current, uint32_t total);

#endif /* COMMON_PROGRESS_H_ */