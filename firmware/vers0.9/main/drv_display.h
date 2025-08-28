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
  void __vGotoXY(uchar _ucCol, uchar _ucLin);
  void __vPosCursor(uchar *_pucCol, uchar *_pucLin);
  void __vStringDisplay(char *_pcString);
  void __vInicioDisplay(tModoDisplay _eModoDisplay);

#endif