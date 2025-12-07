#include <avr/io.h>

#include "osc.h"


void osc_2_MHz(void)
{
	// setup 2 Mhz crystal
    OSC.CTRL |= OSC_RC2MEN_bm;
    // wait
    while(!(OSC.STATUS & OSC_RC2MRDY_bm));
    // trigger protection mechanism
    CPU_CCP = CCP_IOREG_gc;
    // enable internal 2 Mhz crystal
    CLK.CTRL = CLK_SCLKSEL_RC2M_gc;
    // disable 32 Mhz oscillator 
    OSC.CTRL &= ~OSC_RC32MEN_bm;
}

void osc_32_MHz(void)
{
	// Configure clock to 32MHz
	
	// Enable the internal 32MHz & 32KHz oscillators
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;
	// Wait for 32Khz oscillator to stabilize
	while(!(OSC.STATUS & OSC_RC32KRDY_bm));
	// Wait for 32MHz oscillator to stabilize
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));

	// A factory-calibrated value is loaded from the signature row of the device
	// and written to this register during reset, giving an oscillator frequency 
	// close to 32.768kHz.
	// OSC.RC32KCAL = nvm_calibration_read_byte(PRODSIGNATURES_RCOSC32K);

	// Enable DFLL - defaults to calibrate against internal 32Khz clock
	DFLLRC32M.CTRL = DFLL_ENABLE_bm;
	// Disable register security for clock update
	CCP = CCP_IOREG_gc;
	// Switch to 32MHz clock
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
	// Disable 2Mhz oscillator
	OSC.CTRL &= ~OSC_RC2MEN_bm;
}
