/*
 * Copyright (c) 2019-2023, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __CONFIGURATION_MANAGER_H_
#define __CONFIGURATION_MANAGER_H_

#include "platform.h"
#include "McuTimeDate.h"
#include "ConfigTypes.h"  /* Shared Configuration data types */
#include "hardware/flash.h"

#include <stdint.h>
#include <stdbool.h>

/* Define static_assert for C if not available */
#ifndef static_assert
#define static_assert _Static_assert
#endif

/* Configuration Manager specific constants */
#define CONFIG_REQUEST_TIMEOUT_S        (5)  /* Timeout for configuration requests in seconds */
#define CONFIG_RF_PIPE_NR               (0)  /* which RF pipe number to use for configuration messages */

/* Predefined day mask constants */
extern const CONFIG_DayMask CONFIG_DAY_NONE;
extern const CONFIG_DayMask CONFIG_DAY_WEEKDAYS;
extern const CONFIG_DayMask CONFIG_DAY_WEEKEND;
extern const CONFIG_DayMask CONFIG_DAY_ALL;

typedef struct {
  uint8_t sensorId;           /* Assigned sensor ID */
  CONFIG_GeneralConf general; /* General configuration data */
  CONFIG_RFConf rfConfig;     /* RF configuration data */
  CONFIG_SensIRConf sensIR;   /* Infrared sensor configuration data */
  CONFIG_SensTOFConf sensTOF; /* TOF sensor configuration data */
  CONFIG_SensUSConf sensUS;   /* Ultrasonic sensor configuration data */
} CONFIG_Data;


typedef struct {
  uint32_t memoryMarker;  /* Marker to indicate valid memory */
  uint8_t version;       /* Storage format version */
  CONFIG_Data configData; /* Actual configuration data */
} CONFIG_PersistentStorage;



typedef struct
{
  TIMEREC time;
  DATEREC date;
} CONFIG_DateTime;

/* Public API functions */

/**
 * \brief Initialize the configuration manager
 * \return ERR_OK on success
 */
uint8_t CONFIG_Init(void);

void CONFIG_SetShellPromtString();

/**
 * \brief Save the current configuration to flash
 * \param forceSave If true, force save even if no changes detected
 * \return ERR_OK on success
 */
uint8_t CONFIG_Save(bool forceSave);

/**
 * \brief Deinitialize the configuration manager
 * \return ERR_OK on success
 */
uint8_t CONFIG_Deinit(void);

/**
 * \brief Request configuration hash check (non-blocking)
 * Triggers the configuration task to perform hash check procedure
 * \return ERR_OK if request was sent, ERR_NOTAVAIL if task not ready
 */
uint8_t CONFIG_RequestHashCheck(void);

/**
 * \brief Send configuration checksum message
 * \return ERR_OK on success
 */
uint8_t CONFIG_SendChecksumCheckMessage(void);

/**
 * \brief Get the assigned sensor ID 
 * \param sensorId Pointer to store the sensor ID
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetSensorID(uint8_t *sensorId);


/**
 * \brief Request General Configuration from aggregator
 * \param configTypes Bitfield indicating which configuration types to request
 * \return ERR_OK on success ERR_TIMEOUT if no response within timeout
 */
uint8_t CONFIG_GetConfig(CONFIG_Types configTypes);


/**
 * \brief Get which configuration types are verified by the aggregator
 * \param configTypes Pointer to store verified configuration types
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetVerificationStatus(CONFIG_Types *configTypes);


/**
 * \brief Get time when configuration verification was performed
 * \param configType Configuration type to query
 * \param dateTime Pointer to store verification date and time
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetVerificationDateTime(uint8_t configType, CONFIG_DateTime *dateTime);

/**
 * \brief Calculate configuration checksum from any configuration structures
 * \param config Pointer to configuration structure
 * \param checksum Buffer to store calculated checksum
 * \return ERR_OK on success
 */
uint8_t CONFIG_CalculateChecksum(const void *config, uint16_t *checksum);


/**
 * \brief Get next wake-up time for RTC alarm
 * \param dateTime Pointer to store next wake date and time
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetNextWakeTime(CONFIG_DateTime *dateTime);

/**
 * \brief Get daily start time from configuration
 * \param time Pointer to store the start time
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetDailyStartTime(TIMEREC *time);

/**
 * \brief Get daily stop time from configuration
 * \param time Pointer to store the stop time
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetDailyStopTime(TIMEREC *time);

/**
 * \brief Get daily stop time from configuration
 * \param time Pointer to store the stop time
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetDailyStopTime(TIMEREC *time);

/**
 * \brief Get active days from configuration
 * \param dayMask Pointer to store the active days bitfield
 * \return ERR_OK on success
 */
uint8_t CONFIG_GetActiveDays(CONFIG_DayMask *dayMask);

#if PL_CONFIG_USE_SHELL
/**
 * \brief Shell command parser for configuration manager
 */
uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

#endif /* __CONFIGURATION_MANAGER_H_ */