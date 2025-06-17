/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "McuLib.h"
#include "McuSTHS34pf80.h"
#include "sths34pf80_reg.h"
#include "McuGenericI2C.h"
#include <stdbool.h>
#include "McuUtility.h"
#include "McuWait.h"
#include "McuLog.h"

#define McuSTHS34PF80_I2C_DEVICE_ADDRESS   (0x5A) /* 7bit I2C address, not-shifted value */


static stmdev_ctx_t dev_ctx; /* STHS34PF80 object */
static uint8_t tx_buffer[1000]; 
static uint8_t whoami = 0; /* who am I register value */
static sths34pf80_status_t status;

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_delay(uint32_t ms);


/* init ---------------------------------------------------------*/
uint8_t McuSTHS34pf80_Init(void)
{
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.mdelay = platform_delay;
    dev_ctx.handle = 1234;

    sths34pf80_device_id_get(&dev_ctx, &whoami);
    if (whoami != STHS34PF80_ID)
        return ERR_PARAM_RECIPIENT;

    int32_t status = 0;
        
     // Set temperature object number set average (AVG_TMOS = 32)
    status |= sths34pf80_avg_tobject_num_set(&dev_ctx, STHS34PF80_AVG_TMOS_32);

    // Set ambient temperature average (AVG_TAMB = 8)
    status |= sths34pf80_avg_tambient_num_set(&dev_ctx, STHS34PF80_AVG_T_8);

    // Set block data rate update to true
    status |= sths34pf80_block_data_update_set(&dev_ctx, true);

    // Set the data rate (ODR) to 1Hz
    status |= sths34pf80_odr_set(&dev_ctx, STHS34PF80_ODR_AT_1Hz);

    return ERR_OK;
}

bool McuSTHS34pf80_IsDataReady(void)
{  
    int32_t ret = sths34pf80_read_reg(&dev_ctx, STHS34PF80_STATUS, (uint8_t *)&status, 1);

    if (ret != 0) {
        McuLog_error("STH34PF80 Error reading status register: %d", ret);
        return false;
    }
    return status.drdy;
}

uint8_t McuSTHS34pf80_GetPresence(bool *presenceFlag, int16_t *presenceVal)
{
    sths34pf80_func_status_t func_status;
    int32_t ret;
    *presenceVal = 0; /* default value */

    ret = sths34pf80_read_reg(&dev_ctx, STHS34PF80_FUNC_STATUS, (uint8_t *)&func_status, 1);
    if (ret != 0) {
        return ERR_FAILED; /* Error reading function status */
    }
    if (func_status.pres_flag == 0) {
        *presenceFlag = false; /* No presence detected */
        return ERR_OK;
    }
    *presenceFlag = true; /* Presence detected */
    ret = sths34pf80_tpresence_raw_get(&dev_ctx, presenceVal);
    if (ret != 0) return ERR_FAILED; /* Error reading presence value */

    return ERR_OK; /* Presence detected and value read successfully */
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    uint8_t *buf = (uint8_t *)bufp;
    return McuGenericI2C_WriteAddress(McuSTHS34PF80_I2C_DEVICE_ADDRESS, &reg, sizeof(reg), buf, len);
}


static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    return McuGenericI2C_ReadAddress(McuSTHS34PF80_I2C_DEVICE_ADDRESS, &reg, sizeof(reg), bufp, len);
}

static void platform_delay(uint32_t ms)
{
    McuWait_Waitms(ms);
}