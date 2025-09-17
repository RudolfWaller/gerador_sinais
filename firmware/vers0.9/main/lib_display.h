#ifndef LIB_DISPLAY_H
#define LIB_DISPLAY_H

//****************************************************************
// Constantes

#define PIXEL_X 128
#define PIXEL_Y  64

#define FONTE_3x5   0 /* 32 col x 10 lin */
#define FONTE_5x7   1 /* 21 col x  8 lin */
#define FONTE_6x8   2 /* 21 col x  8 lin */
#define FONTE_11x16 3 /* ?? col x ?? lin */

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

typedef uint16_t tTipoDisplay[512];

extern tTipoDisplay vuiEspelhoDisplay;
extern tTipoDisplay vuiDisplay;

var bool bDisplayAlterado;

var uint8_t ui8FonteAtual;
var uint8_t ui8LarguraAscii, ui8AlturaAscii, ui8LinMaxAscii, ui8ColMaxAscii;

#ifdef MAIN
  const ulong cvulTabDec[6+1]={1,10,100,1000,10000,100000,1000000};
#else
  extern const ulong cvulTabDec[6+1];
#endif

extern const tTipoDisplay cvsCamadaMascara[NUM_MASCARAS];

//****************************************************************
// Rotinas

void vInicioBibliotecaDisplay(void);
//void __vDspAtualizaDisplay(void);

void vDspLimpaDisplay(void);
void vDspPonto(int16_t _i16Col, int16_t _i16Lin);
void vDspBranco(int16_t _i16Col, int16_t _i16Lin);
void vDspLinha(int16_t _i16X1, int16_t _i16Y1, int16_t _i16X2, int16_t _i16Y2);
void vDspRetangulo(int16_t _i16Col1, int16_t _i16Lin1, int16_t _i16Col2, int16_t _i16Lin2, bool _bPreenche);
void vDspLimpa(int16_t _i16Col1, int16_t _i16Lin1, int16_t _i16Col2, int16_t _i16Lin2);
void vDspCirculo(int iX0, int iY0, int iRaio);
void vDspDisco(int iX0, int iY0, int iRaio);
void vDspTrapezio(int16_t _i16X0, int16_t _i16Y0, int16_t _i16X1, int16_t _i16Y1, int16_t _i16X2, int16_t _i16Y2, int16_t _i16X3, int16_t _i16Y3, bool _bPreenche);
void vDspPreenche(int16_t _i16Coluna, int16_t _i16Linha);
void vDspAscii(char cCh);
void vDspString(const char *pcCh);
void vPosCursor(int16_t _i16Col, int16_t _i16Lin, bool _bTexto);
void vDspSelecionaMascara(uchar ucNumCamadaMascara);
void vDspFonte(uchar ucNumFonte, bool bInvertido);
void vDspMostraNum(uint16_t _ui16Num, int iNumCaracteres);
void vDspAtualizaDisplay(void);

#endif
