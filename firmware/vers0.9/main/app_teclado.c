#include "aplic.h"

//****************************************************************************

#define COL0 0x08
#define COL1 0x40
#define COL2 0x10
#define COL3 0x20
#define COLUNAS (COL0|COL1|COL2|COL3)
#define LIN0 0x01
#define LIN1 0x02
#define LIN2 0x04
#define LIN3 0x08
#define LINHAS (LIN0|LIN1|LIN2|LIN3)
#define NO_TEC 0x00

//****************************************************************************

static const char *TAG = "app_Tec";
static enum{
  eINI_VAR,
  eESP_TECLAR,
  ePREP_LE_TECLAS,
  eLE_TECLA,
  eESP_LIB_TECLA,
}eTecSel;

static const uint8_t vui8Linhas[] ={LIN0, LIN1, LIN2, LIN3};
//static const uint8_t vui8Colunas[]={COL0, COL1, COL2, COL3};
static uint8_t ui8Latch;

static const char *vpcTeclas[4]={"369#",
                                 "147*",
                                 "2580",
                                 "ABCD"};

//****************************************************************************

static inline void vEnviaLinhas(uint8_t _ui8Valor)
{
  ui8Latch=(ui8Latch & ~(LINHAS)) | (_ui8Valor & LINHAS);
  ui8Teclado(ui8Latch);
}

//****************************************************************************

static inline uint8_t ui8LeColunas(void)
{
  return(ui8Teclado(ui8Latch) & COLUNAS);
}

//****************************************************************************

static int8_t i8CalcColuna(uint8_t _ui8ColunaLida)
{
  switch(_ui8ColunaLida){
    case COL0:
      return(0);

    case COL1:
      return(1);
      
    case COL2:
      return(2);
      
    case COL3:
      return(3);

    default:
      return(-1);
  }
}

//****************************************************************************

static void vTaskTeclado(void *pvParameters) 
{
  uint8_t _ui8TecAux;
  uint8_t _ui8QtdeLida, _ui8Tecla, _ui8NumLin;
  int8_t _i8NumCol;

  eTecSel =eINI_VAR;
  ui8Latch=0x00;

  sTeclas = xQueueCreate(5, sizeof(uint8_t));  

  while (1) {
    switch(eTecSel){
      case eINI_VAR:
        vEnviaLinhas(LIN0|LIN1|LIN2|LIN3);
        eTecSel=eESP_TECLAR;
        break;

      case eESP_TECLAR:
        _ui8TecAux=ui8LeColunas();
        if(_ui8TecAux!=NO_TEC)
          eTecSel=ePREP_LE_TECLAS;
        break;
        
      case ePREP_LE_TECLAS:
        _ui8QtdeLida=0;
        _ui8Tecla   =0;
        eTecSel     =eLE_TECLA;
        break;
        // Não tem break aqui
        
      case eLE_TECLA:
        for(_ui8NumLin=0; _ui8NumLin<4; _ui8NumLin++){
          vEnviaLinhas(vui8Linhas[_ui8NumLin]);
          esp_rom_delay_us(2000);
          _ui8TecAux=ui8LeColunas();
          _i8NumCol=i8CalcColuna(_ui8TecAux);
          if(_i8NumCol>=0){
            if(_ui8Tecla==0)
              _ui8Tecla=vpcTeclas[_ui8NumLin][_i8NumCol];
            else
              if(_ui8Tecla==vpcTeclas[_ui8NumLin][_i8NumCol]){
                if(++_ui8QtdeLida>=1){
                  do{
                  }while(xQueueSend(sTeclas, &_ui8Tecla, pdMS_TO_TICKS(100)) != pdPASS);

                  eTecSel=eESP_LIB_TECLA;
                }
              }
              else
                eTecSel=eESP_LIB_TECLA;
          }
        }
        if(_ui8Tecla==0)
          eTecSel =eINI_VAR;
        break;
        
      case eESP_LIB_TECLA:
        if((ui8Latch & LINHAS)!=(LIN0|LIN1|LIN2|LIN3)){
          vEnviaLinhas(LIN0|LIN1|LIN2|LIN3);
        }
        else
          if(ui8LeColunas()==0x0)
            eTecSel =eINI_VAR;
        break;
    }  

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

//****************************************************************************

void vInicioAppTeclado(void)
{
  ESP_LOGI(TAG, "Iniciando teclado APP");

  xTaskCreate(vTaskTeclado, "Task_Teclado", 2048, NULL, 1, NULL);
  
  ESP_LOGI(TAG, "teclado APP iniciado");
}
