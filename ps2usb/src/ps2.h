/*
Copyright 2010,2011,2012,2013 Jun WAKO <wakojun@gmail.com>

This software is licensed with a Modified BSD License.
All of this is supposed to be Free Software, Open Source, DFSG-free,
GPL-compatible, and OK to use in both free and proprietary applications.
Additions and corrections to this file are welcome.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

* Neither the name of the copyright holders nor the names of
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PS2_H
#define PS2_H

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
/*
 * Primitive PS/2 Library for AVR
 *
 * PS/2 Resources
 * --------------
 * [1] The PS/2 Mouse/Keyboard Protocol
 * http://www.computer-engineering.org/ps2protocol/
 * Concise and thorough primer of PS/2 protocol.
 *
 * [2] Keyboard and Auxiliary Device Controller
 * http://www.mcamafia.de/pdf/ibm_hitrc07.pdf
 * Signal Timing and Format
 *
 * [3] Keyboards(101- and 102-key)
 * http://www.mcamafia.de/pdf/ibm_hitrc11.pdf
 * Keyboard Layout, Scan Code Set, POR, and Commands.
 *
 * [4] PS/2 Reference Manuals
 * http://www.mcamafia.de/pdf/ibm_hitrc07.pdf
 * Collection of IBM Personal System/2 documents.
 *
 * [5] TrackPoint Engineering Specifications for version 3E
 * https://web.archive.org/web/20100526161812/http://wwwcssrv.almaden.ibm.com/trackpoint/download.html
 */
#define PS2_ACK         0xFA
#define PS2_RESEND      0xFE
#define PS2_SET_LED     0xED

// TODO: error numbers
#define PS2_ERR_NONE        0
#define PS2_ERR_STARTBIT1   1
#define PS2_ERR_STARTBIT2   2
#define PS2_ERR_STARTBIT3   3
#define PS2_ERR_PARITY      0x10
#define PS2_ERR_NODATA      0x20

#define PS2_LED_SCROLL_LOCK 0
#define PS2_LED_NUM_LOCK    1
#define PS2_LED_CAPS_LOCK   2

#define USB_LED_NUM_LOCK                0
#define USB_LED_CAPS_LOCK               1
#define USB_LED_SCROLL_LOCK             2

// config
#define PS2_CLOCK_PORT  PORTD
#define PS2_CLOCK_PIN   PIND
#define PS2_CLOCK_DDR   DDRD
#define PS2_CLOCK_BIT   3
#define PS2_DATA_PORT   PORTD
#define PS2_DATA_PIN    PIND
#define PS2_DATA_DDR    DDRD
#define PS2_DATA_BIT    5
#define PS2_INT_INIT()  do {    \
	EICRA |= ((1<<ISC11) |      \
	(0<<ISC10));				\
	} while (0);
#define PS2_INT_ON()  do {      \
    EIMSK |= (1<<INT1);         \
	} while (0);
#define PS2_INT_OFF() do {      \
    EIMSK &= ~(1<<INT1);        \
	} while (0);
#define USB_INT_ON()  do {      \
	EIMSK |= (1<<INT0);         \
} while (0)
#define USB_INT_OFF() do {      \
    EIMSK &= ~(1<<INT0);        \
} while (0)
#define PS2_INT_VECT    INT1_vect


extern uint8_t volatile ps2_error;

void ps2_host_init(void);
uint8_t ps2_host_send(uint8_t data);
uint8_t ps2_host_recv_response(void);
uint8_t ps2_host_recv(void);
void ps2_host_set_led(uint8_t usb_led);


/* Check port settings for clock and data line */
#if !(defined(PS2_CLOCK_PORT) && \
      defined(PS2_CLOCK_PIN) && \
      defined(PS2_CLOCK_DDR) && \
      defined(PS2_CLOCK_BIT))
#   error "PS/2 clock port setting is required in config.h"
#endif

#if !(defined(PS2_DATA_PORT) && \
      defined(PS2_DATA_PIN) && \
      defined(PS2_DATA_DDR) && \
      defined(PS2_DATA_BIT))
#   error "PS/2 data port setting is required in config.h"
#endif
extern volatile uint8_t requestBit;
/*--------------------------------------------------------------------
 * static functions
 *------------------------------------------------------------------*/
static inline void clock_lo(void)
{
	// output 0
	PS2_CLOCK_PORT &= ~(1<<PS2_CLOCK_BIT);
	PS2_CLOCK_DDR  |=  (1<<PS2_CLOCK_BIT);
}
static inline void clock_release(void)
{
	/* input without pull up */
	PS2_CLOCK_DDR  &= ~(1<<PS2_CLOCK_BIT);
	
}
static inline bool clock_in(void)
{
	//PS2_CLOCK_DDR  &= ~(1<<PS2_CLOCK_BIT);
	//PS2_CLOCK_PORT |=  (1<<PS2_CLOCK_BIT);
	////_delay_us(1);
	//__asm("nop;nop");
	return PS2_CLOCK_PIN&(1<<PS2_CLOCK_BIT);
}
static inline void data_lo(void)
{
	PS2_DATA_PORT &= ~(1<<PS2_DATA_BIT);
	PS2_DATA_DDR  |=  (1<<PS2_DATA_BIT);
}
static inline void data_hi(void)
{
	/* input with pull up */
	PS2_DATA_PORT |=  (1<<PS2_DATA_BIT);
	PS2_DATA_DDR  &= ~(1<<PS2_DATA_BIT);
}
static inline bool data_in(void)
{
	//PS2_DATA_DDR  &= ~(1<<PS2_DATA_BIT);
	//PS2_DATA_PORT |=  (1<<PS2_DATA_BIT);
	//__asm("nop;nop");
	
	return PS2_DATA_PIN&(1<<PS2_DATA_BIT);
	
}
static inline void data_release(void){
	PS2_DATA_DDR  &= ~(1<<PS2_DATA_BIT);
	PS2_DATA_PORT |=  (1<<PS2_DATA_BIT);
}

#endif
