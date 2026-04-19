/***********************************************************************************************************************
 * Copyright [2020-2022] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO
 * THE SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * THE DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/
#ifndef LOADER_TABLE_H_
#define LOADER_TABLE_H_

#include "bsp_api.h"


/* Value for loader_table */
#define TABLE_ENTRY_NUM                  (10)
#define TABLE_ENABLE                     (1)
#define TABLE_DISABLE                    (0)
#define TABLE_INVALID_VALUE              (0xffffffff)


/* loader_table_t definition (64 bytes per entry) */
#define LOADER_TABLE_SIZE  64

typedef struct __attribute__((packed)) {
    uint32_t * src;
    uint32_t * dst;
    uint32_t size;
    uint32_t enable_flag;
    uint8_t app_id;
    uint8_t bank_id;
    uint8_t is_dual_bank;
} loader_table_fields_t;

typedef struct __attribute__((packed)) {
    loader_table_fields_t f;
    uint8_t reserved[LOADER_TABLE_SIZE - sizeof(loader_table_fields_t)];
} loader_table_t;

/* GCC/Clang 静态断言 */
_Static_assert(sizeof(loader_table_t) == LOADER_TABLE_SIZE, "loader_table_t must be 64 bytes");

#endif /* LOADER_TABLE_H_ */
