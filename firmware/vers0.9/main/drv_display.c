#include "aplic.h"

//****************************************************************************

//****************************************************************************

#define vEscreveDado(Dado) vEscreveDisplay(Dado, TRUE)
#define vEscreveControle(Dado) vEscreveDisplay(Dado, FALSE)

//****************************************************************************

static void vEscreveDisplay(uchar _ucByte, bool _bControl)
{
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

}

//****************************************************************************

void vComandoDisplay(uchar Com)
{
  vEscreveDado(Com);
  esp_rom_delay_us(1);
}

//****************************************************************************

void vDadoDisplay(uchar Dado)
{
  vEscreveDado(Dado);
  esp_rom_delay_us(20);
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
  vEscreveDado(0);
  vEscreveControle(0);
  esp_rom_delay_us(1000);
  /*
  vComandoDisplay(0x30); esp_rom_delay_us( 100);
  vComandoDisplay(0x30); esp_rom_delay_us( 100);
  vComandoDisplay(0x34); esp_rom_delay_us( 100);
  vComandoDisplay(0x36); esp_rom_delay_us( 100);
  vComandoDisplay(0x03); esp_rom_delay_us( 100);
  vComandoDisplay(0x01); esp_rom_delay_us(2000);
  */
 vComandoDisplay(0x06); esp_rom_delay_us(100);
 vComandoDisplay(0x08); esp_rom_delay_us(100);
 vComandoDisplay(0x10); esp_rom_delay_us(100);
 vComandoDisplay(0x30); esp_rom_delay_us(100);
 vComandoDisplay(0x01); esp_rom_delay_us(2000);
 vEscreveDado('*');
}
