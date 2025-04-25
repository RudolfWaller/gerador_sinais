#include <aplic.h>

#define ui32AjustaFreq(Freq) 

#define CAL_VAL (-339UL) // trim this with WWV
#define MULT_EN  (1 << 0) // enable multiplier bit
#define REF_CLK  (30000000UL+CAL_VAL) // reference clock 30 MHz plus calibration
#define F_FACTOR (4294967296.0/(REF_CLK*6.0)) // frequency to binary factor
#define P_FACTOR (360.0/32.0) // phase to binary factor

//--------------------------------------------------------------

static uint32_t ui32Freq;
static uint8_t  ui8Fase;
static const char *TAG = "drv_DDS";

//--------------------------------------------------------------

void vInitDds(void)
{
  ESP_LOGI(TAG, "Iniciando DDS"); 

  vPulso(PIN_DDS_WCLK);
  vPulso(PIN_DDS_FQ_UD);

  vEnviaFreq(0, 0);

  ESP_LOGI(TAG, "DDS iniciado");
}

void vEnviaFreq(uint32_t _ui32Freq, uint8_t _ui8Fase)
{
  // -----------------------------------------------------------------------------------------------------------------------------
  // Word | Data[7]        | Data[6]  | Data[5]  | Data[4]  | Data[3]        | Data[2]    | Data[1]   | Data[0]
  // -----------------------------------------------------------------------------------------------------------------------------
  // W0   | Phase–b4 (MSB) | Phase–b3 | Phase–b2 | Phase–b1 | Phase–b0 (LSB) | Power-Down | Log ic 0* | 6xREFCLK Multiplier Enable
  // W1   | Freq–b31 (MSB) | Freq–b30 | Freq–b29 | Freq–b28 | Freq–b27       | Freq–b26   | Freq–b25  | Freq–b24
  // W2   | Freq–b23       | Freq–b22 | Freq–b21 | Freq–b20 | Freq–b19       | Freq–b18   | Freq–b17  | Freq–b16
  // W3   | Freq–b15       | Freq–b14 | Freq–b13 | Freq–b12 | Freq–b11       | Freq–b10   | Freq–b9   | Freq–b8
  // W4   | Freq–b7        | Freq–b6  | Freq–b5  | Freq–b4  | Freq–b3        | Freq–b2    | Freq–b1   | Freq–b0 (LSB)

  uint8_t _ui8Cont;

  ESP_LOGI(TAG, "Enviando frequência de %ld Hz e fase de %d", _ui32Freq, _ui8Fase);

  ui32Freq=_ui32Freq;
  ui8Fase =_ui8Fase;

  _ui32Freq=((uint64_t) _ui32Freq)*((uint64_t) 1000L)/((uint64_t) 42L);
  _ui8Fase=(_ui8Fase << 3) | 0b001;

  // now we itterate through the first 32 bits, 8 at a time, streaming to the DataPin.
  for (_ui8Cont=4; _ui8Cont!=0; _ui8Cont--, _ui32Freq>>=8) 
  {
    //ESP_LOGI(TAG, "W%d=%x", _ui8Cont, (uint8_t) (_ui32Freq & 0xFF));

    // stream out bits to DataPin, pulsing clock pin
    vShiftOut(PIN_DDS_DATA, PIN_DDS_WCLK, FALSE, (_ui32Freq & 0xFF));
  }

  //ESP_LOGI(TAG, "W0=%x", _ui8Fase & 0xFF);

  // now send the final 8 bits to complete the 40 bit instruction
  vShiftOut(PIN_DDS_DATA, PIN_DDS_WCLK, FALSE, _ui8Fase & 0xFF);

  // and once all 40 bits have been sent
  // finally we toggle the load bit to say we are done
  // and let the AD9851 do its stuff
  vPulso(PIN_DDS_FQ_UD);
}

//--------------------------------------------------------------

/*
int AD9851::setPhase (int phase)
{
	// constrain phase value to 0...359
	// note: phase wraps around 359->0
	while (phase < 1) {
		phase += 360;
	}

	while (phase > 359) {
		phase -= 360;
	}

	// derive phase value
	_phase = (phase / P_FACTOR);
	_phase <<= 3;
	_phase |= MULT_EN;

	_update(); // send phase data

	// convert it back to show what is ACTUALLY set
	phase = (_phase & ~MULT_EN);
	phase >>= 3;
	phase *= P_FACTOR;
	return phase;
}

uint32_t AD9851::setFreq (uint32_t freq)
{
	// derive register value
	_freq = (uint32_t)((freq * F_FACTOR) + 0.5);
	_update(); // send it
	// convert it back to show what is ACTUALLY set
	return (uint32_t)((_freq / F_FACTOR) + 0.5);
}

*/