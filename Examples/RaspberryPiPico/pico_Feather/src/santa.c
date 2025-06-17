/*
 * Copyright (c) 2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if 0
#include "santa.h"
#include "McuShell.h"
#include "McuUtility.h"
#include "McuLog.h"
#if PL_CONFIG_HAS_BATTERY_ADC
  #include "Battery.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "Buzzer.h"
#endif
#include "buttons.h"
#include "leds.h"
#include "McuLED.h"
#if PL_CONFIG_USE_DRIVE
  #include "Drive.h"
#endif
#if PL_CONFIG_HAS_LCD
  #include "LCD.h"
#endif

#if McuLib_CONFIG_CPU_IS_ESP32
  static Santa_RobotMoveStatus_e Santa_RobotMoveStatus = Santa_MOVE_STATUS_UNKNOWN;
#endif
#if PL_IS_INTRO_ZUMO_K22
  #define SANTA_TIMEOUT_PERIOD_MS   (1000)
  static TimerHandle_t timeoutTimer; /* timer for timeout and to stop robot in case of no communication */
#endif

#if PL_IS_INTRO_ZUMO_K22
void Santa_TimeoutRestart(void) {
  (void)xTimerStart(timeoutTimer, pdMS_TO_TICKS(100));
}

void Santa_TimeoutStop(void) {
  (void)xTimerStop(timeoutTimer, pdMS_TO_TICKS(100));
}

static void vTimerCallbacktTimeout(TimerHandle_t pxTimer) {
  McuLog_trace("timeout timer expired");
#if PL_CONFIG_USE_DRIVE
  if (Santa_GetRobotMoveStatus()!=Santa_MOVE_STATUS_STOPPED) {
    DRV_SetMode(DRV_MODE_STOP);
    RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE, Santa_MOVE_STATUS_STOPPED, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  }
#endif
}
#endif /* #if PL_IS_INTRO_ZUMO_K22 */

Santa_RobotMoveStatus_e Santa_GetRobotMoveStatus(void) {
#if PL_IS_INTRO_ZUMO_K22
  switch(DRV_GetMode()) {
    case DRV_MODE_STOP: return Santa_MOVE_STATUS_STOPPED;
    case DRV_MODE_POS:
    case DRV_MODE_SPEED: return Santa_MOVE_STATUS_MOVING;
    default: return Santa_MOVE_STATUS_UNKNOWN;
  }
#else
  return Santa_RobotMoveStatus;
#endif
}

#if McuLib_CONFIG_CPU_IS_ESP32
void Santa_SetRobotMoveStatus(Santa_RobotMoveStatus_e status) {
  Santa_RobotMoveStatus = status;
}
#endif

static bool Santa_ButtonIsPressed(void) {
#if PL_IS_INTRO_ZUMO_K22
  return BTN_SW2ButtonIsPressed();
#else
  return BTN_IsPressed(BTN_NAV_CENTER);
#endif
}

#if PL_HAS_RADIO
static bool Santa_LedIsOn(void) {
#if PL_IS_INTRO_ZUMO_K22
  return McuLED_Get(LEDS_Right);
#else
  return Leds_Get(LEDS_CONFIG_HAS_RED_LED);
#endif
}
#endif /* PL_HAS_RADIO */

#if PL_HAS_RADIO
static void Santa_SetLed(bool on) {
#if PL_IS_INTRO_ZUMO_K22
  McuLED_Set(LEDS_Right, on);
#else
  if (on) {
    Leds_On(LEDS_CONFIG_HAS_RED_LED);
  } else {
    Leds_Off(LEDS_CONFIG_HAS_RED_LED);
  }
#endif
}
#endif /* PL_HAS_RADIO */

#if PL_IS_INTRO_ZUMO_K22
static void Santa_RobotOnButtonEvent(BTN_Buttons_e button, McuDbnc_EventKinds kind) {
  const char *p = NULL;
  DRV_Mode newMode = DRV_MODE_NONE;
  RAPP_MSG_DataIDType notifyID = RAPP_MSG_TYPE_DATA_ID_NONE;
  uint32_t notifyValue = 0;
  int32_t speedL = -1;
  int32_t speedR = -1;

  if (kind==MCUDBNC_EVENT_PRESSED) {
    switch(button) {
      case BTN_NAV_UP:
        p = "pressed up, 'a'";
        #if PL_CONFIG_USE_DRIVE
          notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
          notifyValue = Santa_MOVE_STATUS_MOVING;
          speedL = 500;
          speedR = 500;
          newMode = DRV_MODE_SPEED;
          Santa_TimeoutRestart();
        #endif
        break;
      case BTN_NAV_DOWN:
        p = "pressed down, 'c'";
        #if PL_CONFIG_USE_DRIVE
          notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
          notifyValue = Santa_MOVE_STATUS_MOVING;
          speedL = -500;
          speedR = -500;
          newMode = DRV_MODE_SPEED;
          Santa_TimeoutRestart();
        #endif
        break;
      case BTN_NAV_LEFT:
        p = "pressed left, 'd'";
        #if PL_CONFIG_USE_DRIVE
          notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
          notifyValue = Santa_MOVE_STATUS_MOVING;
          speedL = -500;
          speedR = 500;
          newMode = DRV_MODE_SPEED;
          Santa_TimeoutRestart();
        #endif
        break;
      case BTN_NAV_RIGHT:
        p = "pressed right, 'b'";
        #if PL_CONFIG_USE_DRIVE
          notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
          notifyValue = Santa_MOVE_STATUS_MOVING;
          speedL = 500;
          speedR = -500;
          newMode = DRV_MODE_SPEED;
          Santa_TimeoutRestart();
        #endif
        break;
      case BTN_NAV_CENTER:
        p = "pressed center, 's'";
        #if PL_CONFIG_USE_DRIVE
          notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
          notifyValue = Santa_MOVE_STATUS_STOPPED;
          newMode = DRV_MODE_STOP;
          Santa_TimeoutStop();
        #endif
        break;
      default:
        p = NULL;
        break;
    } /* switch */
  } else if (kind==MCUDBNC_EVENT_PRESSED_REPEAT) {
    switch(button) {
      case BTN_NAV_UP:
        p = "long up";
        break;
      case BTN_NAV_DOWN:
        p = "long down";
        break;
      case BTN_NAV_LEFT:
        p = "long left";
        break;
      case BTN_NAV_RIGHT:
        p = "long right";
        break;
      case BTN_NAV_CENTER:
        p = "long center";
        break;
      default:
        p = NULL;
        break;
    } /* switch */
  } else if (kind==MCUDBNC_EVENT_LONG_PRESSED_REPEAT) {
    switch(button) {
      case BTN_NAV_UP:
        p = "long repeat up";
        break;
      case BTN_NAV_DOWN:
        p = "long repeat down";
        break;
      case BTN_NAV_LEFT:
        p = "long repeat left";
        break;
      case BTN_NAV_RIGHT:
        p = "long repeat right";
        break;
      case BTN_NAV_CENTER:
        p = "long repeat center";
        break;
      default:
        p = NULL;
        break;
    } /* switch */
  } else if (kind==MCUDBNC_EVENT_RELEASED) {
    switch(button) {
      case BTN_NAV_UP:
        p = "release up";
        break;
      case BTN_NAV_DOWN:
        p = "release down";
        break;
      case BTN_NAV_LEFT:
        p = "release left";
        break;
      case BTN_NAV_RIGHT:
        p = "release right";
        break;
      case BTN_NAV_CENTER:
        p = "release center";
        break;
      default:
        p = NULL;
        break;
    } /* switch */
  } else if (kind==MCUDBNC_EVENT_LONG_RELEASED) {
    switch(button) {
      case BTN_NAV_UP:
        p = "long release up";
        break;
      case BTN_NAV_DOWN:
        p = "long release down";
        break;
      case BTN_NAV_LEFT:
        p = "long release left";
        break;
      case BTN_NAV_RIGHT:
        p = "long release right";
        break;
      case BTN_NAV_CENTER:
        p = "long elease center";
        break;
      default:
        p = NULL;
        break;
    } /* switch */
  } /* if-else */
  #if PL_CONFIG_USE_DRIVE
  if (notifyID!=RAPP_MSG_TYPE_DATA_ID_NONE) {
    RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, notifyID, notifyValue, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  }
  if (speedL!=-1 && speedR!=-1) {
    DRV_SetSpeed(speedL, speedR);
  }
  if (DRV_GetMode()!=newMode) { /* changed mode? */
    DRV_SetMode(newMode);
  }
  #endif
  if (p!=NULL) {
    McuLog_info(p);
  #if PL_CONFIG_USE_BUZZER
    //BUZ_Beep(500, 200);
  #endif
  }
}
#endif /* PL_IS_INTRO_ZUMO_K22 */

#if McuLib_CONFIG_CPU_IS_ESP32
void Santa_ESP32OnButtonEvent(uint32_t buttonBits, McuDbnc_EventKinds kind) {
#if PL_CONFIG_HAS_LCD /* navigation button messages are handled by the LCD module and forwarded if configured as such */
  LCD_OnButtonEvent(buttonBits, kind);
#elif PL_HAS_RADIO /* send directly navigation button messages */
  uint32_t val;

  val = (kind<<16)|buttonBits;
  RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_NAV, val, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
#endif
}
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#if PL_HAS_RADIO
uint8_t Santa_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet) {
  RAPP_MSG_DataIDType msgID;
  uint32_t msgValue;
  uint16_t value16;

  switch(type) {
    /* ------------ General data messages -------------------------------*/
    #if PL_IS_INTRO_ZUMO_K22
    case RAPP_MSG_TYPE_JOYSTICK_BTN:
      *handled =true;
      char button = data[0];
      switch(button) {
        case 'a': Santa_RobotOnButtonEvent(BTN_NAV_UP, MCUDBNC_EVENT_PRESSED); break;
        case 'c': Santa_RobotOnButtonEvent(BTN_NAV_DOWN, MCUDBNC_EVENT_PRESSED); break;
        case 'd': Santa_RobotOnButtonEvent(BTN_NAV_LEFT, MCUDBNC_EVENT_PRESSED); break;
        case 'b': Santa_RobotOnButtonEvent(BTN_NAV_RIGHT, MCUDBNC_EVENT_PRESSED); break;
        case 's': Santa_RobotOnButtonEvent(BTN_NAV_CENTER, MCUDBNC_EVENT_PRESSED); break; /* stop */
        default:
          break;
      }
      break;
    #endif
    /* ------------ Received a request to set a value -------------------*/
    case RAPP_MSG_TYPE_REQUEST_SET_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
      #if PL_CONFIG_USE_LEDS
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled =true;
          value16 = McuUtility_GetValue16LE(&data[2]);
          Santa_SetLed(value16);
          break;
      #endif
        default:
          break;
      } /* switch */
      break;
    /* ------------ Received a Query -> send back response -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
      #if PL_CONFIG_HAS_BATTERY_ADC
        case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
          *handled = true;
          BATT_MeasureBatteryVoltage(&value16);
          RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, (uint32_t)(value16*10), srcAddr, RPHY_PACKET_FLAGS_NONE);
        #if PL_CONFIG_USE_BUZZER
          BUZ_Beep(500, 200);
        #endif
          break;
      #endif
      #if PL_CONFIG_USE_LEDS
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled =true;
          msgValue = Santa_LedIsOn();
          RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
      #endif
      #if PL_CONFIG_USE_BUTTONS
        case RAPP_MSG_TYPE_DATA_ID_BUTTON:
          *handled =true;
          msgValue = Santa_ButtonIsPressed();
          RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
      #endif
      #if PL_IS_INTRO_ZUMO_K22
        case RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE:
          *handled =true;
          msgValue = Santa_GetRobotMoveStatus();
          RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
      #endif
        default:
          break;
      }
      break;
    /* ------------ Received the response to a a Query -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE: /* received data value for request */
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      msgValue = McuUtility_GetValue32LE(&data[2]);
      switch(msgID) {
        case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
          *handled = true;
          McuLog_info("Rx: Battery voltage is %d mV", msgValue);
          break;
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled = true;
          McuLog_info("Rx: LED is %s", msgValue==0?"off":"on");
          break;
        case RAPP_MSG_TYPE_DATA_ID_BUTTON:
          *handled = true;
          McuLog_info("Rx: Button is %s", msgValue==0?"off":"on");
          break;
        default:
          break;
      }
      break;
      /* ------------ Received a Notifications -------------------*/
      case RAPP_MSG_TYPE_NOTIFY_VALUE: /* received notification */
        msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
        msgValue = McuUtility_GetValue32LE(&data[2]);
        switch(msgID) {
        #if PL_CONFIG_HAS_BATTERY_ADC
          case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
            *handled = true;
            McuLog_info("Notify: Battery voltage is %d mV", msgValue);
            break;
        #endif
          case RAPP_MSG_TYPE_DATA_ID_LED:
            *handled = true;
            McuLog_info("Notify: LED is %s", msgValue==0?"off":"on");
            break;
          case RAPP_MSG_TYPE_DATA_ID_BUTTON:
            *handled = true;
            McuLog_info("Notify: Button is %s", msgValue==0?"off":"on");
            break;

          case RAPP_MSG_TYPE_DATA_ID_NAV:
            *handled = true;
            //McuLog_info("Notify: Nav is 0x%0x", msgValue);
            #if PL_IS_INTRO_ZUMO_K22
              Santa_RobotOnButtonEvent(msgValue&0xffff, (msgValue>>16)&0xffff);
            #endif
            break;
          default:
            break;
        } /* switch */
        break;

    default:
      break;
  } /* switch */
  return ERR_OK;
}
#endif /* PL_HAS_RADIO */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"santa", (unsigned char*)"Santa status\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"santa", (unsigned char*)"Group of santa commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows santa help or status\r\n", io->stdOut);
#if PL_HAS_RADIO
  McuShell_SendHelpStr((unsigned char*)"  beep <f> <t>", (unsigned char*)"Send a beep with frequency and duration\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get battery", (unsigned char*)"Query battery voltage\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get button", (unsigned char*)"Query button status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get led", (unsigned char*)"Query LED\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set led on|off", (unsigned char*)"Set LED\r\n", io->stdOut);
#endif
  return ERR_OK;
}

uint8_t Santa_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"santa help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"santa status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
#if PL_HAS_RADIO
  } else if (McuUtility_strncmp((char*)cmd, (char*)"santa beep ", sizeof("santa beep ")-1)==0) {
    uint16_t freq, time;
    uint8_t dataBuf[4]; /* 2 byte frequency, 2 byte duration */
    const unsigned char *p;

    p = cmd + sizeof("santa beep ")-1;
    *handled = true;
    if (McuUtility_ScanDecimal16uNumber(&p, &freq)==ERR_OK && McuUtility_ScanDecimal16uNumber(&p, &time)==ERR_OK) {
      McuUtility_SetValue16LE(freq, &dataBuf[0]);
      McuUtility_SetValue16LE(time, &dataBuf[2]);
      return RAPP_SendPayloadDataBlock(dataBuf, sizeof(dataBuf), RAPP_MSG_TYPE_BEEP, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
    } else {
      McuShell_SendStr((unsigned char*)"ERR: wrong format\r\n", io->stdErr);
      return ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, (char*)"santa get battery")==0) {
    *handled = true;
    return RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BATTERY_V, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"santa get button")==0) {
    *handled = true;
    return RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"santa get led")==0) {
    *handled = true;
    return RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"santa set led on")==0) {
    *handled = true;
    return RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 1, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"santa set led off")==0) {
    *handled = true;
    return RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
#endif /* PL_HAS_RADIO */
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

#if 0 /* not used */
static void santaTask(void *pv) {
  for(;;) {
    #if 0 && PL_CONFIG_USE_BUTTONS /* template for how to send a button event */
    if (Santa_ButtonIsPressed()) {
      RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 1, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE); /* pressed */
      while(Santa_ButtonIsPressed()) {
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      RNETA_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE); /* released */
    }
    #endif
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
#endif

void Santa_Init(void) {
#if 0
  if (xTaskCreate(santaTask, "santa", 1000/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#endif
#if PL_IS_INTRO_ZUMO_K22
  timeoutTimer = xTimerCreate(
        "timeout", /* name */
        pdMS_TO_TICKS(SANTA_TIMEOUT_PERIOD_MS), /* period/time */
        pdFALSE, /* auto reload */
        (void*)0, /* timer ID */
        vTimerCallbacktTimeout); /* callback */
  if (timeoutTimer==NULL) {
    McuLog_fatal("failed creating timer");
    for(;;); /* failure! */
  }
#endif
}

#endif