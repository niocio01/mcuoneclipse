/*
 * Copyright (c) 2019-2023, Nico Zuber
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ConfigurationManager.h"

#include "platform.h"
#include "McuLib.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuRTOS.h"
#include "RNet/RApp.h"
#include "RNet_App.h"
#include "Radio.h"
#include "ConfigTypes.h"
#include "McuNRF24L01.h"
#include "McuFlash.h"
#include "hardware/flash.h"

#if PL_CONFIG_USE_SHELL
  #include "shell.h"
#endif

#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
  #include "MinIniKeys.h"
#endif

/* Task notification flags - similar to Radio.c pattern */
#define CONFIG_FLAG_CONNECTION_SETUP_RECEIVED    (1<<0)  /* Connection setup message received */
#define CONFIG_FLAG_GENERAL_CONFIG_RECEIVED      (1<<1)  /* General configuration received */
#define CONFIG_FLAG_SENSOR_CONFIG_RECEIVED       (1<<2)  /* Sensor configuration received */
#define CONFIG_FLAG_HASH_RESPONSE_RECEIVED       (1<<3)  /* Hash check response received */
#define CONFIG_FLAG_REQUEST_HASH_CHECK           (1<<4)  /* Request hash check procedure */
#define CONFIG_FLAG_REQUEST_CONFIGURATION        (1<<5)  /* Request configuration from aggregator */
#define CONFIG_FLAG_TIMEOUT                      (1<<6)  /* Configuration timeout occurred */

/* Storage version for persistent data */
#define CONFIG_STORAGE_VERSION                   (0x01)
#define CONFIG_FLASH_ADDRESS                     McuMinINI_CONFIG_FLASH_NVM_ADDR_START
#define CONFIG_FLASH_SIZE                        FLASH_SECTOR_SIZE

static TaskHandle_t configTaskHandle = NULL;
static CONFIG_PersistentStorage config = {0};

static void configTask(void *pv) {
  uint32_t notifiedValue = 0;  
  
  for(;;) {
    /* Wait for notification */
    if (xTaskNotifyWait(0x00,          /* Don't clear any bits on entry */
                        UINT32_MAX,   /* Clear all bits on exit */
                        &notifiedValue,
                        portMAX_DELAY) == pdTRUE) {
      /* Process notification flags */
      if (notifiedValue & CONFIG_FLAG_CONNECTION_SETUP_RECEIVED) {
        /* Handle connection setup received */
      }
      if (notifiedValue & CONFIG_FLAG_GENERAL_CONFIG_RECEIVED) {
        /* Handle general configuration received */
      }
      if (notifiedValue & CONFIG_FLAG_SENSOR_CONFIG_RECEIVED) {
        /* Handle sensor configuration received */
      }
      if (notifiedValue & CONFIG_FLAG_HASH_RESPONSE_RECEIVED) {
        /* Handle hash response received */
        /* TODO: Get the actual response message from radio reception */
      }
      if (notifiedValue & CONFIG_FLAG_REQUEST_HASH_CHECK) {
        /* Send hash check message */
        if (CONFIG_SendChecksumCheckMessage() == ERR_OK) {
          McuLog_info("Hash check message sent successfully");
          /* Response will be handled when CONFIG_FLAG_HASH_RESPONSE_RECEIVED is set */
        } else {
          McuLog_error("Failed to send hash check message");
        }
      }
      if (notifiedValue & CONFIG_FLAG_REQUEST_CONFIGURATION) {
        /* Request configuration from aggregator */
      }
      if (notifiedValue & CONFIG_FLAG_TIMEOUT) {
        /* Handle timeout event */
      }
    }
  }
}

uint8_t CONFIG_SendChecksumCheckMessage(void)
{
  CONFIG_HashCheckMessage hashMsg = {0};
  
  /* Get hardware ID (you may need to implement this based on your hardware) */
  // TODO: Get actual hardware ID from chip
  memset(hashMsg.hardwareId, 0xAA, CONFIG_HARDWARE_ID_SIZE); /* Placeholder */
  
  hashMsg.sensorId = config.configData.sensorId;

  /* Calculate the Checksums */
  CONFIG_CalculateChecksum(&config.configData.general, &hashMsg.generalConfigHash);
  CONFIG_CalculateChecksum(&config.configData.sensIR, &hashMsg.sensorIRConfigHash);
  CONFIG_CalculateChecksum(&config.configData.sensTOF, &hashMsg.sensorTOFConfigHash);
  CONFIG_CalculateChecksum(&config.configData.sensUS, &hashMsg.sensorUSConfigHash);

  uint8_t txAddr[5];
  McuNRF24L01_GetRxAddress(CONFIG_RF_PIPE_NR, txAddr, 5);
  RNWK_ShortAddrType dstAddr = txAddr[4];

  uint8_t result = RAPP_SendPayloadDataBlock((uint8_t*)&hashMsg, sizeof(hashMsg), MSG_TYPE_CONFIG_CHECK_HASHES, RNWK_ADDR_BROADCAST, RPHY_PACKET_FLAGS_NONE);
  if (result != ERR_OK) {
    return ERR_FAILED;
  }
  return ERR_OK;
}

uint8_t CONFIG_CalculateChecksum(const void *config, uint16_t *checksum)
{
  if (config == NULL || checksum == NULL) {
    return ERR_PARAM_DATA;
  }

  const uint8_t *data = (const uint8_t *)config;
  *checksum = 0x1234; /* Initial seed value */
  
  /* Simple checksum algorithm - XOR with rotation */
  for (uint8_t i = 0; i < sizeof(uint16_t); i++) {
    *checksum ^= data[i];
    *checksum = (*checksum << 1) | (*checksum >> 15); /* 16-bit rotation */
    *checksum ^= (i + 1); /* Add position dependency */
  }
  
  return ERR_OK;
}


void CONFIG_Notify(uint32_t flags) {
  if (configTaskHandle != NULL && flags != 0) {
    (void)xTaskNotify(configTaskHandle, flags, eSetBits);
  }
}

void CONFIG_NotifyFromISR(uint32_t flags, BaseType_t *pxHigherPriorityTaskWoken) {
  if (configTaskHandle != NULL && flags != 0) {
    (void)xTaskNotifyFromISR(configTaskHandle, flags, eSetBits, pxHigherPriorityTaskWoken);
  }
}

uint8_t CONFIG_RequestHashCheck(void) {
  /* Request hash check from outside task context - triggers configTask to send the message */
  if (configTaskHandle != NULL) {
    CONFIG_Notify(CONFIG_FLAG_REQUEST_HASH_CHECK);
    return ERR_OK;
  }
  return ERR_NOTAVAIL;
}

uint8_t CONFIG_SendHashCheck(void) {
  /* Send hash check message directly (can be called from any context) */
  return CONFIG_SendChecksumCheckMessage();
}

void getDefaultConfig(void) {
  /* Set default configuration values */

  memset(&config, 0, sizeof(CONFIG_PersistentStorage));
  config.memoryMarker = 0xDEADBEEF;
  config.version = CONFIG_STORAGE_VERSION;
  config.configData.sensorId = CONFIG_SENSOR_ID_UNASSIGNED;

  uint8_t channel = 81;
  // McuNRF24L01_GetChannel(&channel);
  // config.configData.rfConfig.rfChannel = channel;

  /* TODO: get other default settings */
}

uint8_t CONFIG_Save(bool forceSave)
{
  /* Check if current config is different from the one in flash, and if so, save it. To prevent unnecessary writes */
  if (memcmp(&config, (void *)CONFIG_FLASH_ADDRESS, sizeof(CONFIG_PersistentStorage)) != 0 || forceSave)
  {  
    /* Local flash buffer to prevent reading over the persistentStorage memory when writitng to Flash */
    uint8_t flashBuffer[FLASH_SECTOR_SIZE];
    
    memset(flashBuffer, 0xFF, sizeof(flashBuffer));
    memcpy(flashBuffer, &config, sizeof(CONFIG_PersistentStorage));

    if(McuFlash_Program((void *)CONFIG_FLASH_ADDRESS, flashBuffer, FLASH_SECTOR_SIZE) != ERR_OK) {
      McuLog_error("Failed to write configuration to flash.");
      return ERR_FAILED;
    }
  }  
  return ERR_OK;
}

void CONFIG_SetShellPromtString()
{
  char mcuShell_PromptString[15];
  char sensorIdStr[4];
  McuUtility_strcpy(mcuShell_PromptString, sizeof(mcuShell_PromptString), (unsigned char *)"pico");
  McuUtility_Num8uToStr(sensorIdStr, sizeof(sensorIdStr), config.configData.sensorId);
  McuUtility_strcat(mcuShell_PromptString, sizeof(mcuShell_PromptString), (unsigned char *)sensorIdStr);
  McuShell_SetHostname(mcuShell_PromptString);
}

uint8_t CONFIG_Init(void) {
  /* Load configuration from flash */
  McuFlash_RegisterMemory((void*)CONFIG_FLASH_ADDRESS, sizeof(CONFIG_PersistentStorage));
  McuFlash_Read((void*)CONFIG_FLASH_ADDRESS, &config, sizeof(CONFIG_PersistentStorage));
  if (config.memoryMarker != 0xDEADBEEF || config.version != CONFIG_STORAGE_VERSION) {
    /* Invalid or incompatible configuration - use defaults */
    McuLog_info("No valid configuration found, using defaults");
    getDefaultConfig();

    return CONFIG_Save(false);
  }
  
  CONFIG_SetShellPromtString();

  CONFIG_Save(true);

  if (xTaskCreate(
    configTask, 
    "ConfigTask", 
    4*1024/sizeof(StackType_t), /* task stack size */
    (void*)NULL, /* optional task startup argument */
    tskIDLE_PRIORITY+2,  /* initial priority */
    (TaskHandle_t*)&configTaskHandle /* optional task handle to create */
  ) != pdPASS) 
  {
    McuLog_fatal("failed creating task");
    for(;;){} /* error! probably out of memory */
  }
  
  return ERR_OK;
}

#if PL_CONFIG_USE_SHELL
static uint8_t CONFIG_PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];
  
  McuShell_SendStatusStr((unsigned char*)"config", (unsigned char*)"Configuration Manager status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  sensor ID", (unsigned char*)"", io->stdOut);
  McuUtility_Num8uToStr(buf, sizeof(buf), config.configData.sensorId);
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  
  McuShell_SendStatusStr((unsigned char*)"  version", (unsigned char*)"", io->stdOut);
  McuUtility_Num8uToStr(buf, sizeof(buf), config.version);
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  
  return ERR_OK;
}

static uint8_t CONFIG_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"config", (unsigned char*)"Configuration manager commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  check", (unsigned char*)"Send configuration hash check message\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP) == 0 || McuUtility_strcmp((char*)cmd, "config help") == 0) {
    res = CONFIG_PrintHelp(io);
    *handled = TRUE;
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS) == 0) || (McuUtility_strcmp((char*)cmd, "config status") == 0)) {
    res = CONFIG_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, "config check") == 0) {
    res = CONFIG_SendHashCheck();
    if (res == ERR_OK) {
      McuShell_SendStr((unsigned char*)"Configuration hash check message sent\r\n", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)"Failed to send configuration hash check\r\n", io->stdErr);
    }
    *handled = TRUE;
  }
  return res;
}
#endif

uint8_t CONFIG_Deinit(void) {
  /* Delete the configuration task */
  if (configTaskHandle != NULL) {
    vTaskDelete(configTaskHandle);
    configTaskHandle = NULL;
  }
  return ERR_OK;
}