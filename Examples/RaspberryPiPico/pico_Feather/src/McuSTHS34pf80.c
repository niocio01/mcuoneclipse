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
#include "McuGPIO.h"
#include "McuLog.h"
#include "McuShell.h"

#define McuSTHS34PF80_I2C_DEVICE_ADDRESS   (0x5A) /* 7bit I2C address, not-shifted value */


static stmdev_ctx_t dev_ctx; /* STHS34PF80 object */
static uint8_t whoami = 0; /* who am I register value */
static sths34pf80_status_t status;

#if McuSTHS34PF80_CONFIG_USE_INTERRUPT
    static McuGPIO_Handle_t irqPin = NULL;
    static void (*irqCallback)(void) = NULL;
    static void gpio_IsrCallback(uint gpio, uint32_t events);
    static void InitIRQPin(void);
#endif


/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);


/* init ---------------------------------------------------------*/
uint8_t McuSTHS34pf80_Init(void)
{
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.mdelay = platform_delay;
    dev_ctx.handle = (void *)1234;

    sths34pf80_device_id_get(&dev_ctx, &whoami);
    if (whoami != STHS34PF80_ID)
        return ERR_PARAM_RECIPIENT;

    int32_t status = 0;

    // Set block data rate update to true
    status |= sths34pf80_block_data_update_set(&dev_ctx, true);
        
    // Set object temperature averaging (AVG_TMOS = 8)
    status |= sths34pf80_avg_tobject_num_set(&dev_ctx, STHS34PF80_AVG_TMOS_8);

    // Set ambient temperature averaging (AVG_TAMB = 4)
    status |= sths34pf80_avg_tambient_num_set(&dev_ctx, STHS34PF80_AVG_T_4);

    // Configure presence and motion detection thresholds and hysteresis
    sths34pf80_presence_threshold_set(&dev_ctx, 200);
    sths34pf80_presence_hysteresis_set(&dev_ctx, 20);
    sths34pf80_motion_threshold_set(&dev_ctx, 300);
    sths34pf80_motion_hysteresis_set(&dev_ctx, 30);

    // Set the data rate (ODR) to 2Hz
    status |= sths34pf80_odr_set(&dev_ctx, STHS34PF80_ODR_AT_2Hz);

    // Reset the algorithm, since we changed thresholds
    sths34pf80_algo_reset(&dev_ctx);    

    #if McuSTHS34PF80_CONFIG_USE_INTERRUPT
        sths34pf80_int_or_set(&dev_ctx, STHS34PF80_INT_PRESENCE);
        sths34pf80_route_int_set(&dev_ctx, STHS34PF80_INT_OR);
        sths34pf80_int_mode_t int_mode = {
            .pin = STHS34PF80_PUSH_PULL,
            .polarity = STHS34PF80_ACTIVE_HIGH
        };
        sths34pf80_int_mode_set(&dev_ctx, int_mode);
        InitIRQPin();
    #endif
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

uint8_t McuSTHS34pf80_GetFuncStatus(sths34pf80_func_status_t *func_status)
{
    int32_t ret;

    ret = sths34pf80_read_reg(&dev_ctx, STHS34PF80_FUNC_STATUS, (uint8_t *)func_status, 1);
    if (ret != 0) {
        return ERR_FAILED; /* Error reading function status */
    }
    return ERR_OK; /* Success */
}

#if McuSTHS34PF80_CONFIG_USE_INTERRUPT
uint8_t McuSTHS34pf80_RegisterIntruptCallback(void (*callback)(void))
{
    if (callback == NULL) {
        return ERR_NOTAVAIL; /* Invalid callback */
    }

    irqCallback = callback;
    return ERR_OK;
}
#endif

uint8_t McuSTHS34pf80_SetPresenceThreshold(uint16_t threshold) {
    if (sths34pf80_presence_threshold_set(&dev_ctx, threshold) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetPresenceThreshold(uint16_t *threshold) {
    if (threshold == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_presence_threshold_get(&dev_ctx, threshold) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetMotionThreshold(uint16_t threshold) {
    if (sths34pf80_motion_threshold_set(&dev_ctx, threshold) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetMotionThreshold(uint16_t *threshold) {
    if (threshold == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_motion_threshold_get(&dev_ctx, threshold) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetODR(sths34pf80_odr_t odr) {
    if (sths34pf80_odr_set(&dev_ctx, odr) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetODR(sths34pf80_odr_t *odr) {
    if (odr == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_odr_get(&dev_ctx, odr) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetObjectAveraging(sths34pf80_avg_tobject_num_t avg) {
    if (sths34pf80_avg_tobject_num_set(&dev_ctx, avg) != 0) {
        return ERR_FAILED;
    }
    // Reset the algorithm as required by datasheet when averaging is modified
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetObjectAveraging(sths34pf80_avg_tobject_num_t *avg) {
    if (avg == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_avg_tobject_num_get(&dev_ctx, avg) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetAmbientAveraging(sths34pf80_avg_tambient_num_t avg) {
    if (sths34pf80_avg_tambient_num_set(&dev_ctx, avg) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetAmbientAveraging(sths34pf80_avg_tambient_num_t *avg) {
    if (avg == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_avg_tambient_num_get(&dev_ctx, avg) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetPresenceHysteresis(uint8_t hysteresis) {
    if (sths34pf80_presence_hysteresis_set(&dev_ctx, hysteresis) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetPresenceHysteresis(uint8_t *hysteresis) {
    if (hysteresis == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_presence_hysteresis_get(&dev_ctx, hysteresis) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

uint8_t McuSTHS34pf80_SetMotionHysteresis(uint8_t hysteresis) {
    if (sths34pf80_motion_hysteresis_set(&dev_ctx, hysteresis) != 0) {
        return ERR_FAILED;
    }
    sths34pf80_algo_reset(&dev_ctx);
    return ERR_OK;
}

uint8_t McuSTHS34pf80_GetMotionHysteresis(uint8_t *hysteresis) {
    if (hysteresis == NULL) {
        return ERR_NOTAVAIL;
    }
    if (sths34pf80_motion_hysteresis_get(&dev_ctx, hysteresis) != 0) {
        return ERR_FAILED;
    }
    return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"IR", (unsigned char*)"Group of IR presence sensor (STHS34PF80) commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or detection status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get settings", (unsigned char*)"Show sensor configuration settings\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set pres thr <val>", (unsigned char*)"Set presence threshold (0-65535)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set pres hyst <val>", (unsigned char*)"Set presence hysteresis (0-255)\r\n", io->stdOut);
#if McuSTHS34PF80_CONFIG_USE_MOTION
  McuShell_SendHelpStr((unsigned char*)"  set motion thr <val>", (unsigned char*)"Set motion threshold (0-65535)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set motion hyst <val>", (unsigned char*)"Set motion hysteresis (0-255)\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  set avg obj <val>", (unsigned char*)"Set object averaging (2, 8, 32, 128, 256, 512, 1024, 2048)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set avg amb <val>", (unsigned char*)"Set ambient averaging (1, 2, 4, 8)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set odr <hz>", (unsigned char*)"Set output data rate (off, 0.25, 0.5, 1, 2, 4, 8, 15, 30 Hz)\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"IR", (unsigned char*)"STHS34PF80 Detection Status\r\n", io->stdOut);
  
  bool presenceFlag;
  int16_t presenceVal;
  sths34pf80_func_status_t func_status;
  
  if (McuSTHS34pf80_GetPresence(&presenceFlag, &presenceVal) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  presence", (unsigned char*)"", io->stdOut);
    McuShell_SendStr(presenceFlag ? (unsigned char*)"DETECTED" : (unsigned char*)"NOT DETECTED", io->stdOut);
    McuShell_SendStr((unsigned char*)" (", io->stdOut);
    McuShell_SendNum16s(presenceVal, io->stdOut);
    McuShell_SendStr((unsigned char*)")\r\n", io->stdOut);
  }
  
#if McuSTHS34PF80_CONFIG_USE_MOTION
  if (McuSTHS34pf80_GetFuncStatus(&func_status) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  motion", (unsigned char*)"", io->stdOut);
    McuShell_SendStr(func_status.mot_flag ? (unsigned char*)"DETECTED" : (unsigned char*)"NOT DETECTED", io->stdOut);
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
#endif
  
  return ERR_OK;
}

static uint8_t PrintSettings(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"IR", (unsigned char*)"STHS34PF80 Configuration Settings\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  initialized", (unsigned char*)"yes\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  device ID", (unsigned char*)"0x", io->stdOut);
  McuShell_SendNum8s(whoami, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  
  // Read current settings
  sths34pf80_odr_t odr;
  sths34pf80_avg_tobject_num_t avg_tmos;
  sths34pf80_avg_tambient_num_t avg_tamb;
  uint16_t pres_threshold, motion_threshold;
  uint8_t pres_hyst, motion_hyst;
  
  // Show settings in same order as help commands - presence settings first
  if (McuSTHS34pf80_GetPresenceThreshold(&pres_threshold) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  pres threshold", (unsigned char*)"", io->stdOut);
    McuShell_SendNum16u(pres_threshold, io->stdOut);
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  if (McuSTHS34pf80_GetPresenceHysteresis(&pres_hyst) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  pres hysteresis", (unsigned char*)"", io->stdOut);
    McuShell_SendNum8u(pres_hyst, io->stdOut);
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
#if McuSTHS34PF80_CONFIG_USE_MOTION
  // Motion settings grouped together
  if (McuSTHS34pf80_GetMotionThreshold(&motion_threshold) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  motion threshold", (unsigned char*)"", io->stdOut);
    McuShell_SendNum16u(motion_threshold, io->stdOut);
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  if (McuSTHS34pf80_GetMotionHysteresis(&motion_hyst) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  motion hysteresis", (unsigned char*)"", io->stdOut);
    McuShell_SendNum8u(motion_hyst, io->stdOut);
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
#endif
  
  if (McuSTHS34pf80_GetObjectAveraging(&avg_tmos) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  object avg", (unsigned char*)"", io->stdOut);
    switch(avg_tmos) {
      case STHS34PF80_AVG_TMOS_2: McuShell_SendStr((unsigned char*)"2", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_8: McuShell_SendStr((unsigned char*)"8", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_32: McuShell_SendStr((unsigned char*)"32", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_128: McuShell_SendStr((unsigned char*)"128", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_256: McuShell_SendStr((unsigned char*)"256", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_512: McuShell_SendStr((unsigned char*)"512", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_1024: McuShell_SendStr((unsigned char*)"1024", io->stdOut); break;
      case STHS34PF80_AVG_TMOS_2048: McuShell_SendStr((unsigned char*)"2048", io->stdOut); break;
      default: McuShell_SendStr((unsigned char*)"unknown", io->stdOut); break;
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  if (McuSTHS34pf80_GetAmbientAveraging(&avg_tamb) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  ambient avg", (unsigned char*)"", io->stdOut);
    switch(avg_tamb) {
      case STHS34PF80_AVG_T_1: McuShell_SendStr((unsigned char*)"1", io->stdOut); break;
      case STHS34PF80_AVG_T_2: McuShell_SendStr((unsigned char*)"2", io->stdOut); break;
      case STHS34PF80_AVG_T_4: McuShell_SendStr((unsigned char*)"4", io->stdOut); break;
      case STHS34PF80_AVG_T_8: McuShell_SendStr((unsigned char*)"8", io->stdOut); break;
      default: McuShell_SendStr((unsigned char*)"unknown", io->stdOut); break;
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  if (McuSTHS34pf80_GetODR(&odr) == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*)"  ODR", (unsigned char*)"", io->stdOut);
    switch(odr) {
      case STHS34PF80_ODR_OFF: McuShell_SendStr((unsigned char*)"OFF", io->stdOut); break;
      case STHS34PF80_ODR_AT_0Hz25: McuShell_SendStr((unsigned char*)"0.25 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_0Hz50: McuShell_SendStr((unsigned char*)"0.5 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_1Hz: McuShell_SendStr((unsigned char*)"1 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_2Hz: McuShell_SendStr((unsigned char*)"2 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_4Hz: McuShell_SendStr((unsigned char*)"4 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_8Hz: McuShell_SendStr((unsigned char*)"8 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_15Hz: McuShell_SendStr((unsigned char*)"15 Hz", io->stdOut); break;
      case STHS34PF80_ODR_AT_30Hz: McuShell_SendStr((unsigned char*)"30 Hz", io->stdOut); break;
      default: McuShell_SendStr((unsigned char*)"unknown", io->stdOut); break;
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  return ERR_OK;
}

uint8_t McuSTHS34pf80_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "ir help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "ir status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "ir get settings")==0) {
    *handled = TRUE;
    return PrintSettings(io);
  } else if (McuUtility_strncmp((char*)cmd, "ir set pres thr ", sizeof("ir set pres thr ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set pres thr ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK && val <= 65535) {
      if (McuSTHS34pf80_SetPresenceThreshold((uint16_t)val) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Presence threshold set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting presence threshold\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid value (0-65535)\r\n", io->stdErr);
    }
    return ERR_OK;
#if McuSTHS34PF80_CONFIG_USE_MOTION
  } else if (McuUtility_strncmp((char*)cmd, "ir set motion thr ", sizeof("ir set motion thr ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set motion thr ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK && val <= 65535) {
      if (McuSTHS34pf80_SetMotionThreshold((uint16_t)val) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Motion threshold set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting motion threshold\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid value (0-65535)\r\n", io->stdErr);
    }
    return ERR_OK;
#endif
  } else if (McuUtility_strncmp((char*)cmd, "ir set odr ", sizeof("ir set odr ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set odr ")-1;
    sths34pf80_odr_t odr;
    
    if (McuUtility_strcmp((char*)p, "off")==0) {
      odr = STHS34PF80_ODR_OFF;
    } else if (McuUtility_strcmp((char*)p, "0.25")==0) {
      odr = STHS34PF80_ODR_AT_0Hz25;
    } else if (McuUtility_strcmp((char*)p, "0.5")==0) {
      odr = STHS34PF80_ODR_AT_0Hz50;
    } else {
      uint32_t hz;
      if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &hz) == ERR_OK) {
        switch(hz) {
          case 1: odr = STHS34PF80_ODR_AT_1Hz; break;
          case 2: odr = STHS34PF80_ODR_AT_2Hz; break;
          case 4: odr = STHS34PF80_ODR_AT_4Hz; break;
          case 8: odr = STHS34PF80_ODR_AT_8Hz; break;
          case 15: odr = STHS34PF80_ODR_AT_15Hz; break;
          case 30: odr = STHS34PF80_ODR_AT_30Hz; break;
          default:
            McuShell_SendStr((unsigned char*)"Invalid ODR. Valid: off, 0.25, 0.5, 1, 2, 4, 8, 15, 30 Hz\r\n", io->stdErr);
            return ERR_OK;
        }
      } else {
        McuShell_SendStr((unsigned char*)"Invalid ODR. Valid: off, 0.25, 0.5, 1, 2, 4, 8, 15, 30 Hz\r\n", io->stdErr);
        return ERR_OK;
      }
    }
    
    if (McuSTHS34pf80_SetODR(odr) == ERR_OK) {
      McuShell_SendStr((unsigned char*)"ODR set to ", io->stdOut);
      McuShell_SendStr((unsigned char*)p, io->stdOut);
      if (odr != STHS34PF80_ODR_OFF) {
        McuShell_SendStr((unsigned char*)" Hz", io->stdOut);
      }
      McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)"Error setting ODR\r\n", io->stdErr);
    }
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "ir set avg obj ", sizeof("ir set avg obj ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set avg obj ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK) {
      sths34pf80_avg_tobject_num_t avg;
      switch(val) {
        case 2: avg = STHS34PF80_AVG_TMOS_2; break;
        case 8: avg = STHS34PF80_AVG_TMOS_8; break;
        case 32: avg = STHS34PF80_AVG_TMOS_32; break;
        case 128: avg = STHS34PF80_AVG_TMOS_128; break;
        case 256: avg = STHS34PF80_AVG_TMOS_256; break;
        case 512: avg = STHS34PF80_AVG_TMOS_512; break;
        case 1024: avg = STHS34PF80_AVG_TMOS_1024; break;
        case 2048: avg = STHS34PF80_AVG_TMOS_2048; break;
        default:
          McuShell_SendStr((unsigned char*)"Invalid object averaging. Valid: 2, 8, 32, 128, 256, 512, 1024, 2048\r\n", io->stdErr);
          return ERR_OK;
      }
      
      if (McuSTHS34pf80_SetObjectAveraging(avg) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Object averaging set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting object averaging\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid object averaging value\r\n", io->stdErr);
    }
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "ir set avg amb ", sizeof("ir set avg amb ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set avg amb ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK) {
      sths34pf80_avg_tambient_num_t avg;
      switch(val) {
        case 1: avg = STHS34PF80_AVG_T_1; break;
        case 2: avg = STHS34PF80_AVG_T_2; break;
        case 4: avg = STHS34PF80_AVG_T_4; break;
        case 8: avg = STHS34PF80_AVG_T_8; break;
        default:
          McuShell_SendStr((unsigned char*)"Invalid ambient averaging. Valid: 1, 2, 4, 8\r\n", io->stdErr);
          return ERR_OK;
      }
      
      if (McuSTHS34pf80_SetAmbientAveraging(avg) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Ambient averaging set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting ambient averaging\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid ambient averaging value\r\n", io->stdErr);
    }
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "ir set pres hyst ", sizeof("ir set pres hyst ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set pres hyst ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK && val <= 255) {
      if (McuSTHS34pf80_SetPresenceHysteresis((uint8_t)val) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Presence hysteresis set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting presence hysteresis\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid value (0-255)\r\n", io->stdErr);
    }
    return ERR_OK;
#if McuSTHS34PF80_CONFIG_USE_MOTION
  } else if (McuUtility_strncmp((char*)cmd, "ir set motion hyst ", sizeof("ir set motion hyst ")-1)==0) {
    *handled = TRUE;
    const char *p = (char*)cmd + sizeof("ir set motion hyst ")-1;
    uint32_t val;
    
    if (McuUtility_ScanDecimal32uNumber((const unsigned char**)&p, &val) == ERR_OK && val <= 255) {
      if (McuSTHS34pf80_SetMotionHysteresis((uint8_t)val) == ERR_OK) {
        McuShell_SendStr((unsigned char*)"Motion hysteresis set to ", io->stdOut);
        McuShell_SendNum32u(val, io->stdOut);
        McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)"Error setting motion hysteresis\r\n", io->stdErr);
      }
    } else {
      McuShell_SendStr((unsigned char*)"Invalid value (0-255)\r\n", io->stdErr);
    }
    return ERR_OK;
#endif
  }
  
  return ERR_OK; /* not handled */
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

#if McuSTHS34PF80_CONFIG_USE_INTERRUPT
static void InitIRQPin(void) {
  McuGPIO_Config_t config;

  config.hw.pin = McuSTHS34PF80_CONFIG_INT_PIN;
  config.isInput = true;
  config.hw.pull = McuGPIO_PULL_DISABLE;
  irqPin = McuGPIO_InitGPIO(&config);
  if (irqPin==NULL) {
    McuLog_fatal("failed creating IRQ pin");
    for(;;) {}
  }

  gpio_set_irq_enabled_with_callback(McuSTHS34PF80_CONFIG_INT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
}


static void gpio_IsrCallback(uint gpio, uint32_t events) {
  switch(gpio) {
    case McuSTHS34PF80_CONFIG_INT_PIN:
      /* GPIO interrupt flag is automatically cleared by Pico SDK */
      if (irqCallback != NULL) {
        irqCallback();
      }
      break;
    default:
      break;
  }
}
#endif

