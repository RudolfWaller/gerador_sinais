#define MAIN
#include <aplic.h>

static const char *TAG = "main";

void DoNothing(void)
{
  ESP_LOGE(TAG, "DoNothing()");
}

void app_main(void)
{
  ESP_LOGI(TAG, "\n\n\n\n\n\nIniciando sistema");
  vInicioDriverHardware();
  vInicioDriverDisplay();
  // vInicioBibliotecaDisplay();
  vInicioDriverDds();
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
    gpio_set_level(PIN_LED, TRUE);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    gpio_set_level(PIN_LED, FALSE);
    vTaskDelay(900 / portTICK_PERIOD_MS);
  }
}

