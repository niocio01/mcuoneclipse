/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */



#include "McuVL53L5CX.h"
#include "McuGenericI2C.h"
#include "McuWait.h"


uint8_t VL53L5CX_RdByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_value)
{

	/* implemented by customer. This function returns 0 if OK */

	// swap word order of address
	RegisterAdress = ((RegisterAdress & 0xFF00) >> 8) | ((RegisterAdress & 0x00FF) << 8);

	return McuGenericI2C_ReadAddress(p_platform->address, (uint8_t*)&RegisterAdress, sizeof(RegisterAdress), p_value, sizeof(*p_value));
}

uint8_t VL53L5CX_WrByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value)
{
	uint8_t status = 255;

	/* implemented by customer. This function returns 0 if OK */

	// swap word order of address
	RegisterAdress = ((RegisterAdress & 0xFF00) >> 8) | ((RegisterAdress & 0x00FF) << 8);

	return McuGenericI2C_WriteAddress(p_platform->address, (uint8_t*)&RegisterAdress, sizeof(RegisterAdress), &value, sizeof(value));
}

uint8_t VL53L5CX_WrMulti(
	VL53L5CX_Platform *p_platform,
	uint16_t RegisterAdress,
	uint8_t *p_values,
	uint32_t size)
{	
	/* implemented by customer. This function returns 0 if OK */

	// swap word order of address
	RegisterAdress = ((RegisterAdress & 0xFF00) >> 8) | ((RegisterAdress & 0x00FF) << 8);

	uint32_t remaining = size;
	uint32_t offset = 0;
	uint32_t chunkSize = McuGenericI2C_WRITE_BUFFER_SIZE-sizeof(RegisterAdress);
	uint8_t *p_chunk = p_values;
	uint8_t status = ERR_OK;
	while (remaining > 0) {
		if (remaining < chunkSize) {
			chunkSize = remaining;
		}
		status = McuGenericI2C_WriteAddress(p_platform->address, (uint8_t*)&RegisterAdress, sizeof(RegisterAdress), p_chunk, chunkSize);
		if (status != 0) {
			return status;
		}
		RegisterAdress += chunkSize;
		p_chunk += chunkSize;
		remaining -= chunkSize;
	}
}

uint8_t VL53L5CX_RdMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{	
	/* implemented by customer. This function returns 0 if OK */

	// swap word order of address
	RegisterAdress = ((RegisterAdress & 0xFF00) >> 8) | ((RegisterAdress & 0x00FF) << 8);

	return McuGenericI2C_ReadAddress(p_platform->address, (uint8_t*)&RegisterAdress, sizeof(RegisterAdress), p_values, size);	
}

uint8_t VL53L5CX_Reset_Sensor(
		VL53L5CX_Platform *p_platform)
{
	uint8_t status = 0;
	
	/* (Optional) Need to be implemented by customer. This function returns 0 if OK */
	
	/* Set pin LPN to LOW */
	/* Set pin AVDD to LOW */
	/* Set pin VDDIO  to LOW */
	VL53L5CX_WaitMs(p_platform, 100);

	/* Set pin LPN of to HIGH */
	/* Set pin AVDD of to HIGH */
	/* Set pin VDDIO of  to HIGH */
	VL53L5CX_WaitMs(p_platform, 100);

	return status;
}

void VL53L5CX_SwapBuffer(
		uint8_t 		*buffer,
		uint16_t 	 	 size)
{
	uint32_t i, tmp;
	
	/* Example of possible implementation using <string.h> */
	for(i = 0; i < size; i = i + 4) 
	{
		tmp = (
		  buffer[i]<<24)
		|(buffer[i+1]<<16)
		|(buffer[i+2]<<8)
		|(buffer[i+3]);
		
		memcpy(&(buffer[i]), &tmp, 4);
	}
}	

uint8_t VL53L5CX_WaitMs(
		VL53L5CX_Platform *p_platform,
		uint32_t TimeMs)
{
	McuWait_Waitms(TimeMs);
	
	return 0;
}
