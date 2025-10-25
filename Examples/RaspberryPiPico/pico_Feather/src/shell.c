/*
 * Copyright (c) 2019-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHELL
#include "shell.h"
#include "McuShell.h"
#include "McuRTOS.h"
#include "McuRTT.h"
#include "McuArmTools.h"
#include "McuLog.h"
#include "McuShellUart.h"
#include "McuTimeDate.h"
#include "leds.h"
#if PL_CONFIG_USE_MININI
  #include "McuMinINI.h"
  #include "minGlue-Flash.h"
#endif
#if PL_CONFIG_USE_NVMC
  #include "McuFlash.h"
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoPixel.h"
#endif
#if PL_CONFIG_USE_TUD_CDC
  #include "McuShellCdcDevice.h"
#endif
#if PL_HAS_RADIO
  #include "RNet/McuRNet.h"
  #include "RNet/RStdIO.h"
  #include "RNet_App.h"
#endif
#if PL_CONFIG_USE_RTC
  #include "McuPCF85263A.h"
#endif
#if PL_CONFIG_USE_US_SENS
  #include "McuSRT04T.h"
#endif
#if PL_CONFIG_USE_BATTERY
  #include "McuBattery.h"
#endif
#if PL_CONFIG_USE_IR_SENS
  #include "McuSTHS34pf80.h"
#endif
#if PL_CONFIG_USE_CONFIGURATION_MANAGER
  #include "ConfigurationManager.h"
#endif

static const McuShell_ParseCommandCallback CmdParserTable[] =
{
  McuShell_ParseCommand, /* McuShell component, is first in list */
  McuRTOS_ParseCommand, /* FreeRTOS shell parser */
#if PL_CONFIG_USE_LEDS
  Leds_ParseCommand,
#endif
#if McuArmTools_CONFIG_PARSE_COMMAND_ENABLED
  McuArmTools_ParseCommand,
#endif
#if PL_CONFIG_USE_NVMC
  McuFlash_ParseCommand,
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_ParseCommand,
  ini_ParseCommand,
#endif
#if McuLog_CONFIG_IS_ENABLED
  McuLog_ParseCommand,
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEO_ParseCommand,
#endif
#if PL_CONFIG_USE_TUD_CDC
  McuShellCdcDevice_ParseCommand,
#endif
#if PL_HAS_RADIO
  McuRNet_ParseCommand,
  RNETA_ParseCommand,
#endif
#if PL_CONFIG_USE_RTC
  McuPCF85263A_ParseCommand,
#endif
#if PL_CONFIG_USE_US_SENS
  McuSRT04T_ParseCommand,
#endif
#if PL_CONFIG_USE_BATTERY
  McuBattery_ParseCommand,
#endif
#if PL_CONFIG_USE_IR_SENS
  McuSTHS34pf80_ParseCommand,
#endif
#if PL_CONFIG_USE_CONFIGURATION_MANAGER
  CONFIG_ParseCommand,
#endif
  NULL /* Sentinel */
};

typedef struct {
  McuShell_ConstStdIOType *stdio;
  unsigned char *buf;
  size_t bufSize;
} SHELL_IODesc;

#if PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO
/* I/O handler for RNet StdIO output. Used for things like 'rapp send out hello', to have the 'hello' printed on the console.
  Used as I/O handler for RSTDIO_Print() which uses only stdOut and stdErr */
static void rnet_StdIOReadChar(uint8_t *c) {
  *c = '\0'; /* no input needed */
}

bool rnet_StdIOKeyPressed(void) {
  return false; /* not used */
}

void rnet_StdIOSendChar(uint8_t ch) {
#if PL_CONFIG_USE_SHELL_CDC
  McuShellCdcDevice_GetStdio()->stdOut(ch);
#endif
#if PL_CONFIG_USE_RTT
  McuRTT_GetStdio()->stdOut(ch);
#endif
#if !PL_CONFIG_USE_SHELL_CDC && !PL_CONFIG_USE_RTT
  /* fallback if there is no other connection from above */
  McuShell_GetStdio()->stdOut(ch);
#endif
}

/* default standard I/O struct */
static McuShell_ConstStdIOType rnet_stdio = {
    .stdIn = (McuShell_StdIO_In_FctType)rnet_StdIOReadChar,
    .stdOut = (McuShell_StdIO_OutErr_FctType)rnet_StdIOSendChar,
    .stdErr = (McuShell_StdIO_OutErr_FctType)rnet_StdIOSendChar,
    .keyPressed = rnet_StdIOKeyPressed, /* if input is not empty */
  #if McuShell_CONFIG_ECHO_ENABLED
    .echoEnabled = false,
  #endif
  };
#endif /* PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO */

static const SHELL_IODesc ios[] =
{
#if PL_CONFIG_USE_SHELL_UART
  {&McuShellUart_stdio,  McuShellUart_DefaultShellBuffer,  sizeof(McuShellUart_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_USB_CDC
  {&cdc_stdio,  cdc_DefaultShellBuffer,  sizeof(cdc_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_SHELL_CDC
  {&McuShellCdcDevice_stdio,  McuShellCdcDevice_DefaultShellBuffer,  sizeof(McuShellCdcDevice_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_RTT
  {&McuRTT_stdio,  McuRTT_DefaultShellBuffer,  sizeof(McuRTT_DefaultShellBuffer)},
#endif
#if PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO
  {&RSTDIO_stdio, RSTDIO_DefaultShellBuffer, sizeof(RSTDIO_DefaultShellBuffer)},
#endif
};

void SHELL_SendChar(unsigned char ch) {
  for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    McuShell_SendCh(ch, ios[i].stdio->stdOut);
  }
}

uint8_t SHELL_ParseCommand(unsigned char *cmd) {
  return McuShell_ParseWithCommandTable(cmd, McuShell_GetStdio(), CmdParserTable);
}

void SHELL_SendString(const unsigned char *str) {
#if McuLib_CONFIG_CPU_IS_ESP32
  /* need to improve write speed, as writing character by character is too slow */
  Uart_SendString(str);
#else
  for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    McuShell_SendStr(str, ios[i].stdio->stdOut);
  }
#endif
}

void SHELL_SendStringToIO(const unsigned char *str, McuShell_ConstStdIOType *io) {
  McuShell_SendStr(str, io->stdOut);
}

uint8_t SHELL_ParseCommandIO(const unsigned char *command, McuShell_ConstStdIOType *io, bool silent) {
  if (io==NULL) { /* use a default */
#if PL_CONFIG_USE_SHELL_UART
    io = &McuShellUart_stdio;
#elif PL_CONFIG_USE_USB_CDC
    io = &cdc_stdio;
#elif PL_CONFIG_USE_RTT
    io = &McuRTT_stdio;
#else
  #error "no shell std IO?"
#endif
  }
  return McuShell_ParseWithCommandTableExt(command, io, CmdParserTable, silent);
}




static void ShellTask(void *pvParameters) {
  int i;

  (void)pvParameters; /* not used */
  McuLog_trace("started shell task");
  /* initialize buffers */
  for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0'; /* initialize I/O buffers */
  }
  vTaskDelay(pdMS_TO_TICKS(500)); /* give some time to other tasks to start up */
  McuShell_PrintPrompt(McuShell_GetStdio()); /* print prompt */
  for(;;) {
    /* process all I/Os */
    for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
      (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
    }
  #if PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO
    /* dispatch incoming messages */
    RSTDIO_Print(&rnet_stdio);
  #endif
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_SHELL, 1, 5);
  #else
    vTaskDelay(pdMS_TO_TICKS(20));
  #endif
  } /* for */
}

static void ConfigureLogger(void) {
#if McuLog_CONFIG_IS_ENABLED
  #if PL_CONFIG_USE_RTT && PL_CONFIG_USE_SHELL_UART && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(&McuShellUart_stdio, 1);
  #elif PL_CONFIG_USE_RTT && PL_CONFIG_USE_USB_CDC && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(&cdc_stdio, 1);
  #elif PL_CONFIG_USE_RTT && PL_CONFIG_USE_TUD_CDC && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(McuShellCdcDevice_GetStdio(), 1);
  #elif PL_CONFIG_USE_RTT /* only RTT */
    McuLog_set_console(McuRTT_GetStdio(), 0);
  #elif PL_CONFIG_USE_SHELL_UART /* only UART */
    McuLog_set_console(&McuShellUart_stdio, 0);
  #elif PL_CONFIG_USE_TUD_CDC
    McuLog_set_console(McuShellCdcDevice_GetStdio(), 0);
  #endif
#endif
}

void SHELL_Init(void) {
  BaseType_t res;

#if PL_CONFIG_USE_USB_CDC
  McuShell_SetStdio(&cdc_stdio); /* send to USB CDC */
#elif PL_CONFIG_USE_TUD_CDC
  McuShell_SetStdio(McuShellCdcDevice_GetStdio()); /* send to USB CDC */
#elif PL_CONFIG_USE_RTT
  McuShell_SetStdio(McuRTT_GetStdio()); /* use RTT as the default */
#else
  #error "need a standard I/O defined"
#endif
  ConfigureLogger();
  res = xTaskCreate(ShellTask, "ShellTask", 4*1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+4, NULL);
  if (res!=pdPASS) {
    McuLog_fatal("creating ShellTask task failed!");  // GCOVR_EXCL_LINE
    for(;;) {}                                        // GCOVR_EXCL_LINE
  }
}

void SHELL_Deinit(void) {
  McuShell_SetStdio(NULL);
}

#endif /* PL_CONFIG_USE_SHELL */
