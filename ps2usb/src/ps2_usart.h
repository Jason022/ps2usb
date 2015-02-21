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

#ifndef PS2_USART_H
#define PS2_USART_H

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

// config
/* XCK for clock line and RXD for data line */
#define PS2_CLOCK_PORT  PORTD
#define PS2_CLOCK_PIN   PIND
#define PS2_CLOCK_DDR   DDRD
#define PS2_CLOCK_BIT   4
#define PS2_DATA_PORT   PORTD
#define PS2_DATA_PIN    PIND
#define PS2_DATA_DDR    DDRD
#define PS2_DATA_BIT    0
/* synchronous, odd parity, 1-bit stop, 8-bit data, sample at falling edge */
/* set DDR of CLOCK as input to be slave */
#define PS2_USART_INIT() do {   \
PS2_CLOCK_DDR &= ~(1<<PS2_CLOCK_BIT);   \
PS2_DATA_DDR &= ~(1<<PS2_DATA_BIT);     \
UCSR0C = ((1 << UMSEL00) |  \
(3 << UPM00)   |  \
(0 << USBS0)   |  \
(3 << UCSZ00)  |  \
(0 << UCPOL0));   \
UCSR0A = 0;                 \
UBRR0H = 0;                 \
UBRR0L = 0;                 \
} while (0)
#define PS2_USART_RX_INT_ON() do {  \
    UCSR0B = ((1 << RXCIE0) |       \
              (1 << RXEN0));        \
} while (0)
#define PS2_USART_RX_POLL_ON() do { \
    UCSR0B = (1 << RXEN0);          \
} while (0)
#define PS2_USART_OFF() do {    \
    UCSR0C = 0;                 \
    UCSR0B &= ~((1 << RXEN0) |  \
                (1 << TXEN0));  \
} while (0)
#define PS2_USART_RX_READY      (UCSR0A & (1<<RXC0))
#define PS2_USART_RX_DATA       UDR0
#define PS2_USART_ERROR         (UCSR0A & ((1<<FE0) | (1<<DOR0) | (1<<UPE0)))
#define PS2_USART_RX_VECT       USART_RX_vect

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


extern uint8_t ps2_error;

void ps2_host_init_mouse(void);
uint8_t ps2_host_send_mouse(uint8_t data);
uint8_t ps2_host_recv_response_mouse(void);
uint8_t ps2_host_recv_mouse(void);



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

/*--------------------------------------------------------------------
 * static functions
 *------------------------------------------------------------------*/
static inline void clock_lo(void)
{
    PS2_CLOCK_PORT &= ~(1<<PS2_CLOCK_BIT);
    PS2_CLOCK_DDR  |=  (1<<PS2_CLOCK_BIT);
}
static inline void clock_hi(void)
{
    /* input with pull up */
    PS2_CLOCK_DDR  &= ~(1<<PS2_CLOCK_BIT);
    PS2_CLOCK_PORT |=  (1<<PS2_CLOCK_BIT);
}
static inline bool clock_in(void)
{
    PS2_CLOCK_DDR  &= ~(1<<PS2_CLOCK_BIT);
    PS2_CLOCK_PORT |=  (1<<PS2_CLOCK_BIT);
    _delay_us(1);
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
    PS2_DATA_DDR  &= ~(1<<PS2_DATA_BIT);
    PS2_DATA_PORT |=  (1<<PS2_DATA_BIT);
}
static inline bool data_in(void)
{
    PS2_DATA_DDR  &= ~(1<<PS2_DATA_BIT);
    PS2_DATA_PORT |=  (1<<PS2_DATA_BIT);
    _delay_us(1);
    return PS2_DATA_PIN&(1<<PS2_DATA_BIT);
}

static inline uint16_t wait_clock_lo(uint16_t us)
{
    while (clock_in()  && us) { asm(""); _delay_us(1); us--; }
    return us;
}
static inline uint16_t wait_clock_hi(uint16_t us)
{
    while (!clock_in() && us) { asm(""); _delay_us(1); us--; }
    return us;
}
static inline uint16_t wait_data_lo(uint16_t us)
{
    while (data_in() && us)  { asm(""); _delay_us(1); us--; }
    return us;
}
static inline uint16_t wait_data_hi(uint16_t us)
{
    while (!data_in() && us)  { asm(""); _delay_us(1); us--; }
    return us;
}

/* idle state that device can send */
static inline void idle(void)
{
    clock_hi();
    data_hi();
}

/* inhibit device to send */
static inline void inhibit(void)
{
    clock_lo();
    data_hi();
}

#endif