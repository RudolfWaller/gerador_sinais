#ifndef __DRV_DDS_H

  //---------------------------------------------------------------------------------

  #ifdef MAIN
    var uint32_t ui32FreqMax=179959130; //(419000L*2^32)/10000000L;
  #else
    var uint32_t ui32FreqMax;
  #endif
  //---------------------------------------------------------------------------------

  void vInitDds(void);
  void vEnviaFreq(uint32_t _ui32Freq, int16_t _i16Fase);

#endif
