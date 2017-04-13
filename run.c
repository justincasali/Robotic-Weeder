#define PULSE_TIME 1000
#include "fwa.h"

// These values need to be reworked
#define RX_Calibration   0x01
#define RX_Halt          0x02
#define RX_Recalibration 0x03
#define RX_Pseudocation  0x04

#define TX_Coordinate    0x01
#define TX_Roaming       0x02
#define TX_Location      0x03
#define TX_Shutdown      0x04
#define TX_Positioning   0x05

int main() {

    pin_init();
    uart_init();

    char state = 1;
    char rx = 0x00;
    int x, y;

    // state 2 variables
    int x_max, y_max;
    int x_step[4], y_step[4];
    int x_pixel[4], y_pixel[4];

    while (1) {
        set_led(state);
        switch (state) {

            case 1:
                while (rx != RX_Calibration) rx = uart_receive();
                state = 2;
                break;

            case 2:

                // Clear scalar values
                x_scalar = 0;
                y_scalar = 0;

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
                x_step[0] = x_max/4;     y_step[0] = y_max/4;
                x_step[1] = 3*x_max/4;   y_step[1] = y_max/4;
                x_step[2] = x_max/4;     y_step[2] = 3*y_max/4;
                x_step[3] = 3*x_max/4;   y_step[3] = 3*y_max/4;

                // Move to locations, read coordinate data, & calculate scalar values
                for (int i = 0; i < 4; i++) {

                    home();
                    step(x_step[i], y_step[i]);

                    uart_transmit(TX_Coordinate);
                    // TODO LOOK FOR COORDINATE DATA CONTROL BYTE
                    read_coordinate(&x_pixel[i], &y_pixel[i]);

                    x_scalar += x_step[i] / (float)(x_pixel[i]);
                    y_scalar += y_step[i] / (float)(y_pixel[i]);

                }

                x_scalar /= 4;
                y_scalar /= 4;

                home();

                // Move to roaming state
                state = 3;

                break;

            case 3:
                break;

        }
    }

    return 0;
}
