#ifndef __DRV_HARDWARE_H
  #define __DRV_HARDWARE_H

  #define PIN_RESET          5
  #define PIN_LED            2 
  #define PIN_ADC           27

  #define PIN_SHREG_HOLD    22
  #define PIN_SHREG_DATAOUT  4
  #define PIN_SHREG_DATAIN  21
  #define PIN_SHREG_CLK     19

  #define PIN_DISPLAY_CLK   18
  #define PIN_DISPLAY_DATA  23

  #define PIN_DDS_ADJ       25
  #define PIN_DDS_FQ_UD     17
  #define PIN_DDS_WCLK      16
  #define PIN_DDS_DATA      33
  #define PIN_ENC1_A        36
  #define PIN_ENC1_B        39
  #define PIN_ENC2_A        34
  #define PIN_ENC2_B        35

  //---------------------------------------------------------------------------------

  //---------------------------------------------------------------------------------

  void vInicioDriverHardware(void);
  void vReset(void);
  void vPulso(uint8_t _ui8Pin);
  void vShiftOut(gpio_num_t _gDataPin, gpio_num_t _gClockPin, bool _bMsbFirst, uint8_t _ui8_Dado);

#endif
