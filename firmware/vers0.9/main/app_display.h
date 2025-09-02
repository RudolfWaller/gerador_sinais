#ifndef __APP_DISPLAY_H
  #define __APP_DISPLAY_H

  //****************************************************************
  // Constantes

  //****************************************************************
  // Variáveis

  //****************************************************************
  // Rotinas

  void vInicioAppDisplay(void);

  void vDadoDisplay(uchar _ucDado);
  void vLimpaDisplay(void);
  void vGotoXY(uint8_t _ui8Col, uint8_t _ui8Lin);
  void vStringDisplay(char *_pcString);
  void vPrintf(const char *fmt, ...);
  void vPosPrintf(uint8_t _ui8Col, uint8_t _ui8Lin, const char *fmt, ...);
  void vInicioDisplay(tModoDisplay _eModoDisplay);

#endif
