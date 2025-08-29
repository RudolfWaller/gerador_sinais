#include "aplic.h"

//****************************************************************************

#define PRINTF_BUFFER_SIZE 100

//****************************************************************************

typedef struct {
    uint8_t ui8linha, ui8Coluna;
} Display_t;

//****************************************************************************

static const char *TAG = "app_DSP";
SemaphoreHandle_t sSemDisplay;

//****************************************************************************

void vRecuperaInfoDisplay(void)
{
  TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
  Display_t *pxState = (Display_t *)pvTaskGetThreadLocalStoragePointer(xTaskHandle, 0);

  if(pxState==NULL){
    pxState = (Display_t *)malloc(sizeof(Display_t));
    pxState->ui8linha = 0;
    pxState->ui8Coluna = 0;
    vTaskSetThreadLocalStoragePointer(xTaskHandle, 0, pxState);
  }

  if (xSemaphoreTake(sSemDisplay, portMAX_DELAY) == pdTRUE) 
    __vGotoXY(pxState->ui8Coluna, pxState->ui8linha);
}

//****************************************************************************

void vSalvaInfoDisplay(void)
{
  TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
  Display_t *pxState = (Display_t *)pvTaskGetThreadLocalStoragePointer(xTaskHandle, 0);

  __vGetCursor(&pxState->ui8Coluna, &pxState->ui8linha);
  xSemaphoreGive(sSemDisplay);
}

//****************************************************************************

void vDadoDisplay(uchar _ucDado)
{
  vRecuperaInfoDisplay();
  __vDadoDisplay(_ucDado);
  vSalvaInfoDisplay();
}

//****************************************************************************

void vGotoXY(uint8_t _ui8Col, uint8_t _ui8Lin)
{
  vRecuperaInfoDisplay();  
  __vGotoXY(_ui8Col, _ui8Lin);
  //__vSetCursor(_ui8Col, _ui8Lin);
  vSalvaInfoDisplay();
}

//****************************************************************************

void vStringDisplay(char *_pcString)
{
  vRecuperaInfoDisplay();
  __vStringDisplay(_pcString);
  vSalvaInfoDisplay();
}

//****************************************************************************

void vPrintf(const char *fmt, ...) 
{
  char buffer[PRINTF_BUFFER_SIZE];
  va_list args;
  
  // Inicia a lista de argumentos variáveis.
  va_start(args, fmt);
  
  // Formata a string e a armazena no buffer com segurança.
  // vsnprintf previne overflow do buffer.
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  
  // Finaliza a lista de argumentos.
  va_end(args);
  
  // Envia a string formatada para a função do seu driver.
  vStringDisplay(buffer);
}

//****************************************************************************


void vInicioDisplay(tModoDisplay _eModoDisplay)
{
  __vInicioDisplay(_eModoDisplay);
}

//****************************************************************************

static void st7920_fill_pattern(uint8_t lo, uint8_t hi) 
{
  for (uint8_t y = 0; y < 64; y++) {
    vGotoXY(0, y);
    for (uint8_t x = 0; x < 16; x++) { // 16*2 bytes = 256 bits/linha
      if(y & 4){
        vDadoDisplay(hi); // nibble alto (bits 7..0 vão para 8 colunas ímpares)
        vDadoDisplay(hi); // nibble baixo (8 colunas pares)
      }
      else{
        vDadoDisplay(lo); // nibble alto (bits 7..0 vão para 8 colunas ímpares)
        vDadoDisplay(lo); // nibble baixo (8 colunas pares)
      }
    }
  }
}

//****************************************************************************

static void vTeste0(void)
{
  vGotoXY(0, 0);
  vStringDisplay("12345678901234567890");
  vStringDisplay("12345678901234567890");
  vStringDisplay("12345678901234567890");
  vStringDisplay("12345678901234567890");
  vStringDisplay("12345678901234567890");

  while(1)
    vTaskDelay(pdMS_TO_TICKS(2000)); // Clear    
}

//****************************************************************************

static void vTeste1(void)
{
  uint8_t _ui8Lin, _ui8Col;

  while (1) {
    vInicioDisplay(eGRAPH);
    st7920_fill_pattern(0x0F, 0xF0);
    vTaskDelay(pdMS_TO_TICKS(2000)); // Clear    

    vInicioDisplay(eTEXT);
    for(_ui8Lin=0; _ui8Lin<4; _ui8Lin++){
      for(_ui8Col=0; _ui8Col<16; _ui8Col++){
        vGotoXY(_ui8Col / 2,_ui8Lin);
        vDadoDisplay('0' + _ui8Col + _ui8Lin);
        _ui8Col++;
        vDadoDisplay('0' + _ui8Col + _ui8Lin);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(2000)); // Clear    
  }
}

//****************************************************************************

static void vTeste2(void)
{
  uint8_t _ui8Lin, _ui8Col;

  vInicioDisplay(eTEXT);
  for(_ui8Lin=0; _ui8Lin<4; _ui8Lin++){
    for(_ui8Col=0; _ui8Col<16; _ui8Col++){
      vGotoXY(_ui8Col,_ui8Lin);
      vDadoDisplay('0' + _ui8Col + _ui8Lin);
    }
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Clear    
  }
}

//****************************************************************************

static void vTeste3(void)
{
  uint8_t _ui8Cont;

  _ui8Cont='A';
  while(1){
    vDadoDisplay(_ui8Cont++);
    if(_ui8Cont>'Z')
      _ui8Cont='A';

    vTaskDelay(pdMS_TO_TICKS(20)); // Clear    
  }
}

//****************************************************************************

static void vTeste4(void)
{
  uint8_t _ui8L, _ui8C;
  char _cLetra;

  vInicioDisplay(eTEXT);
  _cLetra='A';
  _ui8L=0;
  _ui8C=0;
  while(1){
    vGotoXY(_ui8C,_ui8L);
    vDadoDisplay(_cLetra);
    if(++_cLetra>'Z')
      _cLetra='A';
    if(++_ui8C>=16){
      _ui8C=0;
      if(++_ui8L>=4)
        _ui8L=3;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

//****************************************************************************

static void vTaskDisplay(void *pvParameters) 
{  
  vTeste3();
}

//****************************************************************************

static void vTaskTesteDisplay(void *pvParameters) 
{ 
  uint8_t _ui8Cont, _ui8Indice;

  _ui8Indice=*(uint8_t *) pvParameters;
  _ui8Cont=0;
  while(1){
    vGotoXY(3*(_ui8Indice%5), _ui8Indice/5);
    vPrintf("%3d",_ui8Cont);
    //vPrintf("%3d",_ui8Indice);
    if(++_ui8Cont>=100)
      _ui8Cont=0;

    vTaskDelay(pdMS_TO_TICKS(500+11*_ui8Indice));
  }
}

//****************************************************************************

void vCriaTarefas(uint8_t _ui8Indice)
{
  //static char *_vcNome[]={"Task_Display0","Task_Display1","Task_Display2","Task_Display3"};
  static uint8_t _ui8Pos[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

  xTaskCreate(vTaskTesteDisplay, "Task_Display", 2048, &_ui8Pos[_ui8Indice], 1, NULL);
}

//****************************************************************************

void vInicioAppDisplay(void)
{
  ESP_LOGI(TAG, "Iniciando display APP");

  sSemDisplay = xSemaphoreCreateMutex();
    
  if (sSemDisplay == NULL) {  
    __vInicioDisplay(eTEXT);
    __vStringDisplay("Erro na criação do Mutex de índice de tarefas");
    ESP_LOGE(TAG, "Erro na criação do Mutex de índice de tarefas");
    while(1)
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
  
  /*
  uint8_t _ui8Cont;

  for(_ui8Cont=0; _ui8Cont<20; _ui8Cont++)
    vCriaTarefas(_ui8Cont);
  */

  ESP_LOGI(TAG, "display APP iniciado");
}
