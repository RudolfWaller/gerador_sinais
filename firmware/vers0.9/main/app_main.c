#define MAIN
#include <aplic.h>

//****************************************************************************

static const char *TAG = "main";

//****************************************************************************

void DoNothing(void)
{
  ESP_LOGE(TAG, "DoNothing()");
}

//****************************************************************************

void app_main(void)
{
  ESP_LOGI(TAG, "\n\n\n\n\n\nIniciando sistema");
  vInicioDriverHardware();
  vInicioDriverDisplay();
  vInicioDriverDds();

  // vInicioBibliotecaDisplay();
  
  vInicioAppHardware();
  vInicioAppDisplay();
  vInicioAppTeclado();
  
  ESP_LOGI(TAG, "\n\nSistema iniciado\n");

  //---------------------------------------------------------

  while(1)
    vTaskDelay(pdMS_TO_TICKS(2000)); 
}

