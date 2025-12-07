#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>

#include "dac.h"
#include "nvm.h"


// RAM placeholder for Sine DAC samples
uint16_t DAC_SINE_SAMPLES[DAC_SINE_SIZE];

// Sinewave 48 steps 12-bit resolution: min = 1 max = 4095
const uint16_t DAC_SINWAVE_BASE[DAC_SINE_SIZE] PROGMEM = 
{
	2048, 2315, 2578, 2831, 3071, 3294, 3495, 3672, 3821, 3939, 4025, 4077, 4095,
	4077, 4025, 3939, 3821, 3672, 3495, 3294, 3072, 2831, 2578, 2315, 2048,
	1781, 1518, 1265, 1025, 802, 601, 424, 275, 157, 71, 19, 1,
	19, 71, 157, 275, 424, 601, 802, 1024, 1265, 1518, 1781
};


void dac_enable(void)
{
	DAC_PORT.DIRSET = DAC_CH0_PIN; // DAC0 output
	DAC_PORT.DIRSET = DAC_CH1_PIN; // DAC1 output
	
	DAC_PORT.OUTCLR = DAC_CH0_PIN; // low
	DAC_PORT.OUTCLR = DAC_CH1_PIN; // low

	// enable DAC and channel 0
    DACB.CTRLA = DAC_ENABLE_bm | DAC_CH0EN_bm;
    // single channel
    DACB.CTRLB = DAC_CHSEL_SINGLE_gc;
    // analog supply voltage AREF
    DACB.CTRLC = DAC_REFSEL_AVCC_gc;
    // no event triggers
    DACB.EVCTRL = 0;

	// low level output initially
	DACB.CH0DATA = 0;
	DACB.CH1DATA = 0;

	// load calibration data
	DACB.CH0GAINCAL = nvm_read_byte(NVM_CMD_READ_CALIB_ROW_gc, 
									PRODSIGNATURES_DACB0GAINCAL);
	DACB.CH0OFFSETCAL = nvm_read_byte(NVM_CMD_READ_CALIB_ROW_gc,
									PRODSIGNATURES_DACB0OFFCAL);	
}

void dac_disable(void)
{
	DACB.CH0DATA = 0; // low level output
	DACB.CH1DATA = 0;
	
	DACB.CTRLA = 0; // disable DAC
	
	DAC_PORT.OUTCLR = DAC_CH0_PIN;
	DAC_PORT.OUTCLR = DAC_CH1_PIN;	
}

void dac_timer_start(void)
{
	// start the timer, prescaler 1
    DAC_TIMER.CTRLA = TC_CLKSEL_DIV1_gc;
}

void dac_timer_stop(void)
{
	// stop the timer
	DAC_TIMER.CTRLA = TC_CLKSEL_OFF_gc;
    DAC_TIMER.CNT = 0;
}

void dac_timer_set(uint16_t t)
{
    DAC_TIMER.PER = t;
}

void dac_sine_init(uint16_t freqHz)
{
	uint8_t i = 0;
	uint32_t cnt = 0;
	
	 // copy to RAM - DMA cannot use PROGMEM data !
	for (i = 0; i < DAC_SINE_SIZE; i++){
		DAC_SINE_SAMPLES[i] = (uint16_t)pgm_read_word(&DAC_SINWAVE_BASE[i]);
	}	

	cnt = (uint32_t)F_CPU / (uint32_t)((uint32_t)freqHz * DAC_SINE_SIZE);

    // Setup Timer and Event Trigger Source (overflow)
	DAC_TIMER.CTRLA = TC_CLKSEL_OFF_gc;
    DAC_TIMER.CTRLB = TC_WGMODE_NORMAL_gc;
    DAC_TIMER.PER = (uint16_t)cnt;
    DAC_TIMER.CNT = 0;

    // enable overflow event
    DAC_TIMER.CTRLD = TC_EVACT_RESTART_gc;
    EVSYS.CH0MUX = EVSYS_CHMUX_TCE0_OVF_gc;

	// Enable DMA
	DMA.CTRL  = 0; // off
	DMA.CTRL  = DMA_RESET_bm; // reset
	while ((DMA.CTRL & DMA_RESET_bm) != 0);  // wait until ready
	DMA.CTRL = DMA_ENABLE_bm;// enable	
	
	// Setup DMA sinewave
	DAC_SINE_DMA_CH.CTRLA = DMA_CH_RESET_bm;
    DAC_SINE_DMA_CH.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc; // channel 0
    
    DAC_SINE_DMA_CH.REPCNT = 0; // infinite repeat
	DAC_SINE_DMA_CH.TRFCNT = DAC_SINE_SIZE * 2; // 2 bytes for DAC output sample!
	DAC_SINE_DMA_CH.CTRLA = DMA_CH_BURSTLEN_2BYTE_gc // size 2 bytes
							| DMA_CH_SINGLE_bm       // single channel
							| DMA_CH_REPEAT_bm;;     // repeat DMA transfer

	// Source address (sinewave RAM dac samples)
	DAC_SINE_DMA_CH.SRCADDR0 = ((uintptr_t)DAC_SINE_SAMPLES) & 0xFF;
	DAC_SINE_DMA_CH.SRCADDR1 = ((uintptr_t)DAC_SINE_SAMPLES >> 0x08) & 0xFF;
	DAC_SINE_DMA_CH.SRCADDR2 = ((uintptr_t)DAC_SINE_SAMPLES >> 0x0F) & 0xFF;
	// Destintion address - DAC B
	DAC_SINE_DMA_CH.DESTADDR0 = ((uintptr_t)&DACB.CH0DATA) & 0xFF;
	DAC_SINE_DMA_CH.DESTADDR1 = (((uintptr_t)&DACB.CH0DATA) >> 0x08) & 0xFF;
	DAC_SINE_DMA_CH.DESTADDR2 = (((uintptr_t)&DACB.CH0DATA) >> 0x0F) & 0xFF;

	DAC_SINE_DMA_CH.ADDRCTRL = DMA_CH_DESTRELOAD_BURST_gc  // reload dest addr
								| DMA_CH_SRCRELOAD_TRANSACTION_gc // reload src addr
								| DMA_CH_DESTDIR_INC_gc // increase dest addr
								| DMA_CH_SRCDIR_INC_gc; // increase src addr

	DAC_SINE_DMA_CH.CTRLA |= DMA_CH_ENABLE_bm; // enable DMA module
}
