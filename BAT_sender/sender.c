/*
 * sender.c
 *
 * Created: 14.11.2013 01:03:55
 *  Author: Peter
 */ 
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "sender.h"

uint8_t phase;
uint8_t phase_index;
uint8_t bit_counter;
uint8_t byte_counter;

//uint8_t ocr2a[] = {23, 70, 23, 140};	/* pulse and pause timing  */
uint8_t ocr2a[] = {7, 15, 7, 33, 46, 108};	/* pulse and pause timing 512 탎, 1024 탎, 512 탎, 2176 탎, 3008 탎, 6976 탎 */
/*                                                A ON                     B ON                     C ON                     D ON                   Master ON				Extra ON		*/				
uint8_t bat_cmd_on[CODES][CODE_BYTES]= {{0xa4, 0x60, 0xac, 0xff},{0xad, 0xa4, 0x05, 0xff},{0xa9, 0xe9, 0x8e, 0xff},{0xa0, 0xcb, 0xd7, 0xff},{0xa9, 0xe9, 0x82, 0xff}, {0x2f, 0x8e, 0x1c, 0xff}};
/*                                                A OFF                    B OFF                    C OFF                    D OFF                  Master OFF			    A OFF		*/
uint8_t bat_cmd_off[CODES][CODE_BYTES]={{0xae, 0x05, 0x7c, 0xff},{0xa1, 0x58, 0x95, 0xff},{0xa4, 0x60, 0xae, 0xff},{0xa2, 0x31, 0x37, 0xff},{0xa4, 0x60, 0xa2, 0xff}, {0xae, 0x05, 0x7c, 0xff}};
uint8_t bat_cmd[CODE_BYTES];

void send_command(uint8_t on_off, uint8_t cmd, uint8_t frame)
{
	uint8_t *ptr;
	ptr = on_off ? &bat_cmd_on[cmd][0] : &bat_cmd_off[cmd][0];						/* on or off */
	memcpy(&bat_cmd, ptr, CODE_BYTES);												/* copy code to working area */
	bit_counter = 0;
	byte_counter = 0;
	phase_index = (frame < 5) ? SHORT_START : LONG_START;							/* four frames with "short" startbit followed by four frames with "long" startbit */
	phase = PULSE;
	OCR2A = (bat_cmd[LAST_CODE_BYTE] & 0x01) ? ocr2a[SEND_ZERO] : ocr2a[SEND_ONE];	/* timing until start of next startbit*/
	TIFR2 = _BV(OCF2B);																/* Clear pending interrupts			  */
//	TCCR2B = (_BV(CS22) | _BV(CS21));												/* start timer, clkT2S/256 (From prescaler) */
	TCCR2B = (_BV(CS22) | _BV(CS21) | _BV(CS20));									/* start timer, clkT2S/1024 (From prescaler) */
}

void sender_timer_init (void)
{														
	TCCR2A = (_BV(WGM21)  | _BV(COM2B0));			/* Mode 2 CTC, Toggle OC2B on Compare Match */
}

void sender_timer_enable_irq (void)
{
	TIMSK2 |= _BV(OCIE2B);
}

void sender_init (void)
{
	DDRD |= _BV(PD3);		/* OC2B							*/
	DDRD |= _BV(PD4);		/* Receiver power control	    */
	PORTD |= _BV(PD4);		/* Normal operation				*/
	sender_timer_init();
	sender_timer_enable_irq();
}

ISR (TIMER2_COMPB_vect)
{	
	static uint8_t data_byte;

	if (phase == DONE){
		TCCR2B = 0;													/* stop timer */
		return;											
	}
	if (phase_index > SEND_ONE){									/* start bit */
		OCR2A = ocr2a[phase_index + phase];							/* appropriate timing   */
		phase = 1 - phase;		
		if (!phase) phase_index = SEND_ZERO;
		return;
	}
	if ((bit_counter == 0) && (phase == PULSE)){					/* first, next command byte */
		data_byte =  bat_cmd[byte_counter]; 
		byte_counter++;
	}
	if (phase == PULSE){											/* New bit, are we dealing with a "0" or "1" ?*/														
		phase_index = (data_byte & 0x80) ? SEND_ONE : SEND_ZERO;
		data_byte <<= 1;											/* prepare for next bit */
		bit_counter++;												/* this byte...         */
		bit_counter &= 0x07;										/* ... done ?			*/
	}
	OCR2A = ocr2a[phase_index + phase];								/* appropriate timing   */ 
	phase = 1 - phase;												/* flip phase			*/
	if ((bat_cmd[byte_counter] == 0xff) && (bit_counter == 0)) phase = DONE;
}