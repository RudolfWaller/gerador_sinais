#ifndef __APLIC_H
  #define __APLIC_H

  #define FALSE 0
  #define TRUE  (!FALSE)

  #ifdef MAIN
    #define var
  #else
    #define var extern
  #endif

  //---------------------------------------------------------------------------------

  #include <stdio.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "driver/gpio.h"
  #include "esp_log.h"
  #include "sdkconfig.h"

  //---------------------------------------------------------------------------------

  #include "drv_hardware.h"
  #include "drv_dds.h"
#endif
