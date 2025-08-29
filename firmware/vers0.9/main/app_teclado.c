#include "aplic.h"

//****************************************************************************

static const char *TAG = "app_Tec";

//****************************************************************************

static void vTaskTeclado(void *pvParameters) 
{
  uint8_t _ui8Linha, _ui8Coluna, _ui8Bit;
  int8_t _i8Tecla;
  uint8_t _ui8UltLeit[4], _ui8LeitAtual, _ui8LeitTeclado;
  const uint8_t _vui8Colunas[]={0x08, 0x40, 0x10, 0x20};
  const uint8_t _vui8Teclas[4][4]={{3,6,9,0xf},
                                   {1,4,7,0xe},
                                   {2,5,8,0},
                                   {10,11,12,13}};

  for(_ui8Linha=0; _ui8Linha<4; _ui8Linha++){
    _ui8UltLeit[_ui8Linha]=0xFF;
  }

  while (1) {
    _i8Tecla=-1;

    for(_ui8Linha=0; _ui8Linha<4; _ui8Linha++){
      _ui8Bit=1<<_ui8Linha;
      ui8Teclado(_ui8Bit);
      esp_rom_delay_us(1000);
      _ui8LeitAtual=ui8Teclado(_ui8Bit);
      _ui8LeitTeclado=_ui8LeitAtual & 0x7E;

      for(_ui8Coluna=0; _ui8Coluna<4; _ui8Coluna++)
        if(_ui8LeitTeclado==_vui8Colunas[_ui8Coluna])
          break;

      if(_ui8LeitTeclado!=_ui8UltLeit[_ui8Linha]){
        if(_ui8UltLeit[_ui8Linha]!=0xFF && _ui8Coluna!=4){
          _i8Tecla=_vui8Teclas[_ui8Linha][_ui8Coluna];
          /*
          vGotoXY(0,0);
          vPrintf("%2X-%2X",_ui8Linha,_ui8Coluna);
          vGotoXY(0,1);
          vPrintf("%2d",_i8Tecla);
          */
        }
        _ui8UltLeit[_ui8Linha]=_ui8LeitTeclado;
      }

      if(_i8Tecla!=-1)
        vPrintf("%d",_i8Tecla);

      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
}

//****************************************************************************

void vInicioAppTeclado(void)
{
  ESP_LOGI(TAG, "Iniciando teclado APP");

  xTaskCreate(vTaskTeclado, "Task_Teclado", 2048, NULL, 1, NULL);
  
  ESP_LOGI(TAG, "teclado APP iniciado");
}
