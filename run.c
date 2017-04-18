#define PULSE_TIME 1000
#include "fwa.h"

#define DANCE 125

#define RX_Calibration   0x01
#define RX_Halt          0x02
#define RX_Recalibration 0x03
#define RX_Pseudocation  0x04

#define COORD 0x10

#define TX_Coordinate    0x01
#define TX_Roaming       0x02
#define TX_Location      0x03
#define TX_Shutdown      0x04
#define TX_Positioning   0x05

int main() {

    pin_init();
    uart_init();

    char state = 0;
    char rx = 0x00;
    int x, y;

    // state 2 variables
    int x_max, y_max;
    int x_step[4], y_step[4];
    int x_pixel[4], y_pixel[4];

    while (1) {
        set_led(state);
        switch (state) {

            case 0:
                for (int i = 0; i < 2; i++) {
                    set_led(1);
                    _delay_ms(DANCE);
                    set_led(2);
                    _delay_ms(DANCE);
                    set_led(4);
                    _delay_ms(DANCE);
                    set_led(8);
                    _delay_ms(DANCE);
                    set_led(4);
                    _delay_ms(DANCE);
                    set_led(2);
                    _delay_ms(DANCE);
                }

                state = 1;
                break;

            case 1:
                while (rx != RX_Calibration) rx = uart_receive();
                state = 2;
                break;

            case 2:

                // Clear scalar & offset values
                x_scalar = 0;
                y_scalar = 0;
                x_offset = 0;
                y_offset = 0;

                // Get x max
                home();
                step(+OOB_STEPS, 0);
                x_max = x_count;
                step(-OOB_STEPS, 0);
                x_max += x_count;
                x_max = x_max / 2;

                // Get y max
                home();
                step(0, +OOB_STEPS);
                y_max = y_count;
                step(0, -OOB_STEPS);
                y_max += y_count;
                y_max = y_max / 2;

                // Construct step locations
                //x_step[0] = x_max/4;     y_step[0] = y_max/4;
                //x_step[1] = 3*x_max/4;   y_step[1] = y_max/4;
                //x_step[2] = x_max/4;     y_step[2] = 3*y_max/4;
                //x_step[3] = 3*x_max/4;   y_step[3] = 3*y_max/4;

                // Construct step locations
                x_step[0] = x_max/4;     y_step[0] = 3*y_max/8;
                x_step[1] = 3*x_max/4;   y_step[1] = 3*y_max/8;
                x_step[2] = x_max/4;     y_step[2] = 5*y_max/8;
                x_step[3] = 3*x_max/4;   y_step[3] = 5*y_max/8;

                // Move to locations and read coordinate data
                for (int i = 0; i < 4; i++) {
                    home();
                    step(x_step[i], y_step[i]);
                    uart_transmit(TX_Coordinate);
                    read_coordinate(&x_pixel[i], &y_pixel[i]);
                }

                // Calculate scalar & offset values
                x_scalar = (x_step[1] - x_step[0]) / (float)(x_pixel[1] - x_pixel[0])
                         + (x_step[3] - x_step[2]) / (float)(x_pixel[3] - x_pixel[2]);
                x_scalar /= 2;

                for (int i = 0; i < 4; i++) x_offset = x_step[i] - x_scalar * x_pixel[i];
                x_offset /= 4;

                y_scalar = (y_step[2] - y_step[0]) / (float)(y_pixel[2] - y_pixel[0])
                         + (y_step[3] - y_step[1]) / (float)(y_pixel[3] - y_pixel[1]);
                y_scalar /= 2;

                for (int i = 0; i < 4; i++) y_offset = y_step[i] - y_scalar * y_pixel[i];
                y_offset /= 4;

                // Return home
                home();

                // Move to roaming state
                state = 3;

                break;

            case 3:
                read_coordinate(&x, &y);
                home();
                pixel_step(x, y);
                break;

        }
    }

    return 0;
}
