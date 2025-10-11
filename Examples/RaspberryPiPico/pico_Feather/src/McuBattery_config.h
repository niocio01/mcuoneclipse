/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCU_BATTERY_CONFIG_H_
#define MCU_BATTERY_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO Configuration */
#define McuBattery_CONFIG_VBAT_GPIO_PIN              (27)    /* GPIO27 (A1, ADC1) */
#define McuBattery_CONFIG_VBAT_ADC_INPUT             (1)     /* ADC input 1 */
#define McuBattery_CONFIG_VIN_GPIO_PIN               (26)    /* GPIO26 (A0) */


/* Voltage Divider Configuration 
 * voltage divider with R1=10k and R2=10k (50% divider):
 * Battery voltage range: 2.5V - 4.2V
 * ADC input range: 1.25V - 2.1V (safe for 3.3V ADC)
 */
#define McuBattery_CONFIG_VOLTAGE_DIVIDER_R1        (10000) /* Upper resistor in ohms (10k) */
#define McuBattery_CONFIG_VOLTAGE_DIVIDER_R2        (10000) /* Lower resistor in ohms (10k) */

/* ADC Reference and Resolution */
#define McuBattery_CONFIG_ADC_VREF_MV              (3300)   /* ADC reference voltage in mV (3.3V) */
#define McuBattery_CONFIG_ADC_RESOLUTION           (12)     /* 12-bit ADC resolution */
#define McuBattery_CONFIG_ADC_MAX_VALUE            (4095)   /* 2^12 - 1 */

/* Measurement Configuration */
#define McuBattery_CONFIG_NUM_SAMPLES              (16)     /* Number of samples for averaging */

/* Battery Voltage Thresholds (in mV) */
#define McuBattery_CONFIG_BATTERY_MAX_MV           (4200)   /* Fully charged LiPo voltage */
#define McuBattery_CONFIG_BATTERY_FULL_MV          (4000)   /* Charging threshold voltage */
#define McuBattery_CONFIG_BATTERY_GOOD_MV          (3600)   /* Good battery voltage */
#define McuBattery_CONFIG_BATTERY_LOW_MV           (3300)   /* Low battery warning threshold */
#define McuBattery_CONFIG_BATTERY_MIN_MV         (3000)   /* Empty/cutoff voltage */

/* Calibration */
#define McuBattery_CONFIG_CALIBRATION_OFFSET       (0)      /* Calibration offset in mV */
#define McuBattery_CONFIG_CALIBRATION_GAIN         (1)      /* Calibration scale factor * 1000 */

#ifdef __cplusplus
}
#endif

#endif /* MCU_BATTERY_CONFIG_H_ */