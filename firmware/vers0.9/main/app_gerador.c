#include "aplic.h"

//****************************************************************************

static const char *TAG = "app_Ger";
static const struct{
  char    vcNome[3+1];
  uint32_t ui32Mult;
} vsPot[3]={{"Hz ", 1},
            {"kHz", 1000},
            {"MHz", 1000000}};
static char     vcFreqAtual[10];
static char     vcFreq[10+1];
static uint8_t  ui8Pot, ui8PotAtual;

//****************************************************************************

static void vMostraFreqAtual(void) 
{
  vLimpaDisplay();
  vStringDisplay("Senoide:");
  vPosPrintf(0, 1, "%s %s", vcFreqAtual, vsPot[ui8PotAtual].vcNome);
}

//****************************************************************************

static void vMostraFreq(void) 
{
  vPosPrintf( 0, 3, "%s %s    ",vcFreq, vsPot[ui8Pot].vcNome);
}

//****************************************************************************

static void vTaskGerador(void *pvParameters) 
{
  char _vcTecla[2];
  char *_pcStr;

  uint8_t _ui8Tam;
  uint32_t _ui32Freq;
  float _fFreq;

  strcpy(vcFreq, "0");
  strcpy(vcFreqAtual, "0");
  _vcTecla[1]=0;

  ui8Pot=0;

  vMostraFreqAtual();
  vMostraFreq();
  
  while(1){
    if (xQueueReceive(sTeclas, &_vcTecla[0], portMAX_DELAY)){
      switch(_vcTecla[0]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          if(strcmp(vcFreq, "0")==0)
            vcFreq[0]=_vcTecla[0];
          else
            if(strlen(vcFreq)<sizeof(vcFreq)-1)
              strcat(vcFreq, _vcTecla);
          vMostraFreq();
          break;

        case '*':
          if(strcmp(vcFreq, "0")!=0){
            _vcTecla[0]=',';
            if(strlen(vcFreq)<sizeof(vcFreq)-2)
              strcat(vcFreq, _vcTecla);
            vMostraFreq();
          }
          break;

        case '#':
          _ui8Tam=strlen(vcFreq);
          if(_ui8Tam==1)
            vcFreq[0]='0';
          else
            vcFreq[_ui8Tam-1]=0;

          vMostraFreq();
          break;

        case 'B':
          if(++ui8Pot>=3)
            ui8Pot=0;
          vMostraFreq();
          break;

        case 'C':
          strcpy(vcFreq, "0");
          vMostraFreq();
          break;

        case 'D':
          strcpy(vcFreqAtual, vcFreq);
          ui8PotAtual=ui8Pot;

          for(_pcStr=vcFreq; *_pcStr!=0; _pcStr++)
            if(*_pcStr==',')
              *_pcStr='.';
          _fFreq=atof(vcFreq)*vsPot[ui8PotAtual].ui32Mult;
          snprintf(vcFreq, sizeof(vcFreq), "%.2f", _fFreq);
          for(_pcStr=vcFreq; *_pcStr!=0; _pcStr++)
            if(*_pcStr=='.'){
              *_pcStr=0;
              break;
            }
          _ui32Freq=atol(vcFreq);
          vEnviaFreq(_ui32Freq, 0);

          strcpy(vcFreq, "0");
          ui8Pot=0;

          vMostraFreqAtual();
          vMostraFreq();
          break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

//****************************************************************************

void vInicioAppGerador(void)
{
  ESP_LOGI(TAG, "Iniciando teclado APP");

  xTaskCreate(vTaskGerador, "Task_Gerador", 4096, NULL, 1, NULL);
  
  ESP_LOGI(TAG, "teclado APP iniciado");
}
