#ifndef _XMEGA_DAC_H_
#define _XMEGA_DAC_H_

#include <stdint.h>


/******************************************************************************/
#define DAC_PORT          PORTB
#define DAC_CH0_PIN       PIN2_bm  // DAC 0
#define DAC_CH1_PIN       PIN3_bm  // DAC 1

#define DAC_TIMER         TCE0      // used to trigger DMA->DAC transfer
#define DAC_SINE_DMA_CH   DMA.CH1
/******************************************************************************/

#define DAC_SINE_SIZE   48	// 16-bit points (samples)
extern uint16_t DAC_SINE_SAMPLES[DAC_SINE_SIZE]; // sinewave samples

// DMA timer settings for particular frequencies @ 32MHZ clock
#define DAC_SINE_FREQ_1200_HZ   (555)
#define DAC_SINE_FREQ_2200_HZ   (303)


void dac_enable(void);
void dac_disable(void);

void dac_timer_start(void);
void dac_timer_stop(void);
void dac_timer_set(uint16_t t);

void dac_sine_init(uint16_t freqHz);


#endif /* _XMEGA_DAC_H_ */
