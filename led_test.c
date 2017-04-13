#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

int main() {
    
    DDRB = 0xf;
    char count = 0;

    while (1) {
        PORTB = count & 0xf;
        count++;
        _delay_ms(250);
    }

    return 0;
}
