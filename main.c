/* 
 * File: main.c
 * Description: Memory Hack for Edifier Sound System
 */

#define COMPA_VECT  TIMER1_COMPA_vect  
#define AVR_ATmega8

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>         //nötig für Zahlumwandlung mit itoa
#include "irsnd/irsnd.h" 

#define DENON_ON        0x021C
#define DENON_OFF       0x011C

void timer1_init (void)
{                                                                       // ATmegaXX:
    OCR1A   =  (F_CPU / F_INTERRUPTS) - 1;                                  // compare value: 1/15000 of CPU frequency
    TCCR1B  = (1 << WGM12) | (1 << CS10);                                   // switch CTC Mode on, set prescaler to 1
    TIMSK   = 1 << OCIE1A;                                                  // OCIE1A: Interrupt by timer compare
}
void timer1_deinit (void) {
    TCCR1B  &= ~(1 << WGM12);                                               // switch CTC Mode on, set prescaler to 1
    TIMSK  &= ~(1 << OCIE1A);                                               // OCIE1A: Interrupt by timer compare    
}

void tv_input_init (void) {
    // set PD2 as input 
    DDRD &= ~(1<<DDD2);
    // disable Pull-up 
    PORTD &= ~(1<<PD2);
}

uint8_t get_tv_state (void) {
    return PIND & (1<<PD2);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * timer 1 compare handler, called every 1/15000 sec
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
ISR(COMPA_VECT)                                                             // Timer1 output compare A interrupt service routine, called every 1/15000 sec
{
    (void) irsnd_ISR();                                                     // call irsnd ISR
} 

void sendCommand(uint16_t command)
{
    static IRMP_DATA irmp_data = {IRMP_DENON_PROTOCOL, 0x0008, 0, 0};
    // static IRMP_DATA irmp_data = {IRMP_NEC_PROTOCOL, ~0x00FF, 0, 0};

    irmp_data.command = command;
    irsnd_send_data (&irmp_data, TRUE);
}

void send_tv_state(uint8_t tv_state) {
    if (tv_state) {
        sendCommand(DENON_ON);
    } else {
        sendCommand(DENON_OFF);
    }
}

int main (void) 
{    
    cli();
    timer1_init();
    irsnd_init(); 
    tv_input_init();
    sei();    

    // on power connect check the current state of tv and send it to denon
    _delay_ms(200);
    uint8_t old_tv_state = get_tv_state();
    send_tv_state(old_tv_state);

    for(;;)
    {
        _delay_ms(200);
        uint8_t tv_state = get_tv_state();
        // detect state change of tv
        if (old_tv_state != tv_state) {
            send_tv_state(tv_state);
            old_tv_state = tv_state;
        }
    }
}
