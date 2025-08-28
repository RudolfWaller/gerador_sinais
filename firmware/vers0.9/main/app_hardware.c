#include "aplic.h"

//****************************************************************************

static const char *TAG = "app_HW";

//****************************************************************************

static void vTaskLed(void *pvParameters) {
  while (1) {
    gpio_set_level(PIN_LED, TRUE);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_LED, FALSE);
    vTaskDelay(900 / portTICK_PERIOD_MS);
  }
}

//****************************************************************************

void vInicioAppHardware(void)
{
  ESP_LOGI(TAG, "Iniciando hardware APP");

  xTaskCreate(vTaskLed, "Task_Led", 2048, NULL, 1, NULL);
  
  ESP_LOGI(TAG, "hardware APP iniciado");
}
