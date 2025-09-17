#include "aplic.h"

//#define ATUALIZA_DISPLAY

//****************************************************************************

#include "lib_display.inc"

static const char *TAG = "lib_DISP";

const uint16_t cvuiMascara[16]={0x8000,0x4000,0x2000,0x1000,
                                0x0800,0x0400,0x0200,0x0100,
                                0x0080,0x0040,0x0020,0x0010,
                                0x0008,0x0004,0x0002,0x0001};

const struct{
  const char *pccFonte;
  char cAltura, cLargura;
  char cMaxLinhas, cMaxColunas;
} cvsFontes[]={{ccFont3x5,                 5, 3,PIXEL_Y/(5+1), PIXEL_X/(3+1)},
               {ccFont5x7,                 7, 5,PIXEL_Y/(7+1), PIXEL_X/(5+1)},
               {ccFont6x8,                 8, 6,PIXEL_Y/(8+1), PIXEL_X/(6+1)},
               {(const char *)ciFont11x16,15,11,PIXEL_Y/(15+1),PIXEL_X/(11+1)},
               };

//****************************************************************************

tTipoDisplay vuiDisplay, vuiEspelhoDisplay;
int16_t  i16LinAscii, i16ColAscii;
uchar ucInvAscii;
uchar ucMascaraAtual;

//****************************************************************************

static bool bDspLePonto(int16_t _i16Col, int16_t _i16Lin)
{
  uint16_t _ui16Endereco;

  if(_i16Lin>=0 && _i16Lin<PIXEL_Y && _i16Col>=0 && _i16Col<PIXEL_X){
    _i16Lin=63-_i16Lin;
    _ui16Endereco=PIXEL_X*((PIXEL_Y-1)-_i16Lin)+_i16Col;
    return((vuiDisplay[_ui16Endereco/16] & cvuiMascara[_ui16Endereco % 16])!=0L);
  }
  else return(TRUE);
}

//****************************************************************************


void vDspPonto(int16_t _i16Col, int16_t _i16Lin)
{
  uint16_t _ui16Endereco;

  if(_i16Lin>=0 && _i16Lin<PIXEL_Y && _i16Col>=0 && _i16Col<PIXEL_X){
    _i16Lin=63-_i16Lin;
    _ui16Endereco=PIXEL_X*((PIXEL_Y-1)-_i16Lin)+_i16Col;
    vuiDisplay[_ui16Endereco/16] |= cvsCamadaMascara[ucMascaraAtual][_ui16Endereco/16] & cvuiMascara[_ui16Endereco % 16];

    bDisplayAlterado=TRUE;
    #ifdef ATUALIZA_DISPLAY
      __vDspAtualizaDisplay();
    #endif
  }
}

//****************************************************************************

void vDspBranco(int16_t _i16Col, int16_t _i16Lin)
{
  if(_i16Lin>=0 && _i16Lin<PIXEL_Y && _i16Col>=0 && _i16Col<PIXEL_X){
    uint16_t uiEndereco;

    _i16Lin=63-_i16Lin;
  
    uiEndereco=PIXEL_X*((PIXEL_Y-1)-_i16Lin)+_i16Col;
    vuiDisplay[uiEndereco/16] &= ~(cvsCamadaMascara[ucMascaraAtual][uiEndereco/16] & cvuiMascara[uiEndereco % 16]);

    bDisplayAlterado=TRUE;
    #ifdef ATUALIZA_DISPLAY
      __vDspAtualizaDisplay();
    #endif
  }
}

//****************************************************************************

void vDspLinha(int16_t _i16X1, int16_t _i16Y1, int16_t _i16X2, int16_t _i16Y2)
{
  int16_t _i16Tmp;
  int16_t _i16X,_i16Y;
  int16_t _i16Dx, _i16Dy;
  int16_t _i16Erro;
  int16_t _i16PassoY;

  bool _bTrocaXY = FALSE;

  // ???
  /* no BBX intersection check at the moment, should be added... */
  // ???

  if(_i16X1==_i16X2){
    if(_i16Y1>_i16Y2){
      while(_i16Y2<=_i16Y1)
        vDspPonto(_i16X1,_i16Y2++);
    }
    else{
      while(_i16Y1<=_i16Y2)
        vDspPonto(_i16X1,_i16Y1++);
    }
    return;
  }
  else if(_i16Y1==_i16Y2){
    if(_i16X1>_i16X2){
      while(_i16X2<=_i16X1)
        vDspPonto(_i16X2++,_i16Y1);
    }
    else{
      while(_i16X1<=_i16X2)
        vDspPonto(_i16X1++,_i16Y1);
    }
    return;
  }

  if ( _i16X1 > _i16X2 )
    _i16Dx = _i16X1-_i16X2;
  else
    _i16Dx = _i16X2-_i16X1;

  if ( _i16Y1 > _i16Y2 )
    _i16Dy = _i16Y1-_i16Y2;
  else
    _i16Dy = _i16Y2-_i16Y1;

  if ( _i16Dy > _i16Dx ){
    _bTrocaXY = 1;
    _i16Tmp = _i16Dx; _i16Dx =_i16Dy; _i16Dy = _i16Tmp;
    _i16Tmp = _i16X1; _i16X1 =_i16Y1; _i16Y1 = _i16Tmp;
    _i16Tmp = _i16X2; _i16X2 =_i16Y2; _i16Y2 = _i16Tmp;
  }

  if ( _i16X1 > _i16X2 ){
    _i16Tmp = _i16X1; _i16X1 =_i16X2; _i16X2 = _i16Tmp;
    _i16Tmp = _i16Y1; _i16Y1 =_i16Y2; _i16Y2 = _i16Tmp;
  }

  _i16Erro = _i16Dx >> 1;
  if ( _i16Y2 > _i16Y1 )
    _i16PassoY= 1;
  else
    _i16PassoY=-1;

  _i16Y = _i16Y1;
  for( _i16X = _i16X1; _i16X <= _i16X2; _i16X++ ){
    if ( !_bTrocaXY)
      vDspPonto(_i16X, _i16Y);
    else
      vDspPonto(_i16Y, _i16X);
    _i16Erro -= (char)_i16Dy;
    if ( _i16Erro < 0 ){
      _i16Y += (int)_i16PassoY;
      _i16Erro += (int)_i16Dx;
    }
  }
}

//****************************************************************************

void vDspLimpa(int16_t _i16Col1, int16_t _i16Lin1, int16_t _i16Col2, int16_t _i16Lin2)
{
  int16_t _i16C;
  int16_t _i16Temp;

  if(_i16Lin2<_i16Lin1){
    _i16Temp=_i16Lin2;
    _i16Lin2=_i16Lin1;
    _i16Lin1=_i16Temp;
  }

  if(_i16Col2<_i16Col1){
    _i16Temp=_i16Col2;
    _i16Col2=_i16Col1;
    _i16Col1=_i16Temp;
  }

  while(_i16Lin1<=_i16Lin2){
    for(_i16C=_i16Col1;_i16C<=_i16Col2;_i16C++)
      vDspBranco(_i16C,_i16Lin1);
    _i16Lin1++;
  }
}

//****************************************************************************

void vDspRetangulo(int16_t _i16Col1, int16_t _i16Lin1, int16_t _i16Col2, int16_t _i16Lin2, bool _bPreenche)
{
  int16_t _i16C;
  int16_t _i16Temp;

  if(_bPreenche){
    if(_i16Lin2<_i16Lin1){
      _i16Temp=_i16Lin2;
      _i16Lin2=_i16Lin1;
      _i16Lin1=_i16Temp;
    }

    if(_i16Col2<_i16Col1){
      _i16Temp=_i16Col2;
      _i16Col2=_i16Col1;
      _i16Col1=_i16Temp;
    }

    while(_i16Lin1<=_i16Lin2){
      for(_i16C=_i16Col1;_i16C<=_i16Col2;_i16C++)
        vDspPonto(_i16C,_i16Lin1);
      _i16Lin1++;
    }
  }
  else{
    vDspLinha(_i16Col1,_i16Lin1,_i16Col2,_i16Lin1);
    vDspLinha(_i16Col2,_i16Lin1,_i16Col2,_i16Lin2);
    vDspLinha(_i16Col1,_i16Lin1,_i16Col1,_i16Lin2);
    vDspLinha(_i16Col1,_i16Lin2,_i16Col2,_i16Lin2);
  }
}

//****************************************************************************

#define vPush(_i16Coluna, _i16Linha)        \
  do                                        \
  {                                         \
    if(!bDspLePonto(_i16Coluna, _i16Linha)) \
    {                                       \
      _pui8StackHead->i16Coluna=_i16Coluna; \
      _pui8StackHead->i16Linha =_i16Linha;  \
      _pui8StackHead++;                     \
    }                                       \
  } while (FALSE)  

void vDspPreenche(int16_t _i16Coluna, int16_t _i16Linha)
{
  typedef struct{
    int16_t i16Coluna;
    int16_t i16Linha;
  } tTipoCoordenada;

  tTipoCoordenada *_pui8Stack, *_pui8StackTail, *_pui8StackHead;

  _pui8StackTail=_pui8StackHead=_pui8Stack=malloc(65536/*10000 128*64*sizeof(tTipoCoordenada)*/);

  if(_pui8Stack==NULL)
    return;

  if(!bDspLePonto(_i16Coluna, _i16Linha)){
    _pui8StackHead->i16Coluna=_i16Coluna;
    _pui8StackHead->i16Linha =_i16Linha;
    _pui8StackHead++;

    while(_pui8StackHead!=_pui8StackTail)
    {
      if(!bDspLePonto(_pui8StackTail->i16Coluna, _pui8StackTail->i16Linha))
      {
        vDspPonto(_pui8StackTail->i16Coluna, _pui8StackTail->i16Linha);
        vPush(_pui8StackTail->i16Coluna+1, _pui8StackTail->i16Linha  );
        vPush(_pui8StackTail->i16Coluna-1, _pui8StackTail->i16Linha  );
        vPush(_pui8StackTail->i16Coluna  , _pui8StackTail->i16Linha+1);
        vPush(_pui8StackTail->i16Coluna  , _pui8StackTail->i16Linha-1);
      }
      _pui8StackTail++;
    }
  }

  xxx=_pui8StackHead-_pui8Stack;
  free(_pui8Stack);
  return;
}

//****************************************************************************

int16_t i16MenorValor,i16MaiorValor;

static void vCalcMenorMaiorValor(int16_t _i16Num);
static void vCalcMenorMaiorValor(int16_t _i16Num)
{
  if(_i16Num<i16MenorValor)
    i16MenorValor=_i16Num;

  if(_i16Num>i16MaiorValor)
    i16MaiorValor=_i16Num;
}

//****************************************************************************

void vDspTrapezio(int16_t _i16X0, int16_t _i16Y0, int16_t _i16X1, int16_t _i16Y1, int16_t _i16X2, int16_t _i16Y2, int16_t _i16X3, int16_t _i16Y3, bool _bPreenche)
{
  vDspLinha(_i16X0,_i16Y0,_i16X1,_i16Y1);
  vDspLinha(_i16X1,_i16Y1,_i16X2,_i16Y2);
  vDspLinha(_i16X2,_i16Y2,_i16X3,_i16Y3);
  vDspLinha(_i16X3,_i16Y3,_i16X0,_i16Y0);

  if(_bPreenche){
    int iXMedio,iYMedio;

    i16MenorValor= 1000;
    i16MaiorValor=-1000;

    vCalcMenorMaiorValor(_i16X0);
    vCalcMenorMaiorValor(_i16X1);
    vCalcMenorMaiorValor(_i16X2);
    vCalcMenorMaiorValor(_i16X3);

    iXMedio=(i16MaiorValor+i16MenorValor)/2;

    i16MenorValor= 1000;
    i16MaiorValor=-1000;

    vCalcMenorMaiorValor(_i16Y0);
    vCalcMenorMaiorValor(_i16Y1);
    vCalcMenorMaiorValor(_i16Y2);
    vCalcMenorMaiorValor(_i16Y3);

    iYMedio=(i16MaiorValor+i16MenorValor)/2;

    vDspPreenche(iXMedio,iYMedio);
  }
}

//****************************************************************************

#define DRAW_UPPER_RIGHT 0x01
#define DRAW_UPPER_LEFT  0x02
#define DRAW_LOWER_RIGHT 0x04
#define DRAW_LOWER_LEFT  0x08

#define vDesenhaLinhaVertical(iX, iY, iDy) vDspLinha(iX,iY,iX,iY+iDy)
//#define vDrawHLine(iX, iY, iDx) vDspLinha(iX,iY,iX+iDx,iY)

static void vDesenhaSecaoCirculo(int iX, int iY, int iX0, int iY0, uchar ucQuadrante)
{
    /* upper right */
    if ( ucQuadrante & DRAW_UPPER_RIGHT ){
      vDspPonto(iX0 + iX,iY0 - iY);
      vDspPonto(iX0 + iY,iY0 - iX);
    }

    /* upper left */
    if ( ucQuadrante & DRAW_UPPER_LEFT ){
      vDspPonto(iX0 - iX,iY0 - iY);
      vDspPonto(iX0 - iY,iY0 - iX);
    }

    /* lower right */
    if ( ucQuadrante & DRAW_LOWER_RIGHT ){
      vDspPonto(iX0 + iX,iY0 + iY);
      vDspPonto(iX0 + iY,iY0 + iX);
    }

    /* lower left */
    if ( ucQuadrante & DRAW_LOWER_LEFT ){
      vDspPonto(iX0 - iX,iY0 + iY);
      vDspPonto(iX0 - iY,iY0 + iX);
    }
}

//****************************************************************************

static void vDesenhaCirculo(int iX0, int iY0, int iRaio, uchar ucQuadrante)
{
    int iF;
    int iDdF_X;
    int iDdF_Y;
    int iIX;
    int iIY;

    iF = 1;
    iF -= iRaio;
    iDdF_X = 1;
    iDdF_Y = 0;
    iDdF_Y -= iRaio;
    iDdF_Y *= 2;
    iIX = 0;
    iIY = iRaio;

    vDesenhaSecaoCirculo(iIX, iIY, iX0, iY0, ucQuadrante);

    while ( iIX < iIY ){
      if (iF >= 0){
        iIY--;
        iDdF_Y += 2;
        iF += iDdF_Y;
      }
      iIX++;
      iDdF_X += 2;
      iF += iDdF_X;

      vDesenhaSecaoCirculo(iIX, iIY, iX0, iY0, ucQuadrante);
    }
}

//****************************************************************************

void vDspCirculo(int iX0, int iY0, int iRaio)
{
  /* check for bounding box */
  {
    int iRaioP, iRaioP2;

    iRaioP = iRaio;
    iRaioP++;
    iRaioP2 = iRaioP;
    iRaioP2 *= 2;
  }

  /* draw circle */
  vDesenhaCirculo(iX0, iY0, iRaio, DRAW_UPPER_RIGHT |
                                   DRAW_UPPER_LEFT  |
                                   DRAW_LOWER_RIGHT |
                                   DRAW_LOWER_LEFT);
}

//****************************************************************************

static void vDesenhaSecaoDisco(int iX, int iY, int iX0, int iY0, uchar ucQuadrante)
{
  /* upper right */
  if ( ucQuadrante & DRAW_UPPER_RIGHT ){
    vDesenhaLinhaVertical(iX0+iX, iY0-iY, iY+1);
    vDesenhaLinhaVertical(iX0+iY, iY0-iX, iX+1);
  }

  /* upper left */
  if ( ucQuadrante & DRAW_UPPER_LEFT ){
    vDesenhaLinhaVertical(iX0-iX, iY0-iY, iY+1);
    vDesenhaLinhaVertical(iX0-iY, iY0-iX, iX+1);
  }

  /* lower right */
  if ( ucQuadrante & DRAW_LOWER_RIGHT ){
    vDesenhaLinhaVertical(iX0+iX, iY0, iY+1);
    vDesenhaLinhaVertical(iX0+iY, iY0, iX+1);
  }

  /* lower left */
  if ( ucQuadrante & DRAW_LOWER_LEFT ){
    vDesenhaLinhaVertical(iX0-iX, iY0, iY+1);
    vDesenhaLinhaVertical(iX0-iY, iY0, iX+1);
  }
}

//****************************************************************************

static void vDesenhaDisco(int iX0, int iY0, int iRaio, uchar ucQuadrante)
{
  int iF;
  int iDdF_X;
  int iDdF_Y;
  int iIX;
  int iIY;

  iF = 1;
  iF -= iRaio;
  iDdF_X = 1;
  iDdF_Y = 0;
  iDdF_Y -= iRaio;
  iDdF_Y *= 2;
  iIX = 0;
  iIY = iRaio;

  vDesenhaSecaoDisco(iIX, iIY, iX0, iY0, ucQuadrante);

  while ( iIX < iIY ){
    if (iF >= 0){
      iIY--;
      iDdF_Y += 2;
      iF += iDdF_Y;
    }
    iIX++;
    iDdF_X += 2;
    iF += iDdF_X;

    vDesenhaSecaoDisco(iIX, iIY, iX0, iY0, ucQuadrante);
  }
}

//****************************************************************************

void vDspDisco(int iX0, int iY0, int iRaio)
{
  /* check for bounding box */
  {
    int iRaioP, iRaioP2;

    iRaioP = iRaio;
    iRaioP++;
    iRaioP2 = iRaioP;
    iRaioP2 *= 2;
  }

  /* draw disc */
  vDesenhaDisco(iX0, iY0, iRaio, DRAW_UPPER_RIGHT |
                                 DRAW_UPPER_LEFT  |
                                 DRAW_LOWER_RIGHT |
                                 DRAW_LOWER_LEFT);
}

//****************************************************************************

void vDspAscii(char cCh)
{
  const char *pccAscii=NULL;
  char cL,cC,cB,cColuna;

  const int *pciAscii=NULL;
  uint16_t uiB,uiColuna,uiInvAscii;

  if(ui8AlturaAscii>8){
    if(ucInvAscii)
      uiInvAscii=0xffff;
    else
      uiInvAscii=0x0000;

    if(cCh>' '){
      pciAscii=(const int *)cvsFontes[ui8FonteAtual].pccFonte;
      pciAscii+=ui8LarguraAscii*(cCh-'!');
    }

    for(cC=0;cC<ui8LarguraAscii;cC++){
      if(cCh>' ')
        uiColuna=*pciAscii++;
      else
        uiColuna=0x0000;

      uiB=(uint16_t) (1<<(ui8AlturaAscii-1));
      for(cL=0;cL<ui8AlturaAscii;cL++){
        if((uiColuna ^ uiInvAscii) & uiB)
          vDspPonto(i16ColAscii+cC,i16LinAscii+cL);
        else
          vDspBranco(i16ColAscii+cC,i16LinAscii+cL);
        uiB>>=1;

      }
    }
  }
  else{
    if(cCh>' '){
      pccAscii=cvsFontes[ui8FonteAtual].pccFonte;
      pccAscii+=ui8LarguraAscii*(cCh-'!');
    }

    for(cC=0;cC<ui8LarguraAscii;cC++){
      if(cCh>' ')
        cColuna=*pccAscii++;
      else
        cColuna=0x0000;

      cB=(char) (1<<(ui8AlturaAscii-1));
      for(cL=0;cL<ui8AlturaAscii;cL++){
        if((cColuna ^ ucInvAscii) & cB)
          vDspPonto(i16ColAscii+cC,i16LinAscii+cL);
        else
          vDspBranco(i16ColAscii+cC,i16LinAscii+cL);
        cB>>=1;
      }
    }
  }
  i16ColAscii+=ui8LarguraAscii+1;

  if(ucInvAscii){
    vDspLinha(i16ColAscii-1,i16LinAscii+ui8AlturaAscii-1,i16ColAscii-1,i16LinAscii);
  }

  if(i16ColAscii>PIXEL_X-ui8LarguraAscii){
    i16ColAscii=0;
    i16LinAscii-=ui8AlturaAscii+1;
  }
}

//****************************************************************************

void vDspString(const char *pcCh)
{
  while(*pcCh)
    vDspAscii(*pcCh++);
}

//****************************************************************************

void vDspMostraNum(uint16_t uiNum,int iNumCaracteres)
{
  char  vcString[10];
  uchar ucPos;

  if(uiNum==0){
    vcString[0]='0';
    ucPos=1;
  }
  else{
    for(ucPos=0;uiNum!=0;ucPos++){
      vcString[ucPos]=(char)('0'+uiNum % 10);
      uiNum/=10;
    }
  }

  if(iNumCaracteres>0)
    while(ucPos<iNumCaracteres)
      vcString[ucPos++]=' ';
  else{
    iNumCaracteres=0-iNumCaracteres;
    while(ucPos<iNumCaracteres)
      vcString[ucPos++]='0';
  }

  vcString[ucPos]=0;

  //------------------------

  ucPos--;
  while(iNumCaracteres--)
    vDspAscii(vcString[ucPos--]);
}

//****************************************************************************

void vPosCursor(int16_t _i16Col, int16_t _i16Lin, bool _bTexto)
{
  if(_bTexto)
  {
    if(_i16Lin>=ui8LinMaxAscii || _i16Lin<0 ||
      _i16Col>=ui8ColMaxAscii || _i16Col<0)
    {
      DoNothing();
    }
    else
    {
      i16LinAscii=63-_i16Lin*(ui8AlturaAscii+1)-(ui8AlturaAscii);
      i16ColAscii=_i16Col*(ui8LarguraAscii+1);
    }
  }
  else
  {
    if(_i16Lin>=PIXEL_Y || _i16Lin<0 ||
       _i16Col>=PIXEL_X || _i16Col<0)
    {
      DoNothing();
    }
    else
    {
      i16LinAscii=_i16Lin;
      i16ColAscii=_i16Col;
    }
  }
}

//****************************************************************************

void vDspFonte(uchar ucNumFonte, bool bInvertido)
{
  ui8FonteAtual  =ucNumFonte;

  ui8LarguraAscii=cvsFontes[ui8FonteAtual].cLargura;
  ui8AlturaAscii =cvsFontes[ui8FonteAtual].cAltura;

  ui8LinMaxAscii =cvsFontes[ui8FonteAtual].cMaxLinhas;
  ui8ColMaxAscii =cvsFontes[ui8FonteAtual].cMaxColunas;

  if(bInvertido)
    ucInvAscii=(uchar)0xff;
  else
    ucInvAscii=0x00;
}

//****************************************************************************

void vDspSelecionaMascara(uchar ucNumCamadaMascara)
{
  ucMascaraAtual=ucNumCamadaMascara;
}

//****************************************************************************

void vDspLimpaDisplay(void)
{
  const uint16_t *_pcui16Mascara;
  uint16_t *_pui16Display,*_pui16EspelhoDisplay;
  uint16_t  _ui16Cont;

  _pcui16Mascara      =&cvsCamadaMascara[ucMascaraAtual][0];
  _pui16Display       =&vuiDisplay[0];
  _pui16EspelhoDisplay=&vuiEspelhoDisplay[  0];

  for(_ui16Cont=512;_ui16Cont!=0;_ui16Cont--)
  {
    *_pui16Display++        &= ~(*_pcui16Mascara);
    *_pui16EspelhoDisplay++ |= *_pcui16Mascara++;
  }

  bDisplayAlterado=TRUE;
}

//****************************************************************************

void __vGotoXYGraph(uint8_t _ui8Col, uint8_t _ui8Lin)
{
  uint8_t _ui8linhaDisplay;
  uint8_t _ui8ColunaDisplay;

  _ui8Lin=63-_ui8Lin;
  _ui8linhaDisplay =_ui8Lin % 32;
  _ui8ColunaDisplay=_ui8Col/16;
  
  if(_ui8Lin>=32)
    _ui8ColunaDisplay+=8;

  __vComandoDisplay(0x80 | _ui8linhaDisplay);    // Define o endereço Y (linha)
  __vComandoDisplay(0x80 | _ui8ColunaDisplay);   // Define o endereço X (coluna em bytes)
}

//****************************************************************************

void __vDspAtualizaDisplay2(tTipoDisplay _tDisplay)
{
  uint8_t  _ui8C,_ui8L;
  bool     _bPosiciona;
  uint16_t *_pui16Display,*_pui16EspelhoDisplay;
  uint16_t _ui16Dado;

  if(bDisplayAlterado && eModoDisplay==eGRAPH)
  {
    _pui16Display       =&_tDisplay[ 0];
    _pui16EspelhoDisplay=&vuiEspelhoDisplay[  0];

    for(_ui8L=0;_ui8L<PIXEL_Y;_ui8L++){
      _bPosiciona=TRUE;

      for(_ui8C=0;_ui8C<PIXEL_X;_ui8C+=16){
        _ui16Dado=*_pui16Display++;
        if(_ui16Dado!=*_pui16EspelhoDisplay){
          *_pui16EspelhoDisplay++=_ui16Dado;
          if(_bPosiciona){
            __vGotoXYGraph(_ui8C,_ui8L);
            _bPosiciona=FALSE;
          }

          __vEscreveDisplay(_ui16Dado>>8, FALSE);
          __vEscreveDisplay(_ui16Dado & 0xff, FALSE);
          __vSetCursor(_ui8C+16, _ui8L);
        }
        else{
          _bPosiciona=TRUE;
          _pui16EspelhoDisplay++;
        }
      }
    }
    bDisplayAlterado=FALSE;
  }
}

//****************************************************************************

void vDspAtualizaDisplay(void)
{
  __vDspAtualizaDisplay2(vuiDisplay);
}

//****************************************************************************
#if 0
static void TesteFont(void)
{
  uint8_t _ui8Lin, _ui8Col;

  _ui8Col=0;
  vDspFonte(FONTE_11x16, FALSE);
  for(_ui8Lin=0;_ui8Lin<4; _ui8Lin++)
  {
    vPosCursor(_ui8Col, _ui8Lin, TRUE);  
    vDspAscii(_ui8Lin+'0');
  }

  _ui8Col=2;
  vDspFonte(FONTE_6x8, FALSE);
  for(_ui8Lin=0;_ui8Lin<7; _ui8Lin++)
  {
    vPosCursor(_ui8Col, _ui8Lin, TRUE);  
    vDspAscii(_ui8Lin+'0');
  }

  _ui8Col=4;
  vDspFonte(FONTE_5x7, FALSE);
  for(_ui8Lin=0;_ui8Lin<8; _ui8Lin++)
  {
    vPosCursor(_ui8Col, _ui8Lin, TRUE);  
    vDspAscii(_ui8Lin+'0');
  }

  _ui8Col=9;
  vDspFonte(FONTE_3x5, FALSE);
  for(_ui8Lin=0;_ui8Lin<10; _ui8Lin++)
  {
    vPosCursor(_ui8Col, _ui8Lin, TRUE);  
    vDspAscii(_ui8Lin+'0');
  }

  //----------------------

  vDspRetangulo(42, 47, 60, 63, TRUE);
  vDspTrapezio(64, 44, 80, 63, 110, 58, 97, 40, TRUE);

  vDspFonte(FONTE_11x16, TRUE);
  vPosCursor(44, 24, FALSE);  
  vDspString("Esp");

/*  
//  vDspFonte(FONTE_3x5, FALSE);
//  vDspFonte(FONTE_5x7, FALSE);
//  vDspFonte(FONTE_6x8, FALSE);
  vDspFonte(FONTE_11x16, FALSE);
  vPosCursor(0, 0, TRUE);
  vDspString("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F");
*/  
/*
  vDspString("Lin 0");
  vPosCursor(0, 1, TRUE);
  vDspString("Lin 1");
  vPosCursor(0, 2, TRUE);
  vDspString("Lin 2");
  vPosCursor(0, 3, TRUE);
  vDspString(" 12345MN");
  vDspString("0123456789012345678901234567890123456789");
  //vGravaTabCaracteres();
*/  
}
#endif
//****************************************************************************

void vInicioBibliotecaDisplay(void)
{
  ESP_LOGI(TAG, "Iniciando display LIB");

  __vInicioDisplay(eGRAPH);

  //xTaskCreate(vTaskBibDisplay, "Task_Display", 4096, NULL, 1, NULL);
  
//void vDspLimpaDisplay(void)

  /*  
  vDspPonto(10,53);
  vDspPonto(118,10);
  vDspPonto(118,53);
  */

  /*
  vDspLinha(100,0,127,48);
  vDspLinha(100,48,127,48);
  vDspLinha(100,0,100,48);
  __vDspAtualizaDisplay();
  vTaskDelay(pdMS_TO_TICKS(5000));  // 100ms entre frames
  */
  /* */
  //TesteFont();
  /* */
  //vDspDisco(50,32,15);

//  DspRetangulo(0,0,127,63,TRUE);

//  DspLinha(0,0,127,63);
//  DspLinha(0,63,127,0);

//  vDspAtualizaDisplay();
//  exit(0);

  vDspAtualizaDisplay();

  ESP_LOGI(TAG, "display LIB iniciado");
}
