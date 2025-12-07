#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "data_aprs.h"
#include "dac.h"


afsk_tx_t afsk_tx_obj;
afsk_tx_t * afsk_tx;


/* Initialize Timers, DMA and DAC */
void data_aprs_init(void)
{
	dac_enable();
}

/* Disable Timer, DMA and DAC */
void data_aprs_end(void)
{
	dac_disable();
	
	DMA.CTRL  = 0; // DMA off
}

inline void sinewave_start(void)
{
    SINEWAVE_TIMER.PER = DATA_TIMER_PER_MARK;
    SINEWAVE_TIMER.CTRLA = TC_CLKSEL_DIV1_gc; //start the timer
}

inline void sinewave_shift(void)
{
    // This function makes timer max value alternate between
    // mark and space symbol timing. It executes in constant time.
    // Counter value has to be reset every time for consistent timing.
    SINEWAVE_TIMER.PER = (DATA_TIMER_PER_MARK + DATA_TIMER_PER_SPACE) - SINEWAVE_TIMER.PER;
    SINEWAVE_TIMER.CNT = 0;
}

inline void sinewave_off(void)
{
	// stop the timer
    SINEWAVE_TIMER.CTRLA = TC_CLKSEL_OFF_gc;
    SINEWAVE_TIMER.CNT = 0;
}



/* Send data packet */
void data_aprs_send(uint8_t * data, uint16_t data_lenght)
{
	afsk_tx = &afsk_tx_obj;
	memset(afsk_tx, 0, sizeof(afsk_tx_t));
	
	dac_sine_init(DATA_FREQ_MARK);

    // HDLC bit Timer @32MHz clock: 
    // 1200 bps =  32000000 / 1200 = 26666
    TIMER_MODEM.CTRLA = TC_CLKSEL_OFF_gc; // timer off
    TIMER_MODEM.CTRLB = TC_WGMODE_NORMAL_gc; // count up to PER
    TIMER_MODEM.INTCTRLA = TC_OVFINTLVL_HI_gc; // overflow interrupt enable
    TIMER_MODEM.PER = 26666; // this gives 1199,985 interrupts per second (baud)
    TIMER_MODEM.CNT = 0; // counter value

	afsk_tx->data_length = data_lenght;
	afsk_tx->data_ptr = data;
	afsk_tx->current_byte = 0x7e; // AX.25 preamble byte
	afsk_tx->bit_one_counter = 0;
	afsk_tx->sync_n = DATA_PREAMBLE_SIZE;
	
	afsk_tx->flags |= (1 << AFSK_TX_FLAG_BUSY);

	// start sinewave generator, use Vcc as output reference
    sinewave_start();
	// start HDLC bit timer: no prescaling, timer clk = 32MHz
    TIMER_MODEM.CTRLA = TC_CLKSEL_DIV1_gc;
}

inline void afsk_tx_complete(void)
{
	sinewave_off(); // disable signal generator
	afsk_tx->flags |= (1 << AFSK_TX_FLAG_DONE);
}

/* Interrupt to transmit bit at 1200 bps */
ISR(TIMER_MODEM_OVF_vect)
{
    // bit stuffing - check if 5 consecutive ones apperaed - add extra zero
    // but don't stuff the flag
    if (afsk_tx->bit_one_counter == 5 && afsk_tx->current_byte != 0x7e)
    {
        sinewave_shift(); // bit 0 = shift frequency
        
        afsk_tx->bit_one_counter = 0;
        return; //don't move to next bit, do it at the next invocation
    }

	// zero bit - shift frequency
    if (!(afsk_tx->current_byte & (1 << afsk_tx->bit_index)))
    {
        sinewave_shift(); // bit 0 = shift frequency
        afsk_tx->bit_one_counter = 0;
    }
    else // one bit - do not change frequency
    {
        //'1' bit is no change in frequency
        afsk_tx->bit_one_counter++;
    }

    afsk_tx->bit_index++;

	// whole byte was sent
    if (afsk_tx->bit_index == 8)
    {
        afsk_tx->bit_index = 0;
        afsk_tx->data_tx_n++;
        
        // check if preamble was sent
		if(!(afsk_tx->flags & (1 << AFSK_TX_FLAG_PREAMBLE)))
		{
			afsk_tx->sync_n--;
			
			if(afsk_tx->sync_n == 0){
				afsk_tx->flags |= (1 << AFSK_TX_FLAG_PREAMBLE);// preamble completed
			}else{
				afsk_tx->current_byte = 0x7e;
				return;
			}
		}        
    
        // if preamble sent, move to data
        if(!(afsk_tx->flags & (1 << AFSK_TX_FLAG_DATA)))
        {
			afsk_tx->current_byte = *afsk_tx->data_ptr++;

			if (afsk_tx->data_length == 0)
			{
				afsk_tx->flags |= (1 << AFSK_TX_FLAG_DATA);// data completed
				afsk_tx->current_byte = 0x7e; // set trail data
				afsk_tx->sync_n = DATA_TRAIL_SIZE;
				return;
			}
			
			afsk_tx->data_length--;
			return;
		}
		
		if(!(afsk_tx->flags & (1 << AFSK_TX_FLAG_TRAIL)))
        {
			afsk_tx->sync_n--;
			
			if(afsk_tx->sync_n == 0)
			{
				afsk_tx->flags |= (1 << AFSK_TX_FLAG_TRAIL);// trail completed
				
				TIMER_MODEM.CTRLA = TC_CLKSEL_OFF_gc; // disable modem bit clock

				// data sent completed event
				afsk_tx_complete();
						
			}
			else
			{
				afsk_tx->current_byte = 0x7e;
				return;
			}			
		}
    }
}
