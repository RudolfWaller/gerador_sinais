#include <aplic.h>

static const char *TAG = "drv_HW";

void vInicioDriverHardware(void)
{
  ESP_LOGI(TAG, "Iniciando hardware");

  // Hardware
  gpio_reset_pin(PIN_RESET);
  gpio_set_direction(PIN_RESET, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_RESET, TRUE);

  gpio_reset_pin(PIN_LED);
  gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_LED, FALSE);

  // DDS

  gpio_reset_pin(PIN_DDS_ADJ);
  gpio_set_direction(PIN_DDS_ADJ, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DDS_ADJ, FALSE);

  gpio_reset_pin(PIN_DDS_WCLK);
  gpio_set_direction(PIN_DDS_WCLK, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DDS_WCLK, FALSE);

  gpio_reset_pin(PIN_DDS_FQ_UD);
  gpio_set_direction(PIN_DDS_FQ_UD, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DDS_FQ_UD, FALSE);

  gpio_reset_pin(PIN_DDS_DATA);
  gpio_set_direction(PIN_DDS_DATA, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DDS_DATA, FALSE);

  // Display

  gpio_reset_pin(PIN_DISPLAY_CLK);
  gpio_set_direction(PIN_DISPLAY_CLK, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DISPLAY_CLK, FALSE);

  gpio_reset_pin(PIN_DISPLAY_DATA);
  gpio_set_direction(PIN_DISPLAY_DATA, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_DISPLAY_DATA, FALSE);

  // Teclado

  gpio_reset_pin(PIN_SHREG_CLK);
  gpio_set_direction(PIN_SHREG_CLK, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_SHREG_CLK, FALSE);

  gpio_reset_pin(PIN_SHREG_DATAOUT);
  gpio_set_direction(PIN_SHREG_DATAOUT, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_SHREG_DATAOUT, FALSE);

  gpio_reset_pin(PIN_SHREG_DATAIN);
  gpio_set_direction(PIN_SHREG_DATAIN, GPIO_MODE_INPUT);

  gpio_reset_pin(PIN_SHREG_HOLD);
  gpio_set_direction(PIN_SHREG_HOLD, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_SHREG_HOLD, TRUE);

  ui8Teclado(0x00);

  // Reset do sistema

  vReset();

  ESP_LOGI(TAG, "Hardware iniciado");
}

void vReset(void)
{
  gpio_set_level(PIN_RESET, TRUE);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(PIN_RESET, FALSE); 
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void vPulso(uint8_t _ui8Pin)
{
  gpio_set_level(_ui8Pin, TRUE);
  __asm__ __volatile__("nop");
  gpio_set_level(_ui8Pin, FALSE);
  __asm__ __volatile__("nop");
}

uint8_t ui8ShiftReg(gpio_num_t _gDataOutPin, gpio_num_t _gDataInPin, gpio_num_t _gClockPin, bool _bMsbFirst, uint8_t _ui8_Dado)
{
  int _iCont;
  uint8_t _ui8Entrada=0x00;

  for (_iCont = 8; _iCont != 0; _iCont--)
  {
    bool _bOutput;

    if (_bMsbFirst)
    {
      _bOutput = _ui8_Dado & 0b10000000;
      _ui8_Dado = _ui8_Dado << 1;
    }
    else
    {
        _bOutput = _ui8_Dado & 0b00000001;
        _ui8_Dado = _ui8_Dado >> 1;
    }
    gpio_set_level(_gDataOutPin, _bOutput);
    vPulso(_gClockPin);

    _ui8Entrada>>=1;
    if(gpio_get_level(_gDataInPin))
      _ui8Entrada|=0x80;
  }

  return(_ui8Entrada);
}

uint8_t ui8Teclado(uint8_t _ui8Saida)
{
  uint8_t _ui8Entrada;

  gpio_set_level(PIN_SHREG_HOLD, FALSE);
  __asm__ __volatile__("nop");
  gpio_set_level(PIN_SHREG_HOLD, TRUE);
  _ui8Entrada=ui8ShiftReg(PIN_SHREG_DATAOUT, PIN_SHREG_DATAIN, PIN_SHREG_CLK, TRUE, _ui8Saida);
  gpio_set_level(PIN_SHREG_HOLD, FALSE);
  __asm__ __volatile__("nop");
  gpio_set_level(PIN_SHREG_HOLD, TRUE);

  return(_ui8Entrada);
}
