#if 0
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
// /RST do LCD é ativo-baixo; seu sinal chega INVERTIDO por uma porta:
#define RST_INVERTED            1
// Seu hardware ATUAL deixa o RS/CS do LCD preso em +5V → não há CS controlável:
#define USE_CS_PIN              0
#define PULSE_CS_EACH_BYTE      0
// Clock mais “lento” para shifters MOSFET/bidirecionais:
#define ST7920_HALFCLK_US       5    // ~166 kHz (suba depois de estabilizar)
#define ST7920_BYTE_US          150  // >= ~72 us entre bytes
#define ST7920_CLEAR_MS         2    // >= 1.6 ms após 0x01/0x02

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

static inline void st7920_set_gdram_addr(uint8_t y, uint8_t x) {
  // Y: 0..63 (linhas), X: 0..7 (cada X cobre 16 colunas)
  st7920_cmd(0x80 | (y & 0x3F));   // Set Y address
  st7920_cmd(0x80 | (x & 0x0F));   // Set X address (0..7)
}

// Liga modo gráfico
static void st7920_graphic_on(void) {
  st7920_cmd(0x30);  esp_rom_delay_us(150); // basic
  st7920_cmd(0x34);  esp_rom_delay_us(150); // extended
  st7920_cmd(0x36);  esp_rom_delay_us(150); // graphic ON
}

// Enche a tela com padrão; cada set de X requer DOIS bytes
void st7920_fill_pattern(uint8_t lo, uint8_t hi) {
  st7920_graphic_on();
  for (uint8_t y = 0; y < 64; y++) {
    st7920_set_gdram_addr(y, 0);
    for (uint8_t x = 0; x < 16; x++) { // 16*2 bytes = 256 bits/linha
      st7920_data(hi); // nibble alto (bits 7..0 vão para 8 colunas ímpares)
      st7920_data(lo); // nibble baixo (8 colunas pares)
    }
  }
}

// Init: reset, modo texto ON e teste rápido
void vInicioDriverDisplay(void)
{
  // Garante idle conhecido
  SID_WR(0); SCLK_LOW();
#if USE_CS_PIN
#if PULSE_CS_EACH_BYTE
  CS_WR(0);                      // idle com CS=0 (se estiver ligado)
#else
  CS_WR(1);                      // idle com CS=1 (se estiver ligado)
#endif
#endif
  st7920_reset();               

#if 0
  // Sequência básica (RE=0)
  st7920_cmd(0x30); _delay_us(100);  // Function Set (basic)
  st7920_cmd(0x30); _delay_us(50);   // (mandar 2x ajuda pós-reset)
  st7920_cmd(0x34); _delay_us(50);   // Display ON, cursor OFF
  st7920_cmd(0x36); _delay_us(50);   // Entry mode (incremento)
  st7920_cmd(0x03); vTaskDelay(1+pdMS_TO_TICKS(ST7920_CLEAR_MS)); // Clear

  st7920_data('#'); _delay_us(100);   // Function Set (basic)

#else
  st7920_cmd(0x30); _delay_us(150);   // Function Set (basic)
  st7920_cmd(0x30); _delay_us(150);   // (mandar 2x ajuda pós-reset)
  st7920_cmd(0x0C); _delay_us(150);   // Display ON, cursor OFF
  st7920_cmd(0x06); _delay_us(150);   // Entry mode (incremento)
  st7920_cmd(0x01); vTaskDelay(pdMS_TO_TICKS(ST7920_CLEAR_MS)); // Clear

  st7920_fill_pattern(0x00, 0xFF);

  //st7920_data('#'); _delay_us(100);   // Function Set (basic)
#endif

  // Prova de vida
  st7920_goto_text(0,0);
  const char *msg = "*OK";
  for (const char *p = msg; *p; ++p) st7920_data((uint8_t)*p);
}

#else
#include "aplic.h"

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
}

//****************************************************************************

void vComandoDisplay(uchar _ucCom)
{
  vEscreveControle(_ucCom);
  if(_ucCom == 0x01 || _ucCom == 0x03)
    vTaskDelay(1+pdMS_TO_TICKS(2)); // Clear
  else
    esp_rom_delay_us(80);
}

//****************************************************************************

void vDadoDisplay(uchar _ucDado)
{
  vEscreveDado(_ucDado);
  esp_rom_delay_us(80); // manter ~72 us entre bytes
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
  //st7920_data('*');
  
  const char *msg = "* OK";
  for (const char *p = msg; *p; ++p) st7920_data((uint8_t)*p);
  
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