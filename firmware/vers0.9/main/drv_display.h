#ifndef __DRV_DISPLAY_H
  #define __DRV_DISPLAY_H

  //****************************************************************
  // Constantes

  //****************************************************************
  // Variáveis

  // var int gdriver, gmode, errorcode;

  //****************************************************************
  // Rotinas

  void vInicioDriverDisplay(void);
  void vComandoDisplay(uchar _ucCom);
  void vDadoDisplay(uchar _ucDado);
  void vGotoXY(uchar _ucCol, uchar _ucLin);

#endif