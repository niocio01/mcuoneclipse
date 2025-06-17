/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SENSOR_READ_H_
#define _SENSOR_READ_H_

#include "platform.h"
#include "application.h"
#include "McuRTOS.h"

/*!
 * \brief Task to read sensor data
 */
void ReadSensors(void);


#endif /* _SENSOR_READ_H_ */