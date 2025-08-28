#include "aplic.h"

//****************************************************************************

#define TEMPO_ENTRE_BYTES   80
#define TEMPO_APOS_COMANDO 100

//****************************************************************************

tModoDisplay eModoDisplay;

static spi_device_handle_t s_lcd_spi = NULL;
static const char *TAG = "drv_DDS";

static uint8_t vui8Display[4][16];
static uint8_t ui8Col, ui8Lin;

//****************************************************************************

static esp_err_t st7920_spi_init(void) {
  spi_bus_config_t bus = {
    .mosi_io_num = PIN_DISPLAY_DATA,
    .miso_io_num = -1,
    .sclk_io_num = PIN_DISPLAY_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4, // 3 bytes por “escrita” + margem
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_DISABLED));

  spi_device_interface_config_t dev = {
    .clock_speed_hz = 20000 * 1000, // 1 MHz (pode subir depois)
    .mode = 0,                         // CPOL=0, CPHA=0
    .spics_io_num = -1,                // CS NÃO usado (preso em VDD)
    .queue_size = 1,
    .flags = SPI_DEVICE_BIT_LSBFIRST,  // NÃO usar → MSB first (então NÃO defina)
  };
  // Remova a flag acima. Mantemos MSB-first por padrão:
  dev.flags = 0;

  return spi_bus_add_device(SPI2_HOST, &dev, &s_lcd_spi);
}

//****************************************************************************

// Envia 8 bits via SPI (MSB-first, mode 0)
static inline void vEnviaByte(uint8_t _ui8byte1, uint8_t _ui8byte2, uint8_t _ui8byte3) 
{
  uint8_t vui8Buf[3] = { _ui8byte1, _ui8byte2, _ui8byte3};
  spi_transaction_t t = {0};

  t.length = 3*8;         // bits
  t.tx_buffer = &vui8Buf;
  esp_err_t err = spi_device_transmit(s_lcd_spi, &t);
  (void)err;
}

//****************************************************************************

static void vEscreveDisplay(uchar _ucByte, bool _bControl)
{
  uint8_t _ui8Header = _bControl ? 0xF8 : 0xFA;
 
  vEnviaByte(_ui8Header, _ucByte & 0xF0, (_ucByte << 4) & 0xF0);// nibble baixo alinhado

  esp_rom_delay_us(TEMPO_ENTRE_BYTES); // ≥ ~72us entre bytes “lógicos”
}

//****************************************************************************

static void vComandoDisplay(uchar _ucCom)
{
  vEscreveDisplay(_ucCom, TRUE);

  if(_ucCom == 0x01 || _ucCom == 0x03)
    vTaskDelay(1+pdMS_TO_TICKS(2)); // Clear
  else
    esp_rom_delay_us(TEMPO_APOS_COMANDO);
}

//****************************************************************************

void __vGotoXY(uint8_t _ui8Col, uint8_t _ui8Lin) {
  switch(eModoDisplay){
    case eGRAPH:
      static const uint8_t base[4] = {0x80, 0x90, 0x88, 0x98};

      // Y: 0..63 (linhas), X: 0..7 (cada X cobre 16 colunas)
      vComandoDisplay(0x80 | (_ui8Lin & 0x3F));   // Set Y address
      vComandoDisplay(0x80 | (_ui8Col & 0x0F));   // Set X address (0..7)
      break;

    case eTEXT:
      if (_ui8Lin >= 4) 
        _ui8Lin = 0;
      if (_ui8Col >= 16) 
        _ui8Col = 0;
        
      ui8Col=_ui8Col;
      ui8Lin=_ui8Lin;

      vComandoDisplay(base[_ui8Lin] + _ui8Col/2);
      if(_ui8Col & 1){
        ui8Col--;
        __vDadoDisplay((uchar)vui8Display[ui8Lin][ui8Col]);
      }
      break;

    default:
      ESP_LOGE(TAG, "Modo Display inválido em vGotoXY()"); 
  }
}

//****************************************************************************

void __vPosCursor(uchar *_pucCol, uchar *_pucLin)
{
  *_pucCol=ui8Col;
  *_pucLin=ui8Lin;
}

//****************************************************************************

void __vDadoDisplay(uchar _ucDado)
{
  uint8_t *_pui8P;
  uint8_t _ui8C, _ui8L;

  if(ui8Col>=16){
    if(++ui8Lin>=4){
      for(_ui8L=0; _ui8L < 3; _ui8L++){
        _pui8P=&vui8Display[_ui8L+1][0];
        __vGotoXY(0, _ui8L);
        for(_ui8C=16; _ui8C !=0; _ui8C--)
          __vDadoDisplay(*_pui8P++);
      }
      __vGotoXY(0, 3);
      for(_ui8C=16; _ui8C !=0; _ui8C--)
        __vDadoDisplay(' ');
      ui8Lin=3;
    }
    __vGotoXY(0, ui8Lin);
  }

  vui8Display[ui8Lin][ui8Col]=_ucDado;

  vEscreveDisplay(_ucDado, FALSE);
  esp_rom_delay_us(TEMPO_ENTRE_BYTES); // manter ~72 us entre bytes
  ui8Col++;
}

//****************************************************************************

void __vStringDisplay(char *_pcString)
{
  while(*_pcString)
    __vDadoDisplay(*_pcString++);
}

//****************************************************************************

void __vInicioDisplay(tModoDisplay _eModoDisplay)
{
  switch(_eModoDisplay){
    case eINIT:
    case eGRAPH:
    case eTEXT:
      if(eModoDisplay!=_eModoDisplay){
        eModoDisplay=_eModoDisplay;

        vComandoDisplay(0x30); // Function Set (basic)
        vComandoDisplay(0x30); // (mandar 2x ajuda pós-reset)
        vComandoDisplay(0x0C); // Display ON, cursor OFF
        vComandoDisplay(0x06); // Entry mode (incremento)
        vComandoDisplay(0x01); 
        __vGotoXY(0,0);

        if(eModoDisplay==eGRAPH){
          vComandoDisplay(0x30);  //esp_rom_delay_us(TEMPO_APOS_COMANDO); // basic
          vComandoDisplay(0x34);  //esp_rom_delay_us(TEMPO_APOS_COMANDO); // extended
          vComandoDisplay(0x36);  //esp_rom_delay_us(TEMPO_APOS_COMANDO); // graphic ON
        }
      }
      break;

    default:
      ESP_LOGE(TAG, "Modo Display inválido em vInicioDisplay()"); 
  }
}

//****************************************************************************

void vInicioDriverDisplay(void)
{
  ESP_LOGI(TAG, "Iniciando Display"); 

  eModoDisplay=eINIT;

  gpio_set_level(PIN_DISPLAY_DATA, 0);
  for (int i = 0; i < 20; i++) 
    vPulso(PIN_DISPLAY_CLK);

  ESP_ERROR_CHECK(st7920_spi_init());    

  __vInicioDisplay(eTEXT);

  ESP_LOGI(TAG, "Display iniciado"); 
}
