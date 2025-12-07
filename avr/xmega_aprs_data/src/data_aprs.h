#ifndef _XMEGA_DATA_APRS_H_
#define _XMEGA_DATA_APRS_H_

#include <stdint.h>


/******************************************************************************/
#define TIMER_MODEM 			TCE1 			// TCD0
#define TIMER_MODEM_OVF_vect 	TCE1_OVF_vect 	// TCD0_OVF_vect
#define SINEWAVE_TIMER 			TCE0 			// used to trigger DMA->DAC transfer
/******************************************************************************/

#define DATA_APRS_RESULT_OK		(0)
#define DATA_APRS_RESULT_ERROR	(1)

#define DATA_FREQ_MARK  		1200  // Hz
#define DATA_FREQ_SPACE			2200  // Hz
#define DATA_PREAMBLE_SIZE		20    // bytes
#define DATA_TRAIL_SIZE			5     // bytes

#define AFSK_TX_FLAG_BUSY		0 // bit 0 - starting TX
#define AFSK_TX_FLAG_PREAMBLE	1 // preamble TX completed
#define AFSK_TX_FLAG_DATA		2 // data TX completed
#define AFSK_TX_FLAG_TRAIL		3 // trail TX completed
#define AFSK_TX_FLAG_DONE		7 // packet data sent

/*
 * Sinewave 48 uint16_t samples (96 bytes) @ 32 MHz clock
 * formula is:
 *            (F_CPU / timer_prescaler) / (FREQ * SINEWAVE_SAMPLES_N)
 * 1200 Hz:   (32 000 000 / 1) / (1200 [Hz] * 48) = 32 000 000 / 57600  = 555
 * 2200 Hz:   (32 000 000 / 1) / (2200 [Hz] * 48) = 32 000 000 / 105600 = 303
 */
#define DATA_TIMER_PER_MARK  	(555)
#define DATA_TIMER_PER_SPACE 	(303)



typedef struct
{
	volatile uint8_t flags;
	volatile uint8_t * data_ptr;
	volatile uint16_t data_length;
	volatile uint16_t data_tx_n;	// total bytes transmitted
	volatile uint8_t bit_index;
	volatile uint8_t bit_one_counter;
	volatile uint8_t current_byte;
	volatile uint8_t sync_n;	// preamble and trail bytes counter

} afsk_tx_t;

extern afsk_tx_t * afsk_tx;



void data_aprs_init(void);
void data_aprs_end(void);
void data_aprs_send(uint8_t * data, uint16_t data_lenght);


#endif /* _XMEGA_DATA_APRS_H_ */
