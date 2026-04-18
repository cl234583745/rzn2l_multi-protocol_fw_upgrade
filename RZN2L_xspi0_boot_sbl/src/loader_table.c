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
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/
#include "loader_table.h"
#include "flash_config.h"




/*
 * loader_table = { Src, Dst, Size, Enable flag(enable:1 disable:0), app_id, bank_id, reserved }
 * table0: Load Application program to System SRAM from flash memory
 * table1: Disable loader_table
 * table2: Disable loader_table
 * table3: Disable loader_table
 */
const loader_table_t loader_table[TABLE_ENTRY_NUM] BSP_PLACE_IN_SECTION("CPU0_LOADER_TABLE") =
{
  { (uint32_t *)app1_bank0_prg_flash_addr, (uint32_t *)app1_bank0_prg_start_addr, (uint32_t)app1_bank0_prg_size, (uint32_t)TABLE_ENABLE, 1, 0, {0, 0} },
  { (uint32_t *)app1_bank1_prg_flash_addr, (uint32_t *)app1_bank1_prg_start_addr, (uint32_t)app1_bank1_prg_size, (uint32_t)TABLE_ENABLE, 1, 1, {0, 0} },
  { (uint32_t *)TABLE_INVALID_VALUE, (uint32_t *)TABLE_INVALID_VALUE, (uint32_t)TABLE_INVALID_VALUE, (uint32_t)TABLE_DISABLE, 0xFF, 0xFF, {0, 0} },
  { (uint32_t *)TABLE_INVALID_VALUE, (uint32_t *)TABLE_INVALID_VALUE, (uint32_t)TABLE_INVALID_VALUE, (uint32_t)TABLE_DISABLE, 0xFF, 0xFF, {0, 0} }
};
