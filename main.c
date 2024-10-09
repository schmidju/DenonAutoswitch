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

void adc_init(void) {    
    ADMUX = (1<<REFS0);               // Set Reference to AVCC and input to ADC0
    ADCSRA = (1<<ADFR)|(1<<ADEN)      // Enable ADC, set prescaler to 128
            |(1<<ADPS2)|(1<<ADPS1)
            |(1<<ADPS0);              // Fadc=Fcpu/prescaler=8000000/128=62.5kHz
                                      // Fadc should be between 50kHz and 200kHz
    ADCSRA |= (1<<ADSC);              // Start the first conversion
}
void adc_deinit(void) {
    ADCSRA = 0x00;
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

int main (void) 
{    
    cli();
    timer1_init();
    irsnd_init(); 
    adc_init();                                                          // initialize irsnd
    sei();    

    _delay_ms(200);

    sendCommand(DENON_ON);
    // on power_on of tv, usb slot is turned on, turned off after 4 seconds and turned on again
    // lets wait 5 seconds so DENON_OFF is only possible if tv was at least 5 seconds on
    _delay_ms(5000);

    // set running to 1 only if there is no power los detected
    uint8_t running = ADC < 1023;

    // loop until powerloss gets detected and settings has been stored
    while(running)
    {
        if(ADC == 1023){
            // power loss detected. disable everything to save energy
            adc_deinit();
            sendCommand(DENON_OFF);
            running = FALSE;
        }
    }

    // main() will never be left
    for(;;);
}
