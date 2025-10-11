#include "SensorReadTask.h"

#if PL_CONFIG_USE_MULTI_TOF
  #include "McuVL53L5CX.h"
#endif
#if PL_CONFIG_USE_IR_SENS
  #include "McuSTHS34pf80.h"
#endif

void ReadSensors(void) {
  #if PL_CONFIG_USE_MULTI_TOF
    bool TofIsReady;
    VL53L5CX_ResultsData Results;
    TofIsReady = McuVL53L5CX_Programm() == ERR_OK;
    if (TofIsReady) {
      McuVL53L5CX_StartRanging();
    }
    else {      
      vTaskSuspend(NULL); /* suspend task if sensor not ready */
    }
  #endif

  for(;;) {
    #if PL_CONFIG_USE_MULTI_TOF
    if (McuVL53L5CX_IsDataReady())
    {
      McuVL53L5CX_GetRangingData(&Results);
      McuLog_info("Distance: %d mm", Results.distance_mm[0]);
    }
    #endif
    
    #if PL_CONFIG_USE_IR_SENS
     if(McuSTHS34pf80_IsDataReady())
     {
      bool presenceFlag;
      int16_t presenceVal;
      McuSTHS34pf80_GetPresence(&presenceFlag, &presenceVal);
      if (presenceFlag) {
        McuLog_info("Presence detected: %d", presenceVal);
      } else {
        McuLog_info("No presence detected");
      }
     }
    #endif
      
    vTaskDelay(pdMS_TO_TICKS(10*100));
  }
}