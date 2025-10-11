/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "McuSRT04T.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "McuWait.h"
#include "McuUtility.h"
#include "McuShell.h"
#include "McuRTOS.h"

static bool isInitialized = false;

uint8_t McuSRT04T_Init(void) {
  if (isInitialized) {
    return ERR_OK;
  }
  
  /* Initialize UART with configured parameters */
  uart_init(McuSRT04T_CONFIG_UART_INSTANCE, McuSRT04T_CONFIG_UART_BAUDRATE);
  
  /* Configure GPIO pins for UART */
  gpio_set_function(McuSRT04T_CONFIG_UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(McuSRT04T_CONFIG_UART_RX_PIN, GPIO_FUNC_UART);
  
  /* Set UART format */
  uart_set_format(McuSRT04T_CONFIG_UART_INSTANCE, McuSRT04T_CONFIG_UART_DATA_BITS, McuSRT04T_CONFIG_UART_STOP_BITS, McuSRT04T_CONFIG_UART_PARITY);
  
  /* Configure FIFO */
#if McuSRT04T_CONFIG_ENABLE_FIFO
  uart_set_fifo_enabled(McuSRT04T_CONFIG_UART_INSTANCE, true);
#else
  uart_set_fifo_enabled(McuSRT04T_CONFIG_UART_INSTANCE, false);
#endif
  
  isInitialized = true;
  return ERR_OK;
}

void McuSRT04T_Deinit(void) {
  if (isInitialized) {
    uart_deinit(McuSRT04T_CONFIG_UART_INSTANCE);
    isInitialized = false;
  }
}

bool McuSRT04T_IsReady(void) {
  return isInitialized;
}

uint8_t McuSRT04T_MeasureDistance(uint16_t *result) {
  if (!isInitialized) {
    return ERR_FAILED;
  }
  
  if (result == NULL) {
    return ERR_PARAM_DATA;
  }
  
  /* Clear any pending data in receive buffer */
  while (uart_is_readable(McuSRT04T_CONFIG_UART_INSTANCE)) {
    uart_getc(McuSRT04T_CONFIG_UART_INSTANCE); /* Clear buffer */
  }
  
  /* Send trigger command */
  uart_putc_raw(McuSRT04T_CONFIG_UART_INSTANCE, 0x55);
  /* Wait for sensor to process */
  McuWait_Waitms(10);
  
  /* Wait for response */
  uint8_t responseBuffer[4];
  uint32_t startTime = McuRTOS_xTaskGetTickCount();
  uint8_t bytesReceived = 0;
  
  while (bytesReceived < 4) {
    /* Check timeout */
    if ((McuRTOS_xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(McuSRT04T_CONFIG_TIMEOUT_MS)) {
      return ERR_BUSY; /* timeout */
    }
    
    /* Try to read a byte */
    if (uart_is_readable(McuSRT04T_CONFIG_UART_INSTANCE)) {
      responseBuffer[bytesReceived] = uart_getc(McuSRT04T_CONFIG_UART_INSTANCE);
      bytesReceived++;
    } else {
      /* No data available, wait a bit */
      McuWait_Waitms(1);
    }
  }
  
  /* Validate response format (should start with 0xFF) */
  if (responseBuffer[0] != 0xFF) {
    return ERR_VALUE; /* Invalid response format */
  }
  
  /* Extract distance from response (bytes 1 and 2) */
  uint16_t distance_mm = (uint16_t)((responseBuffer[1] << 8) | responseBuffer[2]);
  
  /* Check range validity */
  if (distance_mm < McuSRT04T_CONFIG_MIN_DISTANCE_MM || distance_mm > McuSRT04T_CONFIG_MAX_DISTANCE_MM) {
    return ERR_RANGE;
  }
  
  *result = distance_mm;
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"srt04t", (unsigned char*)"SRT04T Ultrasonic Sensor Status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  initialized", isInitialized?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"srt04t", (unsigned char*)"Group of SRT04T sensor commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  measure", (unsigned char*)"Take a distance measurement\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t McuSRT04T_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "srt04t help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "srt04t status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "srt04t measure")==0) {
    *handled = TRUE;
    
    uint16_t distance_mm;
    uint8_t error = McuSRT04T_MeasureDistance(&distance_mm);
    
    if (error == ERR_OK) {
      McuShell_SendStr((unsigned char*)"Distance: ", io->stdOut);
      McuShell_SendNum16u(distance_mm, io->stdOut);
      McuShell_SendStr((unsigned char*)" mm\r\n", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)"Measurement failed, error: ", io->stdErr);
      McuShell_SendNum8u(error, io->stdErr);
      McuShell_SendStr((unsigned char*)"\r\n", io->stdErr);
    }
    
    return ERR_OK;
  }
  
  return ERR_OK; /* not handled */
}