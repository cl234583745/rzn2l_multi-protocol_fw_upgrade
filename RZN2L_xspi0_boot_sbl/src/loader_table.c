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
 * loader_table (64 bytes per entry)
 */
const loader_table_t loader_table[TABLE_ENTRY_NUM] BSP_PLACE_IN_SECTION("CPU0_LOADER_TABLE") =
{
#if APP1_ENABLE
  { .f = { (uint32_t *)app1_bank0_prg_flash_addr, (uint32_t *)app1_bank0_prg_start_addr, (uint32_t)app1_bank0_prg_size, (uint32_t)APP1_ENABLE, APP1_ID, BANK0_ID, APP1_DUAL_BANK }, {0} },
  { .f = { (uint32_t *)app1_bank1_prg_flash_addr, (uint32_t *)app1_bank1_prg_start_addr, (uint32_t)app1_bank1_prg_size, (uint32_t)APP1_DUAL_BANK, APP1_ID, BANK1_ID, 0 }, {0} },
#endif
#if APP2_ENABLE
  { .f = { (uint32_t *)app2_bank0_prg_flash_addr, (uint32_t *)app2_bank0_prg_start_addr, (uint32_t)app2_bank0_prg_size, (uint32_t)APP2_ENABLE, APP2_ID, BANK0_ID, APP2_DUAL_BANK }, {0} },
  { .f = { (uint32_t *)app2_bank1_prg_flash_addr, (uint32_t *)app2_bank1_prg_start_addr, (uint32_t)app2_bank1_prg_size, (uint32_t)APP2_DUAL_BANK, APP2_ID, BANK1_ID, 0 }, {0} },
#endif
#if APP3_ENABLE
  { .f = { (uint32_t *)app3_bank0_prg_flash_addr, (uint32_t *)app3_bank0_prg_start_addr, (uint32_t)app3_bank0_prg_size, (uint32_t)APP3_ENABLE, APP3_ID, BANK0_ID, APP3_DUAL_BANK }, {0} },
  { .f = { (uint32_t *)app3_bank1_prg_flash_addr, (uint32_t *)app3_bank1_prg_start_addr, (uint32_t)app3_bank1_prg_size, (uint32_t)APP3_DUAL_BANK, APP3_ID, BANK1_ID, 0 }, {0} },
#endif
#if APP4_ENABLE
  { .f = { (uint32_t *)app4_bank0_prg_flash_addr, (uint32_t *)app4_bank0_prg_start_addr, (uint32_t)app4_bank0_prg_size, (uint32_t)APP4_ENABLE, APP4_ID, BANK0_ID, APP4_DUAL_BANK }, {0} },
  { .f = { (uint32_t *)app4_bank1_prg_flash_addr, (uint32_t *)app4_bank1_prg_start_addr, (uint32_t)app4_bank1_prg_size, (uint32_t)APP4_DUAL_BANK, APP4_ID, BANK1_ID, 0 }, {0} },
#endif
#if APP5_ENABLE
  { .f = { (uint32_t *)app5_bank0_prg_flash_addr, (uint32_t *)app5_bank0_prg_start_addr, (uint32_t)app5_bank0_prg_size, (uint32_t)APP5_ENABLE, APP5_ID, BANK0_ID, APP5_DUAL_BANK }, {0} },
  { .f = { (uint32_t *)app5_bank1_prg_flash_addr, (uint32_t *)app5_bank1_prg_start_addr, (uint32_t)app5_bank1_prg_size, (uint32_t)APP5_DUAL_BANK, APP5_ID, BANK1_ID, 0 }, {0} },
#endif

};
