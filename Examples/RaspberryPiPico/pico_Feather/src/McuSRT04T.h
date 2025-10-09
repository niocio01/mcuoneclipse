/*
 * Copyright (c) 2025, SRT04T Ultrasonic Distance Sensor Module
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_SRT04T_H_
#define MCU_SRT04T_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "McuSRT04T_config.h"

#if PL_CONFIG_USE_US_SENS

#include "McuLib.h"
#include "McuShell.h"

/**
 * @brief Initialize the SRT04T sensor module
 * @return ERR_OK if successful, ERR_FAILED otherwise
 */
uint8_t McuSRT04T_Init(void);

/**
 * @brief Deinitialize the SRT04T sensor module
 */
void McuSRT04T_Deinit(void);

/**
 * @brief Trigger a distance measurement
 * @param result Pointer to store the measurement result in millimeters
 * @return ERR_OK if successful, ERR_FAILED if sensor not initialized, ERR_BUSY if timeout, ERR_VALUE if invalid data
 */
uint8_t McuSRT04T_MeasureDistance(uint16_t *result);

/**
 * @brief Parse shell commands for the SRT04T module
 * @param cmd Command string to parse
 * @param handled Pointer to flag indicating if command was handled
 * @param io Shell I/O interface
 * @return ERR_OK if successful
 */
uint8_t McuSRT04T_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);


#endif /* PL_CONFIG_USE_US_SENS */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MCU_SRT04T_H_ */