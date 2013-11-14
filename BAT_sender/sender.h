/*
 * sender.h
 *
 * Created: 14.11.2013 01:26:06
 *  Author: Peter
 */ 


#ifndef SENDER_H_
#define SENDER_H_

#include <inttypes.h>

#define SEND_ONE 1
#define SEND_ZERO 0
#define PULSE 0
#define PAUSE 1
#define DONE 2
#define SHORT_START 2
#define LONG_START 4
#define STOP_PULSE 0
#define START_PAUSE 3
#define SPACE_PAUSE 1
#define LAST_CODE_BYTE 2
#define CODE_BYTES 4
#define CODES 6

extern void sender_init(void);
extern void send_command(uint8_t on_off, uint8_t cmd, uint8_t frame);

#endif /* SENDER_H_ */