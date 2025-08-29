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
  #include <string.h>
  #include <unistd.h>
  #include <stdarg.h> 
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/semphr.h" 
  #include "driver/gpio.h"
  #include "esp_log.h"
  #include "sdkconfig.h"
  #include "esp_rom_sys.h"
  #include "driver/spi_master.h"

  //---------------------------------------------------------------------------------

  typedef uint8_t uchar;
  //typedef uint16_t uint;
  typedef uint32_t ulong;

  //---------------------------------------------------------------------------------

  #include "drv_hardware.h"
  #include "drv_display.h"
  #include "lib_display.h"
  #include "drv_dds.h"

  #include "app_hardware.h"
  #include "app_display.h"
  #include "app_teclado.h"

  //---------------------------------------------------------------------------------

  var int xxx;
  
  //---------------------------------------------------------------------------------

  void DoNothing(void);

#endif
