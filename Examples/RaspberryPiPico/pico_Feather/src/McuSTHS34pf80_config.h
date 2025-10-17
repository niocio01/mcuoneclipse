/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_STHS34PF80_CONFIG_H_
#define MCU_STHS34PF80_CONFIG_H_

#include "platform.h"

#if PL_CONFIG_USE_IR_SENS
  #define McuSTHS34PF80_CONFIG_I2C_DEVICE_ADDRESS    (0x5A) /* 7bit I2C address, not-shifted value */

  #ifndef McuSTHS34PF80_CONFIG_USE_INTERRUPT
      #define McuSTHS34PF80_CONFIG_USE_INTERRUPT  (1)
  #endif
  #define McuSTHS34PF80_CONFIG_INT_PIN           (11) /* GPIO11 */

  #define McuSTHS34PF80_CONFIG_USE_MOTION     (0)



#endif /* PL_CONFIG_USE_IR_SENS */

#endif /* MCU_STHS34PF80_CONFIG_H_ */