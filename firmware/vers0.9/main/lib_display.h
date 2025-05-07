#ifndef LIB_DISPLAY_H
#define LIB_DISPLAY_H

//****************************************************************
// Constantes

#define PIXEL_X 128
#define PIXEL_Y  64

#define FONTE_5x7   0 /* 21 col x  8 lin */
#define FONTE_3x5   1 /* 32 col x 10 lin */
#define FONTE_11x16 2 /* ?? col x ?? lin */

/*

|2
|1
|0123456
+---------------------
*/

#define NUM_MASCARAS  3

#define MASCARA_GERAL 0
#define MASCARA_MENU  1
#define MASCARA_INSTR 2

//****************************************************************
// Variáveis

typedef uint tTipoDisplay[512];

extern tTipoDisplay vuiEspelhoDisplay;
extern tTipoDisplay vuiDisplay;

var bool bDisplayAlterado;

var uchar ucFonteAtual;
var uchar ucLarguraAscii, ucAlturaAscii, ucLinMaxAscii, ucColMaxAscii;

#ifdef MAIN
  const ulong cvulTabDec[6+1]={1,10,100,1000,10000,100000,1000000};
#else
  extern const ulong cvulTabDec[6+1];
#endif

extern const tTipoDisplay cvsCamadaMascara[NUM_MASCARAS];

//****************************************************************
// Rotinas

void vInicioBibliotecaDisplay(void);
void vDspAtualizaDisplay(void);

void vDspLimpaDisplay(void);
void vDspPonto(int iLin, int iCol);
void vDspBranco(int iLin, int iCol);
void vDspLinha(int iX1, int iY1, int iX2, int iY2);
void vDspRetangulo(int iCol1, int iLin1, int iCol2, int iLin2, bool bPreenche);
void vDspLimpa(int iCol1, int iLin1, int iCol2, int iLin2);
void vDspCirculo(int iX0, int iY0, int iRaio);
void vDspDisco(int iX0, int iY0, int iRaio);
void vDspTrapezio(int iX0,int iY0,int iX1,int iY1,int iX2,int iY2,int iX3,int iY3,bool bPreenche);
void vDspPreenche(int iColuna, int iLinha);
void vDspAscii(char cCh);
void vDspString(char *pcCh);
void vDspPosAscii(int8_t i8Col, int8_t i8Lin);
void vDspPosGrAscii(int iCol, int iLin);
void vDspSelecionaMascara(uchar ucNumCamadaMascara);
void vDspFonte(uchar ucNumFonte, bool bInvertido);
void vDspMostraNum(uint uiNum,int iNumCaracteres);

#endif
