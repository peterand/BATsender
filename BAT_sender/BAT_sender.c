/*
 * BAT_sender.c
 *
 * Created: 14.11.2013 00:57:52
 *  Author: Peter
 */ 

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "uart.h"
#include "sender.h"

void help_text(void)
{
	 printf_P(PSTR("\nRemote control of BAT(Lidl) RCS AAA3680-A IP20 Electrical Sockets\n"));
	 printf_P(PSTR("Usage:\n"));
	 printf_P(PSTR("Channel A  ON: N A <NL>\n"));
	 printf_P(PSTR("Channel B  ON: N B <NL>\n"));
	 printf_P(PSTR("Channel C  ON: N C <NL>\n"));
	 printf_P(PSTR("Channel D  ON: N D <NL>\n"));
	 printf_P(PSTR("Master     ON: N M <NL>\n\n"));
	 printf_P(PSTR("Channel A OFF: F A <NL>\n"));
	 printf_P(PSTR("Channel B OFF: F B <NL>\n"));
	 printf_P(PSTR("Channel C OFF: F C <NL>\n"));
	 printf_P(PSTR("Channel D OFF: F D <NL>\n"));
	 printf_P(PSTR("Master    OFF: F M <NL>\n\n"));
	 printf_P(PSTR("Keywords and parameters are not case sensitive\n\n"));	 
}

void bat_command(uint8_t on_off, uint8_t cmd)
{
	PORTD &= ~(_BV(PD4));	/* Receiver power down */
	for (uint8_t i=1; i<9; i++){
		send_command(on_off, cmd, i);
		while (bit_is_set(TCCR2B,CS21));
	}
	PORTD |= _BV(PD4);		/* Receiver normal operation */
	printf("OK\n");
}

static void ioinit(void)
{
	uart_init();
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void)
{
	char cbuf[20], s[20];
	ioinit();

	stdout = stdin = &uart_str;
	sender_init();
	sei();
	
	help_text();
	for (;;){
		printf_P(PSTR("Enter command: "));
		if (fgets(cbuf, sizeof cbuf - 1, stdin) == NULL) help_text();
		else{
		  switch (tolower(cbuf[0])){
			default:
				printf("Unknown command: %s\n", cbuf);
				help_text();
				break;
			case 'f':
				if (sscanf(cbuf, "%*s %s", s) == 1){
					switch (tolower(s[0])){
						default:
							printf("Illegal channel: %s\n", cbuf);
							help_text();
							break;
						case 'a':
							bat_command(0,0);
							break;
						case 'b':
							bat_command(0,1);
							break;
						case 'c':
							bat_command(0,2);
							break;
						case 'd':
							bat_command(0,3);
							break;
						case 'm':
							bat_command(0,4);
							break;
					}
				}
				else{
					printf("Syntax error\n");
					help_text();
				}
				break;
			case 'n':
				if (sscanf(cbuf, "%*s %s", s) == 1){
					switch (tolower(s[0])){
						default:
						printf("Illegal channel: %s\n", cbuf);
						help_text();
						break;
						case 'a':
						bat_command(1,0);
						break;
						case 'b':
						bat_command(1,1);
						break;
						case 'c':
						bat_command(1,2);
						break;
						case 'd':
						bat_command(1,3);
						break;
						case 'm':
						bat_command(1,4);
						break;
						case 'x':
						bat_command(1,5);
						break;
					}
				}
				else{
					printf("Syntax error\n");
					help_text();
				}
				break;
		  }
		}
	}
}				