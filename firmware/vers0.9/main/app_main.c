#define MAIN
#include <aplic.h>

static const char *TAG = "main";

void app_main(void)
{
  ESP_LOGI(TAG, "\n\n\n\n\n\nIniciando sistema");
  vInitHw();
  vInitDds();
  ESP_LOGI(TAG, "Sistema iniciado");
  /*
  vEnviaFreq(1, 0);
  vEnviaFreq(10, 0);
  vEnviaFreq(100, 0);
  vEnviaFreq(1000, 0);
  vEnviaFreq(10000, 0);
  vEnviaFreq(100000, 0);
  vEnviaFreq(1000000, 0);
  vEnviaFreq(10000000, 0);
  */
  //---------------------------------------------------------
  
  while (1) {
    // ESP_LOGI(TAG, "Led %d", bLed);
    // gpio_set_level(PIN_LED, bLed);
    // bLed=!bLed;
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

