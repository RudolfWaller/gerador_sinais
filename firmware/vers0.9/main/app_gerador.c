#include "aplic.h"

//****************************************************************************

static const char *TAG = "app_Ger";

//****************************************************************************

static void vTaskGerador(void *pvParameters) 
{
  uint8_t _ui8Tecla;

  vStringDisplay("Ok ");
  
  while(1){
    if (xQueueReceive(sTeclas, &_ui8Tecla, portMAX_DELAY)) 
      vDadoDisplay(_ui8Tecla);

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

//****************************************************************************

void vInicioAppGerador(void)
{
  ESP_LOGI(TAG, "Iniciando teclado APP");

  xTaskCreate(vTaskGerador, "Task_Gerador", 2048, NULL, 1, NULL);
  
  ESP_LOGI(TAG, "teclado APP iniciado");
}
