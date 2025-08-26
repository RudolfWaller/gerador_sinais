// drv_display.c  — ST7920 serial (3 fios) robusto p/ ESP-IDF
#include "aplic.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;

// === Configuráveis ===
#define RST_INVERTED            1   // 1 se o /RST do LCD chega INVERTIDO (seu caso)
#define USE_CS_PIN              1   // 1 se o pino PIN_DISPLAY_CS está ligado ao LCD
#define PULSE_CS_EACH_BYTE      1   // 1 = sobe CS antes de cada byte e baixa ao final
#define ST7920_HALFCLK_US       1   // meia-largura do clock (>= ~0,3us por datasheet)
#define ST7920_BYTE_US          120 // >= ~72us entre bytes
#define ST7920_CLEAR_MS         2   // >= ~1,6ms após 0x01/0x02

// --- helpers ---
static inline void _delay_us(uint32_t us) { esp_rom_delay_us(us); }
static inline void SCLK_LOW(void)  { gpio_set_level(PIN_DISPLAY_CLK, 0); }
static inline void SCLK_HIGH(void) { gpio_set_level(PIN_DISPLAY_CLK, 1); }
static inline void SID_WR(int v)   { gpio_set_level(PIN_DISPLAY_DATA, v!=0); }
static inline void CS_WR(int v) {
#if USE_CS_PIN
  gpio_set_level(PIN_DISPLAY_CS, v!=0);
#else
  (void)v;
#endif
}

// pulso de clock com tempos simétricos e setup de dados garantido
static inline void pulse_clk(void) {
  SCLK_HIGH(); _delay_us(ST7920_HALFCLK_US);
  SCLK_LOW();  _delay_us(ST7920_HALFCLK_US);
}

// envia 8 bits MSB-first com setup antes da borda de subida
static inline void shift_out_msb(u8 b) {
  for (int i=0; i<8; i++) {
    SID_WR((b & 0x80) != 0);      // dados estáveis antes da subida
    _delay_us(1);                 // barreira mínima
    pulse_clk();
    b <<= 1;
  }
}

// header 0xF8 (cmd) / 0xFA (data) + 2 bytes com nibbles alinhados em D7..D4
static /*inline*/ void st7920_write_byte(bool is_data, u8 val) {
#if USE_CS_PIN && PULSE_CS_EACH_BYTE
  // recomendação: SCLK=0 e SID estável antes de mexer em CS (datasheet)
  SCLK_LOW(); SID_WR(0);
  CS_WR(1);
#endif
  shift_out_msb(is_data ? 0xFA : 0xF8);
  shift_out_msb( val      & 0xF0);   // nibble alto em D7..D4
  shift_out_msb((val<<4)  & 0xF0);   // nibble baixo em D7..D4
  _delay_us(ST7920_BYTE_US);
#if USE_CS_PIN && PULSE_CS_EACH_BYTE
  // manter SCLK baixo; depois baixar CS (reseta contador serial)
  SCLK_LOW();
  CS_WR(0);
  _delay_us(1);
#endif
}

static inline void st7920_cmd (u8 c) { st7920_write_byte(false, c); }
static inline void st7920_data(u8 d) { st7920_write_byte(true,  d); }

static inline void st7920_reset(void) {
#if RST_INVERTED
  // 1 no ESP -> 0 no LCD (reset ativo)
  gpio_set_level(PIN_RESET, 1); _delay_us(20);
  gpio_set_level(PIN_RESET, 0);
#else
  gpio_set_level(PIN_RESET, 0); _delay_us(20);
  gpio_set_level(PIN_RESET, 1);
#endif
  vTaskDelay(pdMS_TO_TICKS(40));
}

// DDRAM (modo texto). A segunda instrução seleciona coluna (0..15)
static inline void st7920_goto_text(u8 col, u8 row) {
  // Mapeamentos de linha variam por módulo; 0x80,0x90,0x88,0x98 é o mais comum.
  static const u8 base[4] = {0x80, 0x90, 0x88, 0x98};
  row &= 3; col &= 0x0F;
  st7920_cmd(base[row]);     _delay_us(100);
  st7920_cmd(0x80 | col);    _delay_us(100);
}

// ---- API compatível com o que você já chama ----
void vComandoDisplay(uint8_t Com) { st7920_cmd(Com); }
void vDadoDisplay   (uint8_t Dado){ st7920_data(Dado); }
void vGotoXY        (uint8_t c, uint8_t l) { st7920_goto_text(c,l); }

// Init: texto ON e teste rápido
void vInicioDriverDisplay(void)
{
  /*
  // pinos
  gpio_reset_pin(PIN_DISPLAY_CLK);
  gpio_set_direction(PIN_DISPLAY_CLK, GPIO_MODE_OUTPUT);
  SCLK_LOW();

  gpio_reset_pin(PIN_DISPLAY_DATA);
  gpio_set_direction(PIN_DISPLAY_DATA, GPIO_MODE_OUTPUT);
  SID_WR(0);

#if USE_CS_PIN
  gpio_reset_pin(PIN_DISPLAY_CS);
  gpio_set_direction(PIN_DISPLAY_CS, GPIO_MODE_OUTPUT);
  CS_WR(0); // idle em 0 → contador serial resetado
#endif

  gpio_reset_pin(PIN_RESET);
  gpio_set_direction(PIN_RESET, GPIO_MODE_OUTPUT);
#if RST_INVERTED
  gpio_set_level(PIN_RESET, 0);
#else
  gpio_set_level(PIN_RESET, 1);
#endif
  */
  // reset e sequência básica
  //st7920_reset();
  st7920_cmd(0x30); _delay_us(120);   // Basic instruction set
  st7920_cmd(0x0C); _delay_us(120);   // Display ON, cursor OFF
  st7920_cmd(0x06); _delay_us(120);   // Entry mode
  st7920_cmd(0x01); vTaskDelay(pdMS_TO_TICKS(ST7920_CLEAR_MS)); // Clear

  // prova de vida (texto)
  st7920_goto_text(0,0);
  const char *msg = "*OK";
  for (const char *p = msg; *p; ++p) st7920_data((uint8_t)*p);

  // Se quiser entrar em GRÁFICO logo após:
  st7920_cmd(0x30); _delay_us(120);
  st7920_cmd(0x34); _delay_us(120); // extended
  st7920_cmd(0x36); _delay_us(120); // graphic ON
  st7920_data(0x55);
  st7920_data(0x55);
  st7920_data(0x55);
  st7920_data(0x55);
  st7920_data(0x55);
  st7920_data(0x55);
}


#if 0
#include "aplic.h"

//****************************************************************************

//****************************************************************************

#define vEscreveDado(Dado) vEscreveDisplay(Dado, TRUE)
#define vEscreveControle(Dado) vEscreveDisplay(Dado, FALSE)

//****************************************************************************

static /*inline*/ void pulse_clk(void) {
  __asm__ __volatile__("nop");
  esp_rom_delay_us(100);
  // Pulso curto de clock (datasheet pede >300 ns high/low)
  gpio_set_level(PIN_DISPLAY_CLK, 1);
  // margem, pode ser 0..1 us; CPU já atende, mas colocamos um "no-op"
  __asm__ __volatile__("nop");
  esp_rom_delay_us(100);
  gpio_set_level(PIN_DISPLAY_CLK, 0);
  esp_rom_delay_us(100);
}

static /*inline*/ void shift_out_msb(uint8_t byte) {
  for (int i = 0; i < 8; i++) {
    int bit = (byte & 0x80) ? 1 : 0;
    gpio_set_level(PIN_DISPLAY_DATA, bit);
    pulse_clk();
    byte <<= 1;
  }
}

static /*inline*/ void st7920_write_byte(bool is_data, uint8_t val) {
  uint8_t header;

  esp_rom_delay_us(100);

  header = is_data ? 0xFA : 0xF8;  // 1111 1 RW RS 0
  shift_out_msb(header);
  shift_out_msb(val & 0xF0);            // nibble alto em D7..D4
  shift_out_msb((val << 4) & 0xF0);     // nibble baixo em D7..D4
  esp_rom_delay_us(80);
}

static /*inline*/ void st7920_cmd(uint8_t c)  { st7920_write_byte(false, c); }
static /*inline*/ void st7920_data(uint8_t d) { st7920_write_byte(true,  d); }

static /*inline*/ void st7920_goto(uint8_t col, uint8_t row) {
  static const uint8_t base[4] = {0x80, 0x90, 0xA0, 0xB0};
  if (row > 3) 
    row = 0;
  st7920_cmd(base[row] + (col & 0x0F));
}

static void vEscreveDisplay(uchar _ucByte, bool _bControl)
{
  uint8_t _ui8Header = _bControl ? 0xF8 : 0xFA;
 
  vShiftOut(PIN_DISPLAY_DATA, PIN_DISPLAY_CLK, true, _ui8Header);
  vShiftOut(PIN_DISPLAY_DATA, PIN_DISPLAY_CLK, true, _ucByte & 0xF0);       // nibble alto alinhado
  vShiftOut(PIN_DISPLAY_DATA, PIN_DISPLAY_CLK, true, (_ucByte << 4) & 0xF0);// nibble baixo alinhado
  // manter ~72 us entre bytes

  /*
  uint8_t _ui8Cont, _ui8Mascara;  

  gpio_set_level(PIN_DISPLAY_DATA, TRUE);
  for(_ui8Cont=5+1; _ui8Cont!=0; _ui8Cont--)
    vPulso(PIN_DISPLAY_CLK);

  gpio_set_level(PIN_DISPLAY_DATA, _bControl);
  vPulso(PIN_DISPLAY_CLK);

  vPulso(PIN_DISPLAY_CLK); // 0

  _ui8Mascara=0x80;
  for(_ui8Cont=4; _ui8Cont!=0; _ui8Cont--)
  {
    gpio_set_level(PIN_DISPLAY_DATA, _ucByte & _ui8Mascara);
    vPulso(PIN_DISPLAY_CLK);
    _ui8Mascara >>= 1;
  }

  gpio_set_level(PIN_DISPLAY_DATA, FALSE);
  for(_ui8Cont=4; _ui8Cont!=0; _ui8Cont--)
    vPulso(PIN_DISPLAY_CLK);

  for(_ui8Cont=4; _ui8Cont!=0; _ui8Cont--)
  {
    gpio_set_level(PIN_DISPLAY_DATA, _ucByte & _ui8Mascara);
    vPulso(PIN_DISPLAY_CLK);
    _ui8Mascara >>= 1;
  }
      
  gpio_set_level(PIN_DISPLAY_DATA, FALSE);
  for(_ui8Cont=4; _ui8Cont!=0; _ui8Cont--)
    vPulso(PIN_DISPLAY_CLK);
  */
}

//****************************************************************************

void vComandoDisplay(uchar _ucCom)
{
  vEscreveControle(_ucCom);
  esp_rom_delay_us(80);
}

//****************************************************************************

void vDadoDisplay(uchar _ucDado)
{
  vEscreveDado(_ucDado);
  esp_rom_delay_us(80);
}

//****************************************************************************

void vGotoXY(uchar Col, uchar Lin)
{
  vComandoDisplay(0x80 | (Lin & 0x3f)); esp_rom_delay_us(100);
  vComandoDisplay(0x80 | (Col & 0x0f)); esp_rom_delay_us(100);
}

//****************************************************************************

void vInicioDriverDisplay(void)
{
  gpio_set_level(PIN_DISPLAY_CS, TRUE);

  //st7920_reset();
  gpio_set_level(PIN_DISPLAY_DATA, 0);
  for (int i = 0; i < 20; i++) 
    pulse_clk();

  // --- Sequência básica (RE=0) ---
  st7920_cmd(0x30);  // Function Set: 8-bit (não importa no serial), basic instr.
  esp_rom_delay_us(100);
  st7920_cmd(0x0C);  // Display ON, cursor OFF, blink OFF
  esp_rom_delay_us(100);
  st7920_cmd(0x06);  // Entry mode: I/D=1 (incremento), S=0
  esp_rom_delay_us(100);
  st7920_cmd(0x01);  // Clear
  vTaskDelay(2 / portTICK_PERIOD_MS);
  //vTaskDelay(pdMS_TO_TICKS(ST7920_CLEAR_MS));

  // Teste rápido: imprime "* OK"
  st7920_goto(0, 0);
  st7920_data('*');
  /*
  const char *msg = "* OK";
  for (const char *p = msg; *p; ++p) st7920_data((uint8_t)*p);
  */

  gpio_set_level(PIN_DISPLAY_CS, FALSE);

  return;

  vComandoDisplay(0x30); esp_rom_delay_us(100);
  vComandoDisplay(0x0C); esp_rom_delay_us(100); // display ON, cursor off, blink off
  vComandoDisplay(0x06); esp_rom_delay_us(100); // entry mode
  vComandoDisplay(0x01); vTaskDelay(2 / portTICK_PERIOD_MS); // clear (>=1.6ms)
  vGotoXY(0,0);
  vDadoDisplay('*');

  /*  
  vEscreveDado(0);
  vEscreveControle(0);
  esp_rom_delay_us(1000);
  / *
  vComandoDisplay(0x30); esp_rom_delay_us( 100);
  vComandoDisplay(0x30); esp_rom_delay_us( 100);
  vComandoDisplay(0x34); esp_rom_delay_us( 100);
  vComandoDisplay(0x36); esp_rom_delay_us( 100);
  vComandoDisplay(0x03); esp_rom_delay_us( 100);
  vComandoDisplay(0x01); esp_rom_delay_us(2000);
  * /
  vComandoDisplay(0x06); esp_rom_delay_us(100);
  vComandoDisplay(0x08); esp_rom_delay_us(100);
  vComandoDisplay(0x10); esp_rom_delay_us(100);
  vComandoDisplay(0x30); esp_rom_delay_us(100);
  vComandoDisplay(0x01); esp_rom_delay_us(2000);
  vEscreveDado('*');
  */
}
#endif