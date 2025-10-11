/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "McuBattery.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "McuWait.h"
#include "McuUtility.h"
#include "McuShell.h"

static bool isInitialized = false;

uint8_t McuBattery_Init(void) {
  if (isInitialized) {
    return ERR_OK;
  }
  
  /* Initialize ADC */
  adc_init();
  
  /* Make sure GPIO is high-impedance, no pullups etc */
  adc_gpio_init(McuBattery_CONFIG_VBAT_GPIO_PIN);
  
  /* Initialize USB power detection pin as digital input */
  gpio_init(McuBattery_CONFIG_VIN_GPIO_PIN);
  gpio_set_dir(McuBattery_CONFIG_VIN_GPIO_PIN, GPIO_IN);
  gpio_pull_down(McuBattery_CONFIG_VIN_GPIO_PIN);  /* Pull down to ensure clean reading */
  
  /* Select ADC input */
  adc_select_input(McuBattery_CONFIG_VBAT_ADC_INPUT);
  
  isInitialized = true;
  return ERR_OK;
}

void McuBattery_Deinit(void) {
  if (isInitialized) {
    /* Reset GPIO to input with no special function */
    gpio_init(McuBattery_CONFIG_VBAT_GPIO_PIN);
    gpio_set_dir(McuBattery_CONFIG_VBAT_GPIO_PIN, GPIO_IN);
    isInitialized = false;
  }
}

bool McuBattery_IsReady(void) {
  return isInitialized;
}

bool McuBattery_IsUSBPowered(void) {
  if (!isInitialized) {
    return false;
  }
  /* Read the USB power detection pin - HIGH means USB power present */
  return gpio_get(McuBattery_CONFIG_VIN_GPIO_PIN);
}

uint8_t McuBattery_GetStatus(uint8_t *status)
{
    if (!isInitialized) {
        return ERR_FAILED;
    }
    
    if (status == NULL) {
        return ERR_PARAM_DATA;
    }

    uint16_t voltage_mv;
        uint8_t result = McuBattery_MeasureVoltage(&voltage_mv);
        if (result != ERR_OK) {
            return ERR_FAILED;
        }
    
    /* Check if USB power is connected first */
    if (McuBattery_IsUSBPowered()) {  
        /* If voltage is higher than expected, battery is probably not connected */
        if (voltage_mv > McuBattery_CONFIG_BATTERY_MAX_MV) {
            *status = McuBattery_STATUS_NOT_CONNECTED;
        } else {
            *status = McuBattery_STATUS_CHARGING;
        }
    }
    else {
      /* Determine battery status based on voltage when not charging */
      if (voltage_mv >= McuBattery_CONFIG_BATTERY_FULL_MV) {
          *status = McuBattery_STATUS_FULL;
      } else if (voltage_mv >= McuBattery_CONFIG_BATTERY_GOOD_MV) {
          *status = McuBattery_STATUS_GOOD;
      } else if (voltage_mv >= McuBattery_CONFIG_BATTERY_LOW_MV) {
          *status = McuBattery_STATUS_LOW;
      } else if (voltage_mv >= McuBattery_CONFIG_BATTERY_MIN_MV) {
          *status = McuBattery_STATUS_CRITICAL;
      } else {
          *status = McuBattery_STATUS_NOT_CONNECTED; // Below measurable range
      }    
    }
    return ERR_OK;
}

uint8_t McuBattery_GetRawADC(uint16_t *adc_raw) {
  if (!isInitialized) {
    return ERR_FAILED;
  }
  
  if (adc_raw == NULL) {
    return ERR_PARAM_DATA;
  }
  
  /* Select the correct ADC input */
  adc_select_input(McuBattery_CONFIG_VBAT_ADC_INPUT);
  
  /* Take multiple samples for averaging */
  uint32_t sum = 0;
  for (int i = 0; i < McuBattery_CONFIG_NUM_SAMPLES; i++) {
    sum += adc_read();
    }
  
  *adc_raw = (uint16_t)(sum / McuBattery_CONFIG_NUM_SAMPLES);
  return ERR_OK;
}

uint8_t McuBattery_MeasureVoltage(uint16_t *voltage_mv) {
  if (!isInitialized) {
    return ERR_FAILED;
  }
  
  if (voltage_mv == NULL) {
    return ERR_PARAM_DATA;
  }
  
  uint16_t adc_raw;
  uint8_t result = McuBattery_GetRawADC(&adc_raw);
  if (result != ERR_OK) {
    return result;
  }
  
  /* Convert ADC value to voltage at ADC input (in mV) */
  uint32_t adc_voltage_mv = ((uint32_t)adc_raw * McuBattery_CONFIG_ADC_VREF_MV) / McuBattery_CONFIG_ADC_MAX_VALUE;
  
  /* Calculate battery voltage considering voltage divider */
  /* Battery voltage = ADC voltage * (R1 + R2) / R2 */
  uint32_t battery_voltage_mv = (adc_voltage_mv * (McuBattery_CONFIG_VOLTAGE_DIVIDER_R1 + McuBattery_CONFIG_VOLTAGE_DIVIDER_R2)) / McuBattery_CONFIG_VOLTAGE_DIVIDER_R2;
  
  /* Apply calibration */
  battery_voltage_mv = (battery_voltage_mv * McuBattery_CONFIG_CALIBRATION_GAIN);
  battery_voltage_mv += McuBattery_CONFIG_CALIBRATION_OFFSET;
  
  *voltage_mv = (uint16_t)battery_voltage_mv;
  return ERR_OK;
}

uint8_t McuBattery_GetChargePercentage(uint8_t *percentage) {
  if (!isInitialized) {
      return ERR_FAILED;
  }
  if (percentage == NULL) {
      return ERR_PARAM_DATA;
  }

  /* Check battery connection status first */
  uint8_t status;
  if (McuBattery_GetStatus(&status) != ERR_OK) {
    *percentage = 0;
    return ERR_FAILED;
  }
  
  /* If battery not connected, percentage is 0 */
  if (status == McuBattery_STATUS_NOT_CONNECTED) {
    *percentage = 0;
    return ERR_OK;
  }

  uint16_t voltage_mv;
  if (McuBattery_MeasureVoltage(&voltage_mv) != ERR_OK) {
    *percentage = 0;  // Unable to measure voltage, return 0%
    return ERR_FAILED;
  }

  /* Linear mapping from empty voltage to full voltage */
  if (voltage_mv >= McuBattery_CONFIG_BATTERY_MAX_MV) {
    *percentage = 100;
  } else if (voltage_mv <= McuBattery_CONFIG_BATTERY_MIN_MV) {
    *percentage = 0;
  } else {
    /* Linear interpolation between empty and full */
    uint32_t range = McuBattery_CONFIG_BATTERY_MAX_MV - McuBattery_CONFIG_BATTERY_MIN_MV;
    uint32_t offset = voltage_mv - McuBattery_CONFIG_BATTERY_MIN_MV;
    *percentage = (uint8_t)((offset * 100) / range);
  }
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"battery", (unsigned char*)"Battery Module Status\r\n", io->stdOut);
  
  if (isInitialized) {
    uint8_t status, percentage;
    uint16_t voltage_mv, adc_raw;
    if (McuBattery_GetStatus(&status) == ERR_OK) {
      switch (status) {
        case McuBattery_STATUS_NOT_CONNECTED: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"not connected\r\n", io->stdOut); break;
        case McuBattery_STATUS_CHARGING: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"charging\r\n", io->stdOut); break;
        case McuBattery_STATUS_FULL: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"full\r\n", io->stdOut); break;
        case McuBattery_STATUS_GOOD: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"good\r\n", io->stdOut); break;
        case McuBattery_STATUS_LOW: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"low\r\n", io->stdOut); break;
        case McuBattery_STATUS_CRITICAL: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"critical\r\n", io->stdOut); break;
        default: McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"unknown\r\n", io->stdOut); break;
      }
    } else {
      McuShell_SendStatusStr((unsigned char*)"  status", (unsigned char*)"error\r\n", io->stdOut);
    }

    if (McuBattery_MeasureVoltage(&voltage_mv) == ERR_OK && McuBattery_GetRawADC(&adc_raw) == ERR_OK && McuBattery_GetChargePercentage(&percentage) == ERR_OK) {
      McuShell_SendStatusStr((unsigned char*)"  charge", (unsigned char*)"", io->stdOut);
      McuShell_SendNum8u(percentage, io->stdOut);
      McuShell_SendStr((unsigned char*)"%\r\n", io->stdOut);

      McuShell_SendStatusStr((unsigned char*)"  voltage", (unsigned char*)"", io->stdOut);
      McuShell_SendNum16u(voltage_mv / 1000, io->stdOut);
      McuShell_SendStr((unsigned char*)".", io->stdOut);
      uint16_t decimal = (voltage_mv % 1000) / 10;  /* Get 2 decimal places */
      if (decimal < 10) {
        McuShell_SendStr((unsigned char*)"0", io->stdOut);  /* Leading zero for single digit */
      }
      McuShell_SendNum16u(decimal, io->stdOut);
      McuShell_SendStr((unsigned char*)" V\r\n", io->stdOut);

      McuShell_SendStatusStr((unsigned char*)"  raw ADC", (unsigned char*)"", io->stdOut);
      McuShell_SendNum16u(adc_raw, io->stdOut); 
      McuShell_SendStr((unsigned char*)" \r\n", io->stdOut);
    }

  }
  
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"battery", (unsigned char*)"Group of battery measurement commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  measure", (unsigned char*)"Measure battery voltage\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  raw", (unsigned char*)"Get raw ADC reading\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t McuBattery_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "battery help")==0) {
    *handled = TRUE;
    return PrintHelp(io);

  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "battery status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);

  } else if (McuUtility_strcmp((char*)cmd, "battery measure")==0) {
    *handled = TRUE;
    uint16_t voltage_mv;
    if( McuBattery_MeasureVoltage(&voltage_mv) == ERR_OK) {
      McuShell_SendStr((unsigned char*)"Battery Voltage: ", io->stdOut);
      McuShell_SendNum16u(voltage_mv / 1000, io->stdOut);
      McuShell_SendStr((unsigned char*)".", io->stdOut);
      uint16_t decimal = (voltage_mv % 1000) / 10;  /* Get 2 decimal places */
      if (decimal < 10) {
        McuShell_SendStr((unsigned char*)"0", io->stdOut);  /* Leading zero for single digit */
      }
      McuShell_SendNum16u(decimal, io->stdOut);
      McuShell_SendStr((unsigned char*)" V\r\n", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)"Measurement failed\r\n", io->stdErr);
    }
    
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "battery raw")==0) {
    *handled = TRUE;
    
    uint16_t adc_raw;
    uint8_t error = McuBattery_GetRawADC(&adc_raw);
    
    if (error == ERR_OK) {
      uint32_t adc_voltage_mv = ((uint32_t)adc_raw * McuBattery_CONFIG_ADC_VREF_MV) / McuBattery_CONFIG_ADC_MAX_VALUE;
      
      McuShell_SendStr((unsigned char*)"Raw ADC: ", io->stdOut);
      McuShell_SendNum16u(adc_raw, io->stdOut);
      McuShell_SendStr((unsigned char*)" (", io->stdOut);
      McuShell_SendNum32u(adc_voltage_mv, io->stdOut);
      McuShell_SendStr((unsigned char*)" mV at ADC input)\r\n", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)"ADC reading failed, error: ", io->stdErr);
      McuShell_SendNum8u(error, io->stdErr);
      McuShell_SendStr((unsigned char*)"\r\n", io->stdErr);
    }
    
    return ERR_OK;
  }
  
  return ERR_OK; /* not handled */
}
