/*
 * Copyright (c) 2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SANTA_H_
#define _SANTA_H_

#include "platform.h"

#if 0
  #include "McuLib.h"
  #include "McuDebounce.h"

  #if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

  uint8_t Santa_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
  #endif

  #include "RNet_App.h"

  uint8_t Santa_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet);

  #if McuLib_CONFIG_CPU_IS_ESP32
    void Santa_ESP32OnButtonEvent(uint32_t buttonBits, McuDbnc_EventKinds kind);
  #endif

  typedef enum Santa_RobotMoveStatus_e {
    Santa_MOVE_STATUS_UNKNOWN=0,
    Santa_MOVE_STATUS_STOPPED=1,
    Santa_MOVE_STATUS_MOVING=2,
  } Santa_RobotMoveStatus_e;

  Santa_RobotMoveStatus_e Santa_GetRobotMoveStatus(void);
  void Santa_SetRobotMoveStatus(Santa_RobotMoveStatus_e status);

  void Santa_Init(void);

#endif /* PL_CONFIG_USE_SANTA */

#endif /* _SANTA_H_ */