/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #ifndef MCU_VL53L5CX_H_
 #define MCU_VL53L5CX_H_
 
 #include <stdint.h>
 #include "McuVL53L5CX_config.h"
 #include "McuShell.h"
 #include "vl53l5cx_api.h"

/*!
 * \brief Initialize the VL53L5CX sensor
 * \return Error code, or ERR_OK
 */
uint8_t McuVL53L5CX_Init(void);

/*!
 * \brief Load the Firmaware to the VL53L5CX sensor
 * \return Error code, or ERR_OK
 */
uint8_t McuVL53L5CX_Programm(void);

/*!
 * \brief Start the VL53L5CX sensor ranging
 * \return Error code, or ERR_OK
 */
uint8_t McuVL53L5CX_StartRanging(void);

/*!
 * \brief Get the ranging data from the VL53L5CX sensor
 * \param results Pointer to the results data structure
 * \return Error code, or ERR_OK
 */
uint8_t McuVL53L5CX_GetRangingData(VL53L5CX_ResultsData *results);


/*!
 * \brief Check if the VL53L5CX sensor is ready to provide data
 * \param isReady Pointer to a variable to store the readiness status
 * \return true, if the sensor is ready, false otherwise
 */
bool McuVL53L5CX_IsDataReady(void);


 #endif /* MCU_VL53L5CX_H_ */