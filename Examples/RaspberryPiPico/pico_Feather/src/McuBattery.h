/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_BATTERY_H_
#define MCU_BATTERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "McuBattery_config.h"
#include "McuLib.h"
#include "McuShell.h"

/**
 * @brief Battery status enumeration
 */
typedef enum {
    McuBattery_STATUS_NOT_CONNECTED,  /* Battery not connected (e.g., voltage below measurable range) */
    McuBattery_STATUS_CHARGING,       /* Battery is currently charging (>= 4.2V) */
    McuBattery_STATUS_FULL,           /* Battery full (>= 4.0V) */
    McuBattery_STATUS_GOOD,           /* Battery good (3.6V - 4.0V) */
    McuBattery_STATUS_LOW,            /* Battery low (3.3V - 3.6V) */
    McuBattery_STATUS_CRITICAL,       /* Battery critical (< 3.3V) */
} McuBattery_Status_e;

/**
 * @brief Initialize the battery measurement module
 * @return ERR_OK if successful, ERR_FAILED otherwise
 */
uint8_t McuBattery_Init(void);

/**
 * @brief Deinitialize the battery measurement module
 */
void McuBattery_Deinit(void);

/**
 * @brief Check if the battery module is ready
 * @return true if initialized and ready, false otherwise
 */
bool McuBattery_IsReady(void);

/**
 * @brief Get battery status based on voltage
 * @param status Pointer to store the battery status
 * @return ERR_OK if successful, ERR_PARAM_DATA if status is NULL
 */
uint8_t McuBattery_GetStatus(uint8_t *status);

/**
 * @brief Measure battery voltage
 * @param voltage_mv Pointer to store the measured voltage in millivolts
 * @return ERR_OK if successful, ERR_FAILED if not initialized, ERR_PARAM_DATA if voltage_mv is NULL
 */
uint8_t McuBattery_MeasureVoltage(uint16_t *voltage_mv);

/**
 * @brief Get battery charge percentage (0-100%)
 * @param percentage Pointer to store the charge percentage
 * @return ERR_OK if successful, ERR_FAILED if not initialized, ERR_PARAM_DATA if percentage is NULL
 */
uint8_t McuBattery_GetChargePercentage(uint8_t *percentage);

/**
 * @brief Check if the board is powered via USB
 * @return true if USB power is detected, false otherwise
 */
bool McuBattery_IsUSBPowered(void);

/**
 * @brief Get raw ADC reading (for debugging/calibration)
 * @param adc_raw Pointer to store the raw ADC value
 * @return ERR_OK if successful, ERR_FAILED if not initialized, ERR_PARAM_DATA if adc_raw is NULL
 */
uint8_t McuBattery_GetRawADC(uint16_t *adc_raw);


/**
 * @brief Parse shell commands for the battery module
 * @param cmd Command string to parse
 * @param handled Pointer to flag indicating if command was handled
 * @param io Shell I/O interface
 * @return ERR_OK if successful
 */
uint8_t McuBattery_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

#ifdef __cplusplus
}
#endif

#endif /* MCU_BATTERY_H_ */