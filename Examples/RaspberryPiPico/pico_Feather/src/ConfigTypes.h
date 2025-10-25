/*
 * Copyright (c) 2019-2023, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __CONFIG_PROTOCOL_H_
#define __CONFIG_PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "RNet_AppConfig.h"

/* Define static_assert for C if not available */
#ifndef static_assert
#define static_assert _Static_assert
#endif

/* Configuration protocol constants */
#define CONFIG_PROTOCOL_VERSION         (0x01)
#define CONFIG_COORDINATE_SCALE         (100)  /* Scale factor for float to int16 conversion */
#define CONFIG_HARDWARE_ID_SIZE         (6)    /* 6 bytes hardware ID (last 6 bytes of Pico unique chip ID) */
#define CONFIG_RF_PIPE_ADDRESS_SIZE     (5)    /* NRF24L01+ pipe address size in bytes */
#define CONFIG_SENSOR_ID_UNASSIGNED     (0)    /* Sensor not yet assigned an ID */

/* Radio payload constraints (NRF24L01+ with RNET stack overhead and corruption fix) */
#define CONFIG_MAX_RADIO_PAYLOAD        (23)   /* 32 - 8 (RNET) - 1 (corruption fix) */

/* Time management constants */
#define CONFIG_MINUTES_PER_DAY          (1440) /* 24 * 60 minutes */
#define CONFIG_INVALID_TIME             (0xFFFF) /* Invalid time marker */

/* Configuration message types for radio protocol */
typedef enum {
  MSG_TYPE_CONFIG_CHECK_HASHES = 0x60, /* Request to check configuration hashes */
  MSG_TYPE_CONFIG_CHECK_HASHES_RESP,   /* Response to configuration hash check request */

  MSG_TYPE_CONFIG_REQUEST_CONFIG,      /* Request to a specific configuration */

  MSG_TYPE_CONFIG_RESPONSE_RF,        /* Response to RF configuration request */
  MSG_TYPE_CONFIG_RESPONSE_GENERAL,   /* Response to general configuration request */
  MSG_TYPE_CONFIG_RESPONSE_SENS_IR,   /* Response to infrared sensor configuration request */
  MSG_TYPE_CONFIG_RESPONSE_SENS_TOF,  /* Response to TOF sensor configuration request */
  MSG_TYPE_CONFIG_RESPONSE_SENS_US,   /* Response to US sensor configuration request */
} CONFIG_MessageType;

/* Configuration bitfield used for tracking received configurations or hash checking */
typedef struct {
  uint8_t generalConfig   : 1;  /* General config */
  uint8_t rf              : 1;  /* RF config */
  uint8_t sensorIR        : 1;  /* Infrared sensor config */
  uint8_t sensorTOF       : 1;  /* TOF sensor config */
  uint8_t sensorUS        : 1;  /* Ultrasonic sensor config */
} CONFIG_Types;

/* Active days bitfield structure for day-of-week masks */
typedef struct {
  uint8_t monday      : 1;  /* Monday = bit 0 */
  uint8_t tuesday     : 1;  /* Tuesday = bit 1 */
  uint8_t wednesday   : 1;  /* Wednesday = bit 2 */
  uint8_t thursday    : 1;  /* Thursday = bit 3 */
  uint8_t friday      : 1;  /* Friday = bit 4 */
  uint8_t saturday    : 1;  /* Saturday = bit 5 */
  uint8_t sunday      : 1;  /* Sunday = bit 6 */
} CONFIG_DayMask;


/* ------------------------------------------- RF Message type definitions ----------------------------------------------- */

/* Configuration hash check structure sent at startup to verify configuration (sensor → aggregator) */
typedef struct {
  uint8_t hardwareId[CONFIG_HARDWARE_ID_SIZE];   /* Target hardware ID */
  uint8_t sensorId;                              /* ID of the sensor sending the message */
  uint16_t generalConfigHash;                    /* Current stored general configuration hash */
  uint16_t sensorIRConfigHash;                   /* Current stored IR sensor configuration hash */
  uint16_t sensorTOFConfigHash;                  /* Current stored TOF sensor configuration hash */
  uint16_t sensorUSConfigHash;                   /* Current stored US sensor configuration hash */
} CONFIG_HashCheckMessage;  /* 23 bytes total (6 + 1 + 4*2) */

/* Configuration response structure (aggregator → sensor) */
typedef struct {
  uint8_t hardwareId[CONFIG_HARDWARE_ID_SIZE];   /* Target hardware ID */
  uint8_t sensorId;                              /* ID of the sensor receiving the message */
  CONFIG_Types hashStatus;                       /* Bitfield indicating which configs are up-to-date */
} CONFIG_HashResponseMessage;

typedef struct {
  uint8_t ConfigType;                            /* Configuration type to request */
} CONFIG_RequestConfig;

/* General configuration data */
typedef struct {
  uint16_t dailyStartTime;                       /* Daily start time in minutes from midnight (0-1439) */
  uint16_t dailyStopTime;                        /* Daily stop time in minutes from midnight (0-1439) */
  CONFIG_DayMask activeDays;                     /* Days of week when sensor should be active */
  int16_t positionX;                             /* X position * CONFIG_COORDINATE_SCALE, not needed for detection messages */
  int16_t positionY;                             /* Y position * CONFIG_COORDINATE_SCALE, not needed for detection messages */
  uint8_t departmentId;                          /* Department ID, not needed for detection messages */
} CONFIG_GeneralConf;

/* Connection setup data - establishes HW_ID → Sensor ID mapping */
typedef struct {
  uint8_t rfChannel;                                      /* RF channel to use */
  uint8_t rfDataPipeNr;                                   /* RF pipe number for data messages */
  uint16_t configCheckInterval;                           /* Configuration checking interval in seconds (max 65535, which is 18h 12min 15s) */
  uint8_t  configCheckTimeOffsetSec;                      /* Offset for configuration checking in seconds from each minute */
} CONFIG_RFConf;

/* Infrared sensor configuration */
typedef struct {
  uint8_t measuermentInterval; /* Measurement interval in seconds */
  uint8_t motionThreshold;     /* Motion detection threshold (0-255) */
  uint8_t presenceThreshold;   /* Presence detection threshold (0-255) */
} CONFIG_SensIRConf;

/* TOF sensor configuration */
typedef struct {
  uint8_t measuermentInterval; /* Measurement interval in seconds */
  uint8_t detectionRange;      /* Detection range in centimeters */
  uint8_t sensitivity;         /* Sensitivity setting (0-255) */
} CONFIG_SensTOFConf;

/* Ultrasonic sensor configuration */
typedef struct {
  uint8_t measuermentInterval; /* Measurement interval in seconds */
  uint8_t maxDistance;         /* Maximum detection distance in meters */
  uint8_t triggerLevel;        /* Trigger level threshold (0-255) */
} CONFIG_SensUSConf;


/* ------------------------------------------- Size checks ----------------------------------------------- */

/* Check complete message sizes */
static_assert (sizeof(CONFIG_HashCheckMessage) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_HashCheckMessage exceeds radio payload limit");
static_assert (sizeof(CONFIG_HashResponseMessage) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_HashResponseMessage exceeds radio payload limit");
static_assert (sizeof(CONFIG_RequestConfig) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_RequestConfig exceeds radio payload limit");
static_assert (sizeof(CONFIG_SensIRConf) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_SensIRData exceeds radio payload limit");
static_assert (sizeof(CONFIG_SensTOFConf) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_SensTOFData exceeds radio payload limit");
static_assert (sizeof(CONFIG_SensUSConf) <= CONFIG_MAX_RADIO_PAYLOAD, "CONFIG_SensUSData exceeds radio payload limit");

/* Validate critical structure alignments */
static_assert (sizeof(CONFIG_Types) == 1, "CONFIG_Types must be exactly 1 byte");
static_assert (sizeof(CONFIG_DayMask) == 1, "CONFIG_DayMask must be exactly 1 byte");


#endif /* __CONFIG_PROTOCOL_H_ */