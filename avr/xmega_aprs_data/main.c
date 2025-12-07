#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "src/osc.h"
#include "src/dac.h"
#include "src/data_aprs.h"


/*
Test APRS packet example:

ISM434>APRS,WIDE1-1,WIDE2-1:!5427.00N/01831.00EBPHG1310 ISM434.775 \
APRS/BBS/DTMF/VOICE 1200bps WX+Alerts!/A=000394

AX.25 Packet [118 bytes]:

82 a0 a4 a6 40 40 60        [ APRS ]
92 a6 9a 68 66 68 60        [ ISM434 ]
ae 92 88 8a 62 40 62        [ WIDE1-1 ]
ae 92 88 8a 64 40 63        [ WIDE2-1 ]
03 f0                       [ packet flags and type ]

21 35 34 32 37 2e 30 30 4e 2f 30 31 38 33 31 2e    !5427.00N/01831.
30 30 45 42 50 48 47 31 33 31 30 20 49 53 4d 34    00EBPHG1310 ISM4
33 34 2e 37 37 35 20 41 50 52 53 2f 42 42 53 2f    34.775 APRS/BBS/
44 54 4d 46 2f 56 4f 49 43 45 20 31 32 30 30 62    DTMF/VOICE 1200b
70 73 20 57 58 2b 41 6c 65 72 74 73 21 2f 41 3d    ps WX+Alerts!/A=
30 30 30 33 39 34                                  000394

a3 6d                       [ CRC ] 
*/
void data_send_test(void)
{
	uint8_t data[] = 
	{
		0x82, 0xa0, 0xa4, 0xa6, 0x40, 0x40, 0x60,
		0x92, 0xa6, 0x9a, 0x68, 0x66, 0x68, 0x60,
		0xae, 0x92, 0x88, 0x8a, 0x62, 0x40, 0x62,
		0xae, 0x92, 0x88, 0x8a, 0x64, 0x40, 0x63,
		0x03, 0xf0,
		0x21, 0x35, 0x34, 0x32, 0x37, 0x2e, 0x30, 0x30, 
		0x4e, 0x2f, 0x30, 0x31, 0x38, 0x33, 0x31, 0x2e,
		0x30, 0x30, 0x45, 0x42, 0x50, 0x48, 0x47, 0x31, 
		0x33, 0x31, 0x30, 0x20, 0x49, 0x53, 0x4d, 0x34,
		0x33, 0x34, 0x2e, 0x37, 0x37, 0x35, 0x20, 0x41, 
		0x50, 0x52, 0x53, 0x2f, 0x42, 0x42, 0x53, 0x2f,
		0x44, 0x54, 0x4d, 0x46, 0x2f, 0x56, 0x4f, 0x49,
		0x43, 0x45, 0x20, 0x31, 0x32, 0x30, 0x30, 0x62,
		0x70, 0x73, 0x20, 0x57, 0x58, 0x2b, 0x41, 0x6c,
		0x65, 0x72, 0x74, 0x73, 0x21, 0x2f, 0x41, 0x3d,
		0x30, 0x30, 0x30, 0x33, 0x39, 0x34,
		0xa3, 0x6d  // CRC
	};
	uint16_t data_lenght = 118;
	
	
	// TODO: Radio PTT on
	
	data_aprs_send(data, data_lenght);
	while(!(afsk_tx->flags |= (1 << AFSK_TX_FLAG_DONE))); // wait for data sent event

	// TODO: Radio PTT off
}

int main(void)
{
	// disable JTAG
	CCP = CCP_IOREG_gc;       // Enable change to IOREG
	MCU.MCUCR = MCU_JTAGD_bm; // Setting this bit will disable the JTAG interface	

	osc_32_MHz(); // set clock speed to 32MHz
	
	data_aprs_init();
	
	// send data packet 3 times every 5 seconds
	uint8_t i = 3;

	while(i > 0)
	{
		data_send_test();
		
		_delay_ms(5000);
		
		i--;
	}
	
	data_aprs_end(); // disable data sending
	
	while(1){};
}
