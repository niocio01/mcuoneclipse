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

 #define MCU_STHS34PF80_PRESENCE_FLAG   (1U << 0)
#if McuSTHS34PF80_CONFIG_USE_MOTION
 #define MCU_STHS34PF80_MOTION_FLAG     (1U << 1)
#endif

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


/*!
 * \brief Get the function status from the STHS34PF80 sensor
 * \param func_status pointer to store the function status
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetFuncStatus(sths34pf80_func_status_t *func_status);


#if McuSTHS34PF80_CONFIG_USE_INTERRUPT
/*!
 * \brief Register a callback function for handling interrupts from the STHS34PF80 sensor
 * \param callback pointer to the callback function
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_RegisterIntruptCallback(void (*callback)(void));
#endif

/*!
 * \brief Set the presence threshold
 * \param threshold presence threshold value (0-65535)
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetPresenceThreshold(uint16_t threshold);

/*!
 * \brief Get the presence threshold
 * \param threshold pointer to store the presence threshold value
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetPresenceThreshold(uint16_t *threshold);

#if McuSTHS34PF80_CONFIG_USE_MOTION
/*!
 * \brief Set the motion threshold
 * \param threshold motion threshold value (0-65535)
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetMotionThreshold(uint16_t threshold);

/*!
 * \brief Get the motion threshold
 * \param threshold pointer to store the motion threshold value
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetMotionThreshold(uint16_t *threshold);
#endif

/*!
 * \brief Set the output data rate
 * \param odr output data rate setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetODR(sths34pf80_odr_t odr);

/*!
 * \brief Get the output data rate
 * \param odr pointer to store the output data rate setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetODR(sths34pf80_odr_t *odr);

/*!
 * \brief Set the object temperature averaging
 * \param avg object averaging setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetObjectAveraging(sths34pf80_avg_tobject_num_t avg);

/*!
 * \brief Get the object temperature averaging
 * \param avg pointer to store the object averaging setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetObjectAveraging(sths34pf80_avg_tobject_num_t *avg);

/*!
 * \brief Set the ambient temperature averaging
 * \param avg ambient averaging setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetAmbientAveraging(sths34pf80_avg_tambient_num_t avg);

/*!
 * \brief Get the ambient temperature averaging
 * \param avg pointer to store the ambient averaging setting
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetAmbientAveraging(sths34pf80_avg_tambient_num_t *avg);

/*!
 * \brief Set the presence hysteresis value
 * \param hysteresis Hysteresis value (0-255)
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetPresenceHysteresis(uint8_t hysteresis);

/*!
 * \brief Get the presence hysteresis value
 * \param hysteresis pointer to store the hysteresis value
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetPresenceHysteresis(uint8_t *hysteresis);

#if McuSTHS34PF80_CONFIG_USE_MOTION
/*!
 * \brief Set the motion hysteresis value
 * \param hysteresis Hysteresis value (0-255)
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_SetMotionHysteresis(uint8_t hysteresis);

/*!
 * \brief Get the motion hysteresis value
 * \param hysteresis pointer to store the hysteresis value
 * \return Error code, or ERR_OK
 */
uint8_t McuSTHS34pf80_GetMotionHysteresis(uint8_t *hysteresis);
#endif

/*!
 * \brief Parse shell commands for the STHS34PF80 module
 * \param cmd Command string to parse
 * \param handled Pointer to flag indicating if command was handled
 * \param io Shell I/O interface
 * \return ERR_OK if successful
 */
uint8_t McuSTHS34pf80_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

 #endif /* MCUSTHS34PF80_H_ */