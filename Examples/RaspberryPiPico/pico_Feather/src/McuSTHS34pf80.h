/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #ifndef MCUSTHS34PF80_H_
 #define MCUSTHS34PF80_H_
 
 #include <stdint.h>
 #include "McuSTHS34pf80_config.h"
 #include "McuShell.h"
 #include "sths34pf80_reg.h"

/*!
 * \brief Initialize the STHS34PF80 sensor
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_Init(void);

/*!
 * \brief Check if the STHS34PF80 sensor is ready to provide data
 * \return true, if the sensor is ready, false otherwise
 */
bool McuSTHS34pf80_IsDataReady(void);

/*!
 * \brief Get the presence value from the STHS34PF80 sensor
 * \param presenceFlag indicates, if presence is detected
 * \param presenceVal pointer to store the presence value
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetPresence(bool *presenceFlag, int16_t *presenceVal);

#define SENSOR_BUS I2CD1


 #endif /* MCUSTHS34PF80_H_ */