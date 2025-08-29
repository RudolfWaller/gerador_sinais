#ifndef __DRV_DISPLAY_H
  #define __DRV_DISPLAY_H

  //****************************************************************
  // Constantes

  //****************************************************************
  // Variáveis

  // var int gdriver, gmode, errorcode;
  typedef enum{
    eINIT=0x55,
    eGRAPH=0x5A,
    eTEXT=0xA5
  } tModoDisplay;

  //****************************************************************
  // Rotinas

  void vInicioDriverDisplay(void);
  void __vComandoDisplay(uchar _ucCom);
  void __vDadoDisplay(uchar _ucDado);
  void __vGotoXY(uint8_t _ui8Col, uint8_t _ui8Lin);
  void __vGetCursor(uint8_t *_pui8Col, uint8_t *_pui8Lin);
  void __vSetCursor(uint8_t _ui8Col, uint8_t _ui8Lin);
  void __vStringDisplay(char *_pcString);
  void __vInicioDisplay(tModoDisplay _eModoDisplay);

#endif