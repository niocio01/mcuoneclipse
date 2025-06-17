/*
 * Copyright (c) 2025, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "McuLib.h"
#include "McuGenericI2C.h"
#include <stdbool.h>
#include "McuUtility.h"
#include "McuWait.h"
#include "vl53l5cx_api.h"
#include "McuVL53L5CX.h"
#include "McuGPIO.h"
#include "McuLog.h"

#define McuVL53L5CX_I2C_DEVICE_ADDRESS   (0x29) /* 7bit I2C address, not-shifted value */

uint8_t 				status, loop, isAlive, isReady, i;
uint32_t 				integration_time_ms;
VL53L5CX_Configuration 	Dev;			/* Sensor configuration */
VL53L5CX_ResultsData 	Results;		/* Results data from VL53L5CX */
bool init_done = false;					/* flag to indicate if the sensor has been initialized */

static McuGPIO_Handle_t VL53L5CX_LPn_pin_handle; /* GPIO pin handle for LPn pin */

static void init_LPn_pin(void) {
	McuGPIO_Config_t config;
  
	McuGPIO_GetDefaultConfig(&config);
	config.hw.pin = 9; /* GPIO9, D9/pin 21 on feather*/
	config.isInput = false;
	config.isHighOnInit = false; /* enable is LOW active. Have it disabled at the beginning. */
	config.hw.pull = McuGPIO_PULL_DOWN; /* pull down to avoid floating pin */
	VL53L5CX_LPn_pin_handle = McuGPIO_InitGPIO(&config);
	if (VL53L5CX_LPn_pin_handle==NULL) {
	  for(;;){}
	}
  }


uint8_t McuVL53L5CX_Init(void)
{
    Dev.platform.address = McuVL53L5CX_I2C_DEVICE_ADDRESS;

	init_LPn_pin();
	McuGPIO_SetHigh(VL53L5CX_LPn_pin_handle);

    /*********************************/
	/*   Power on sensor and init    */
	/*********************************/

	/* (Optional) Check if there is a VL53L5CX sensor connected */
	status = vl53l5cx_is_alive(&Dev, &isAlive);
	if(!isAlive || status)
	{
		McuLog_error("VL53L5CX not detected at requested address. status: %i, isAlive: %i \n", status, isAlive);
		return status;
	}
	McuLog_info("VL53L5CX isAlive");

}

uint8_t McuVL53L5CX_Programm(void)
{
	if (!isAlive) {
		return ERR_DISABLED; // Error: not initialized
	}

	/* (Mandatory) Init VL53L5CX sensor */
	status = vl53l5cx_init(&Dev);
	if(status)
	{
		McuLog_error("VL53L5CX sensor init failed. status: %i\n", status);
		return status;
	}
	McuLog_info("VL53L5CX sensor init done.");

	// printf("VL53L5CX ULD ready ! (Version : %s)\n",
	//		VL53L5CX_API_REVISION);

	// status = vl53l5cx_set_ranging_frequency_hz(&Dev, 1);
	// if(status)
	// {
	// 	return status;
	// }

	status = vl53l5cx_set_resolution(&Dev, VL53L5CX_RESOLUTION_8X8);
	if(status)
	{
		return status;
	}

	init_done = true; /* set the init done flag */
	return 0;
}

uint8_t McuVL53L5CX_StartRanging(void)
{
	if (!init_done) {
		// printf("VL53L5CX not initialized\n");
		return ERR_DISABLED; // Error: not initialized
	}
	status = vl53l5cx_start_ranging(&Dev);
	if(status)
	{
		// printf("VL53L5CX start ranging failed\n");
		return status;
	}
	return 0;
}

uint8_t McuVL53L5CX_GetRangingData(VL53L5CX_ResultsData *results)
{
	if (!init_done) {
		// printf("VL53L5CX not initialized\n");
		return ERR_DISABLED; // Error: not initialized
	}
	status = vl53l5cx_get_ranging_data(&Dev, results);
	if(status)
	{
		// printf("VL53L5CX get ranging data failed\n");
		return status;
	}
	return 0;
}

bool McuVL53L5CX_IsDataReady(void) {
	if (!init_done) 
	{
		// printf("VL53L5CX not initialized\n");
		return ERR_DISABLED; // Error: not initialized
	}

    uint8_t dataReady = 0;

    uint8_t result = vl53l5cx_check_data_ready(&Dev, &dataReady);
    if (result == 0) return dataReady != 0;

    return false;
}
