// CPU Frequency
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Baud Rate
#define BAUD 9600
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

// Motor Pulse Time (us) _delay_ms() _delay_us()
//#define PULSE_TIME 1000

// Ample amount of steps for system to hit bound switches
#define OOB_STEPS 10000

// Includes
#include <avr/io.h>
#include <util/delay.h>

// Port A
#define EN1     0   // output
#define STP1    1   // output
#define DIR1    2   // output
#define EN2     3   // output
#define STP2    4   // output
#define DIR2    5   // output
#define NAV     6   // output
#define CNFM    7   // input

// Port B
#define LED0    0   // output
#define LED1    1   // output
#define LED2    2   // output
#define LED3    3   // output
#define BTN     4   // input pull-up
#define MOSI    5
#define MISO    6
#define SCK     7

// Port C
#define SA1     0   // input pull-up
#define SA2     1   // input pull-up
#define SA3     2   // input pull-up
#define SA4     3   // input pull-up
#define RED     4   // output
#define GRN     5   // output
#define BLU     6   // output
#define LZR     7   // output

// Port D
#define RXD     0
#define TXD     1
#define GPIO0   2
#define GPIO1   3
#define GPIO2   4
#define GPIO3   5
#define GPIO4   6
#define GPIO5   7

// Global Variables
int x_count;
int y_count;
int x_count_s;
int y_count_s;

int x_loc;
int y_loc;

// steps/pixel
float x_scalar;
float y_scalar;
float x_offset;
float y_offset;

void pin_init() {

    // Set data direction
    DDRA = (1 << EN1) | (1 << STP1) | (1 << DIR1) | (1 << EN2) | (1 << STP2) | (1 << DIR2) | (1 << NAV);
    //DDRA &= ~(1 << CNFM);

    DDRB = (1 << LED0) | (1 << LED1) | (1 << LED2) | (1 << LED3);
    //DDRB &= ~(1 << BTN);

    DDRC = (1 << RED) | (1 << GRN) | (1 << BLU) | (1 << LZR);
    //DDRC &= ~((1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << SA4));

    // Disable Motors
    PORTA = (1 << EN1) | (1 << EN2);

    // Set Button Pull-up
    PORTB = (1 << BTN);

    // Set Switch Pull-ups
    PORTC = (1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << SA4);

}

void set_led(char input) {
    PORTB &= ~0xf;
    PORTB |= (input & 0xf);
}

void set_nav() {
    PORTA |= (1 << NAV);
}

void clear_nav() {
    PORTA &= ~(1 << NAV);
}

void step(int x, int y) {

    // Enable motors
    PORTA &= ~((1 << EN1) | (1 << EN2));

    // x and y bound switch
    char xB, yB;

    // Set X Direction
    if (x < 0) { // -x
        PORTA &= ~(1 << DIR1);
        xB = SA2;
        x = -x;
    }
    else { // +x
        PORTA |= (1 << DIR1);
        xB = SA1;
    }

    // Set Y Direction
    if (y < 0) { // -y
        PORTA |= (1 << DIR2);
        yB = SA4;
        y = -y;
    }
    else { // +y
        PORTA &= ~(1 << DIR2);
        yB = SA3;
    }

    // Clear count values to zero
    x_count = 0;
    y_count = 0;

    // SA switch trigger pulls to zero
    char x_run = 1;
    char y_run = 1;

    // Loops while both axis are below the required steps or SA switch have not been activated
    while (x_run || y_run) {

        if (!(PINB & (1 << BTN))) break;

        if (x_run) x_run = ((x_count < x) && (PINC & (1 << xB)));
        if (y_run) y_run = ((y_count < y) && (PINC & (1 << yB)));

        if (x_run) PORTA |= (1 << STP1);
        if (y_run) PORTA |= (1 << STP2);
        _delay_us(PULSE_TIME/2);

        if (x_run) {PORTA &= ~(1 << STP1); x_count++;}
        if (y_run) {PORTA &= ~(1 << STP2); y_count++;}
        _delay_us(PULSE_TIME/2);

    }

    // Signed count values
    x_count_s = PORTA & (1 << DIR1) ? x_count : -x_count;
    y_count_s = PORTA & (1 << DIR2) ? -y_count : y_count;

    // Disable motors
    PORTA |= (1 << EN1) | (1 << EN2);

}

void home() {
    step(-OOB_STEPS, -OOB_STEPS);
    x_loc = 0;
    y_loc = 0;
}

void move(int x, int y) {
    step(x - x_loc, y - y_loc);
    x_loc = x_loc + x_count_s;
    y_loc = y_loc + y_count_s;
}

void pixel_step(int x, int y) {
    step(x_scalar * x + x_offset, y_scalar * y + y_offset);
}

void pixel_move(int x, int y) {
    move(x_scalar * x + x_offset, y_scalar * y + y_offset);
}

void uart_init() {
    UBRRH = (BAUDRATE >> 8);
    UBRRL = BAUDRATE;
    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void uart_transmit(unsigned char data) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

unsigned char uart_receive() {
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

// MSB first, might need to change
void read_coordinate(int* x, int* y) {
    *x = (int)(uart_receive()) << 8;
    *x |= (int)(uart_receive());
    *y = (int)(uart_receive()) << 8;
    *y |= (int)(uart_receive());
}

void send_coordinate(int x, int y) {
    uart_transmit(x >> 8 & 0xff);
    uart_transmit(x & 0xff);
    uart_transmit(y >> 8 & 0xff);
    uart_transmit(y & 0x0ff);
}
