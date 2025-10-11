/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_SRT04T_CONFIG_H_
#define MCU_SRT04T_CONFIG_H_

#include "platform.h"
/* UART Configuration */
#define McuSRT04T_CONFIG_UART_INSTANCE     (uart0)   /*!< UART instance for communication */
#define McuSRT04T_CONFIG_UART_BAUDRATE     (9600)   /*!< Baud rate for UART communication */
#define McuSRT04T_CONFIG_UART_TX_PIN       (0)       /*!< TX pin (GP0) */
#define McuSRT04T_CONFIG_UART_RX_PIN       (1)       /*!< RX pin (GP1) */
#define McuSRT04T_CONFIG_UART_DATA_BITS    (8)       /*!< Number of data bits */
#define McuSRT04T_CONFIG_UART_STOP_BITS    (1)       /*!< Number of stop bits */
#define McuSRT04T_CONFIG_UART_PARITY       (UART_PARITY_NONE)  /*!< Parity setting */

/* Protocol Configuration */
#define McuSRT04T_CONFIG_TIMEOUT_MS        (1000)    /*!< Default timeout for receiving response (ms) */

/* Range Configuration */
#define McuSRT04T_CONFIG_MIN_DISTANCE_MM   (30)      /*!< Minimum measurable distance in mm */
#define McuSRT04T_CONFIG_MAX_DISTANCE_MM   (4000)    /*!< Maximum measurable distance in mm */


#endif /* MCU_SRT04T_CONFIG_H_ */