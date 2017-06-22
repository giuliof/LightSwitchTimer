// Internal 128 kHz Oscillator
// CKSEL Fuses to “0100”
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <inttypes.h>

// 0x00 = OFF - 0x01 = on - 0x02 timer to off (a define could be useful)
volatile uint8_t status = 0x00;
volatile uint8_t timer  = 0;
void relay_pulse();
void goto_sleep();

ISR(INT0_vect){
    // change interrupt level
    //~ MCUCR &= (_BV(ISC00) | _BV(ISC01));
    // check status
    // if off, turn on
    if (status == 0x00) {
        PORTB |= _BV(PB1);
        relay_pulse();
        
        status = 0x01;
        sleep_cpu();
    }
    // if on, set timer
    else if (status == 0x01) {
        status = 0x02;
        //~ TCNT0  = 0xFF - 146;
        TCCR0A &= ~(_BV(WGM00) | _BV(WGM01));
        TCNT0   = 0x00;
        TCCR0B |= (_BV(CS02) | _BV(CS00));
    }
    else {
        TCCR0B &= ~(_BV(CS02) | _BV(CS00));
        timer = 0;
        relay_pulse();
        PORTB &= ~_BV(PB1);
        
        status = 0x00;
        sleep_cpu();
    }
    _delay_ms(100);
    // clear button interrupt generated from bouncing
    GIFR |= _BV(INTF0);
}   

ISR(TIM0_OVF_vect){
    if (timer == 3840) {
        TCCR0B &= ~(_BV(CS02) | _BV(CS00));
        
        timer = 0;
        PORTB &= ~(_BV(PB1));
        relay_pulse();
        
        status = 0x00;
        sleep_cpu();
    }
    else {
        PORTB ^= _BV(PB1);
        timer++;
    }
}

void relay_pulse() {
    PORTB |= _BV(PB3);
    _delay_ms(100);
    PORTB &= ~_BV(PB3);
}

void goto_sleep() {
    //~ MCUCR &= ~(_BV(ISC00) | _BV(ISC01));
    sleep_cpu();
}

int main() {
    // Change main clock prescaler to N=256
    // unlock prescaler
    CLKPR = _BV(CLKPCE);
    // set prescaling factor
    CLKPR = _BV(CLKPS3)
    
    PLLCSR &= ~_BV(PLLE);

    // set up IO
    // relay PIN in output (PB3) and signaling LED (PB1)
    DDRB  |= _BV(PB1) | _BV(PB3);
    // button PIN in pullup (PB2)
    PORTB |= _BV(PB2);  // Pull-up on PB2 (button pin)
    
    PORTB |= _BV(PB1);
    _delay_ms(1000);
    PORTB ^= _BV(PB1);
    _delay_ms(1000);
    
    // timer overflow interrupt
    TIMSK  |= _BV(TOIE0);
    // set up button pin interrupt (low level- only for wakeup)
    MCUCR |= (_BV(ISC00) | _BV(ISC01));
    //~ MCUCR &= (_BV(ISC00) | _BV(ISC01));
    // Sleep mode power-down
    MCUCR |= _BV(SM1);
    GIMSK |= _BV(INT0);
    
    sei();
    
    sleep_cpu();
    
    while(1) {
        
    }
}

//~ TCCR0B |= _BV(CS02) | _BV(CS00);
