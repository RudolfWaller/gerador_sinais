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
  void vGotoXY(uchar _ucCol, uchar _ucLin);
  void vStringDisplay(char *_pcString);
  void vPrintf(const char *fmt, ...);
  void vInicioDisplay(tModoDisplay _eModoDisplay);

#endif
