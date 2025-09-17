#include "aplic.h"

//****************************************************************************

#define TEMPO_ENTRE_BYTES   80
#define TEMPO_APOS_COMANDO 100
#define PRINTF_BUFFER_SIZE 100

//****************************************************************************

static spi_device_handle_t s_lcd_spi = NULL;
static const char *TAG = "drv_DISP";

static uint8_t vui8Display[4][16];
static uint8_t ui8Col, ui8Lin;
//static u8g2_t u8g2;
//static spi_device_handle_t s_lcd_spi;

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

void __vEscreveDisplay(uchar _ucByte, bool _bControle)
{
  uint8_t _ui8Header = _bControle ? 0xF8 : 0xFA;
 
  vEnviaByte(_ui8Header, _ucByte & 0xF0, (_ucByte << 4) & 0xF0);// nibble baixo alinhado

  esp_rom_delay_us(TEMPO_ENTRE_BYTES); // ≥ ~72us entre bytes “lógicos”
}

//****************************************************************************

void __vComandoDisplay(uchar _ucCom)
{
  __vEscreveDisplay(_ucCom, TRUE);

  if(_ucCom == 0x01 || _ucCom == 0x03)
    esp_rom_delay_us(10000);
  else
    esp_rom_delay_us(TEMPO_APOS_COMANDO);
}

//****************************************************************************

void __vGotoXY(uint8_t _ui8Col, uint8_t _ui8Lin) 
{
  static const uint8_t _vui8Base[4] = {0x80, 0x90, 0x88, 0x98};

  switch(eModoDisplay){
    case eGRAPH:
      vPosCursor(_ui8Col, _ui8Lin, TRUE);
      break;

    case eTEXT:
      if(ui8Col!=_ui8Col || ui8Lin!=_ui8Lin){
        if (_ui8Lin >= 4) 
          _ui8Lin = 0;
        if (_ui8Col >= 16) 
          _ui8Col = 0;
          
        ui8Col=_ui8Col;
        ui8Lin=_ui8Lin;

        __vComandoDisplay(_vui8Base[_ui8Lin] + _ui8Col/2);
        if(_ui8Col & 1){
          ui8Col--;
          __vDadoDisplay((uchar)vui8Display[ui8Lin][ui8Col]);
        }
      }
      break;

    default:
      ESP_LOGE(TAG, "Modo Display inválido em vGotoXY()"); 
  }
}

//****************************************************************************

void __vGetCursor(uint8_t *_pui8Col, uint8_t *_pui8Lin)
{
  *_pui8Col=ui8Col;
  *_pui8Lin=ui8Lin;
}

//****************************************************************************

void __vSetCursor(uint8_t _ui8Col, uint8_t _ui8Lin)
{
  ui8Col=_ui8Col;
  ui8Lin=_ui8Lin;
}

//****************************************************************************

void __vLimpaDisplay(void)
{
  switch(eModoDisplay)
  {
    case eGRAPH:
      vDspLimpaDisplay();
      vDspAtualizaDisplay();
      break;

    case eTEXT:
      __vComandoDisplay(0x01); // Limpa display
      memset(vui8Display, ' ', sizeof(vui8Display));
      __vGotoXY(0,0);
      break;

    default:
      break;
  }
}

//****************************************************************************

void __vDadoDisplay(uchar _ucDado)
{
  uint8_t *_pui8P;
  uint8_t _ui8C, _ui8L;

  switch(eModoDisplay){
    case eTEXT:
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

      __vEscreveDisplay(_ucDado, FALSE);
      esp_rom_delay_us(TEMPO_ENTRE_BYTES); // manter ~72 us entre bytes
      ui8Col++;
      break;

    case eGRAPH:
      vDspAscii(_ucDado);
      break;

    default:
      break;
  }  
}

//****************************************************************************

void __vStringDisplay(char *_pcString)
{
  while(*_pcString)
    __vDadoDisplay(*_pcString++);
}

//****************************************************************************

void __vPrintf(const char *fmt, ...) 
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
  __vStringDisplay(buffer);
}


//****************************************************************************

void __vInicioDisplay(tModoDisplay _eModoDisplay)
{
  if(eModoDisplay==eINIT)
  {
    esp_rom_delay_us(50000);  // 50ms
    
    __vComandoDisplay(0x30); // Function Set - Interface básica (DL=1: 8-bit, RE=0: basic instruction)
    __vComandoDisplay(0x30); // Function Set - Repetir para garantia pós-reset
    __vComandoDisplay(0x0C); // Display ON/OFF Control (D=1: Display ON, C=0: Cursor OFF, B=0: Blink OFF)
    __vComandoDisplay(0x01); // Display Clear - Limpa DDRAM e reseta AC para 00H
    __vComandoDisplay(0x06); // Entry Mode Set (I/D=1: incremento, S=0: sem shift)
  }

  if(eModoDisplay!=_eModoDisplay){
    __vLimpaDisplay();
    eModoDisplay=_eModoDisplay;

    switch(_eModoDisplay)
    {
      case eGRAPH:
        __vComandoDisplay(0x34);  // DL=1, RE=1
        __vComandoDisplay(0x36);  // DL=1, RE=1, G=1 (Extended Function Set - Habilita gráfico (G=1))
        
        esp_rom_delay_us(1000);   // Delay final para estabilização

        __vLimpaDisplay();
        vDspSelecionaMascara(MASCARA_GERAL);
        vDspFonte(FONTE_5x7,FALSE);
        break;

      case eINIT:
      case eTEXT:
        __vComandoDisplay(0x34);  // DL=1, RE=1
        __vComandoDisplay(0x34);  // DL=1, RE=1, G=0  (Extended Function Set - Gráfico OFF)
        __vComandoDisplay(0x30);  // DL=1, RE=0 (Function Set - Volta para basic instruction)
        __vLimpaDisplay();
        break;

      default:
        ESP_LOGE(TAG, "Modo Display inválido em vInicioDisplay()"); 
    }
  }
}

//****************************************************************************
/*
// Callback para SPI - versão simplificada
uint8_t u8x8_esp32_spi_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_BYTE_SEND: {
            spi_transaction_t trans = {0};
            trans.length = 8 * arg_int;
            trans.tx_buffer = arg_ptr;
            spi_device_transmit(s_lcd_spi, &trans);
            break;
        }
        case U8X8_MSG_BYTE_INIT:
            // Inicialização já feita em init_spi_and_display()
            break;
        case U8X8_MSG_BYTE_SET_DC:
            // ST7920 não usa DC
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            break;
        default:
            return 0;
    }
    return 1;
}
*/
//****************************************************************************
/*
// Callback para GPIO e delay
uint8_t u8x8_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // Configurar reset pin se necessário
            break;
            
        case U8X8_MSG_DELAY_NANO:
            // Delay muito pequeno - pode ser ignorado no ESP32
            break;
            
        case U8X8_MSG_DELAY_10MICRO:
            esp_rom_delay_us(10);
            break;
            
        case U8X8_MSG_DELAY_100NANO:
            // Delay muito pequeno - pode ser ignorado
            break;
            
        case U8X8_MSG_DELAY_MILLI:
            if(arg_int>=1000/configTICK_RATE_HZ)
              vTaskDelay(1+pdMS_TO_TICKS(arg_int));
            else
              esp_rom_delay_us(1000*arg_int);
            break;
            
        case U8X8_MSG_DELAY_I2C:
            esp_rom_delay_us(5);
            break;
            
        case U8X8_MSG_GPIO_D0:  // SCL
            break;
            
        case U8X8_MSG_GPIO_D1:  // SDA
            break;
            
        case U8X8_MSG_GPIO_CS:        
            //gpio_set_level(PIN_CS, arg_int);
            break;
            
        case U8X8_MSG_GPIO_DC:
            // ST7920 não usa DC
            break;
            
        case U8X8_MSG_GPIO_RESET:
            break;
            
        default:
            return 0;
    }
    return 1;
}
*/
//****************************************************************************
/*
static esp_err_t st7920_spi_init(void) {
  spi_bus_config_t bus = {
    .mosi_io_num = PIN_DISPLAY_DATA,
    .miso_io_num = -1,
    .sclk_io_num = PIN_DISPLAY_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
        .max_transfer_sz = 1024, // Aumentar para modo buffer
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_DISABLED));

  spi_device_interface_config_t dev = {
    .clock_speed_hz = 1000 * 1000,     // 1 MHz
    .mode = 0,                         // CPOL=0, CPHA=0
    .spics_io_num = -1,                // CS NÃO usado (preso em VDD)
    .queue_size = 1,
    .flags = 0,                        // NÃO usar → MSB first (então NÃO defina)
  };

  return spi_bus_add_device(SPI2_HOST, &dev, &s_lcd_spi);
}
*/
//****************************************************************************
/*
void init_spi_and_display(void) 
{
  ESP_ERROR_CHECK(st7920_spi_init());

  // Setup U8g2 para ST7920
  u8g2_Setup_st7920_128x64_f(&u8g2, U8G2_R0, u8x8_esp32_spi_byte_cb, u8x8_esp32_gpio_and_delay_cb);
  
  // Inicializar display
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  //u8g2_ClearDisplay(&u8g2);
  // Teste inicial simples
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
  u8g2_DrawStr(&u8g2, 0, 10, "INIT OK");
  u8g2_SendBuffer(&u8g2);
  
  while(1)
    vTaskDelay(pdMS_TO_TICKS(2000)); // Mostrar por 2 segundos
}
*/
//****************************************************************************
/*
void demo_fontes_diversas(void) {
    static int contador = 0;
    char buffer[50];
    
    u8g2_ClearBuffer(&u8g2);
    
    // Título pequeno no topo
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);  // Fonte muito pequena (4x6)
    u8g2_DrawStr(&u8g2, 0, 6, "ESP32 + ST7920 Demo");
    
    // Linha horizontal
    u8g2_DrawHLine(&u8g2, 0, 8, 128);
    
    // Fonte pequena (6x10)
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    u8g2_DrawStr(&u8g2, 0, 20, "Fonte 6x10:");
    sprintf(buffer, "Count: %d", contador);
    u8g2_DrawStr(&u8g2, 65, 20, buffer);
    
    // Fonte média (7x14) 
    u8g2_SetFont(&u8g2, u8g2_font_7x14_tr);
    u8g2_DrawStr(&u8g2, 0, 35, "Font 7x14");
    
    // Fonte grande (10x20)
    u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
    u8g2_DrawStr(&u8g2, 0, 55, "Big Text");
    
    // Fonte numérica grande para contador
    u8g2_SetFont(&u8g2, u8g2_font_logisoso16_tn);  // Números grandes
    sprintf(buffer, "%02d", contador % 100);
    u8g2_DrawStr(&u8g2, 90, 55, buffer);
        
    u8g2_SendBuffer(&u8g2);
    
    contador++;
}
*/
//****************************************************************************
/*
void demo_texto_rolante(void) {
    static int scroll_pos = 128;
    const char* texto = "Texto rolante da direita para esquerda!";
    
    u8g2_FirstPage(&u8g2);
    do {
        u8g2_SetFont(&u8g2, u8g2_font_8x13_tr);
        
        // Título fixo
        u8g2_DrawStr(&u8g2, 20, 15, "SCROLL DEMO");
        u8g2_DrawHLine(&u8g2, 0, 18, 128);
        
        // Texto rolante
        u8g2_DrawStr(&u8g2, scroll_pos, 35, texto);
        
        // Barra de progresso animada
        int largura_barra = (scroll_pos + 200) % 128;
        u8g2_DrawBox(&u8g2, 10, 50, largura_barra, 8);
        u8g2_DrawFrame(&u8g2, 9, 49, 110, 10);
        
    } while (u8g2_NextPage(&u8g2));
    
    scroll_pos -= 2;
    if (scroll_pos < -250) scroll_pos = 128;
}
*/
//****************************************************************************
/*
void demo_informacoes_sistema(void) {
    char buffer[64];
    
    u8g2_FirstPage(&u8g2);
    do {
        // Cabeçalho
        u8g2_SetFont(&u8g2, u8g2_font_6x12_tr);
        u8g2_DrawStr(&u8g2, 25, 12, "SYSTEM INFO");
        u8g2_DrawHLine(&u8g2, 0, 15, 128);
        
        // Informações do sistema com fontes pequenas
        u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
        
        // Heap livre
        sprintf(buffer, "Free Heap: %lu", (unsigned long)esp_get_free_heap_size());
        u8g2_DrawStr(&u8g2, 5, 27, buffer);
        
        // Uptime
        uint32_t uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        sprintf(buffer, "Uptime: %lus", (unsigned long)uptime);
        u8g2_DrawStr(&u8g2, 5, 37, buffer);
        
        // Versão IDF
        u8g2_DrawStr(&u8g2, 5, 47, "IDF: ");
        u8g2_DrawStr(&u8g2, 30, 47, esp_get_idf_version());
        
        // Task atual
        sprintf(buffer, "Task: %s", pcTaskGetName(NULL));
        u8g2_DrawStr(&u8g2, 5, 57, buffer);
        
    } while (u8g2_NextPage(&u8g2));
}
*/
//****************************************************************************
/*
void demo_graficos_e_texto(void) {
    static float angulo = 0;
    char buffer[32];
    
    u8g2_FirstPage(&u8g2);
    do {
        // Círculo animado
        int centro_x = 32;
        int centro_y = 32;
        int raio = 20;
        
        // Desenhar círculo
        u8g2_DrawCircle(&u8g2, centro_x, centro_y, raio, U8G2_DRAW_ALL);
        
        // Ponteiro rotativo
        int x = centro_x + (int)(raio * 0.8 * cos(angulo));
        int y = centro_y + (int)(raio * 0.8 * sin(angulo));
        u8g2_DrawLine(&u8g2, centro_x, centro_y, x, y);
        
        // Texto ao lado
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
        u8g2_DrawStr(&u8g2, 70, 15, "Graphics +");
        u8g2_DrawStr(&u8g2, 70, 28, "Text Demo");
        
        sprintf(buffer, "Angle: %.0f", angulo * 180.0 / 3.14159);
        u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
        u8g2_DrawStr(&u8g2, 70, 42, buffer);
        
        // Barra indicadora
        int barra_altura = (int)(20 * (sin(angulo) + 1) / 2);
        u8g2_DrawBox(&u8g2, 70, 50, 10, barra_altura);
        u8g2_DrawFrame(&u8g2, 69, 49, 12, 22);
        
    } while (u8g2_NextPage(&u8g2));
    
    angulo += 0.1f;
    if (angulo > 6.28f) angulo = 0;
}
*/
//****************************************************************************
/*
// Exemplo simples para teste inicial
void demo_simples(void) 
{
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
  u8g2_DrawStr(&u8g2, 0, 15, "ESP32 ST7920 OK!");
  
  u8g2_SetFont(&u8g2, u8g2_font_8x13_tr);
  u8g2_DrawStr(&u8g2, 0, 35, "U8g2 Working");
  
  u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
  u8g2_DrawStr(&u8g2, 0, 60, "SUCCESS");
  u8g2_SendBuffer(&u8g2);
}
*/
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
    .flags = 0,               
  };
  return spi_bus_add_device(SPI2_HOST, &dev, &s_lcd_spi);
}

//****************************************************************************

void vInicioDriverDisplay(void)
{
  ESP_LOGI(TAG, "Iniciando Display"); 

  eModoDisplay=eINIT;

  gpio_set_level(PIN_DISPLAY_DATA, 0);
  for (int i = 0; i < 20; i++) 
    vPulso(PIN_DISPLAY_CLK);

  // Inicializar display
  st7920_spi_init();
  __vInicioDisplay(eTEXT);
  __vGotoXY(0,0); __vStringDisplay("Lin 0");
  __vGotoXY(0,1); __vStringDisplay("Lin 1");
  __vGotoXY(0,2); __vStringDisplay("Lin 2");
  __vGotoXY(0,3); __vStringDisplay("Lin 3");
  
  printf("Display inicializado!\n");

  /*
  // Teste simples primeiro
  demo_simples();
  while(1){
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
  */
/*  
  int demo_atual = 0;
  int contador_delay = 0;
  
  while (1) {
      // Trocar demo a cada 3 segundos (30 * 100ms)
      if (contador_delay >= 30) {
          demo_atual = (demo_atual + 1) % 4;
          contador_delay = 0;
      }
      
      // Executar demo atual
      switch (demo_atual) {
          case 0:
              demo_fontes_diversas();
              break;
          case 1:
              demo_texto_rolante();
              break;
          case 2:
              demo_informacoes_sistema();
              break;
          case 3:
              demo_graficos_e_texto();
              break;
      }
      
      contador_delay++;
      vTaskDelay(pdMS_TO_TICKS(1000));  // 100ms entre frames
  }
*/
  ESP_LOGI(TAG, "Display iniciado"); 
}
