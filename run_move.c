#define PULSE_TIME 1000
#include "fwa.h"

#define DANCE 125

#define RX_Calibration   0x01
#define RX_Halt          0x02
#define RX_Recalibration 0x03
#define RX_Pseudocation  0x04
#define RX_Num_Ready     0x05
#define RX_Coord_Ready   0x06

#define TX_Coordinate    0x01
#define TX_Roaming       0x02
#define TX_Location      0x03
#define TX_Shutdown      0x04
#define TX_Positioning   0x05
#define TX_Confirmation  0x06

int main() {

    pin_init();
    uart_init();
    set_nav();

    char state = 0;
    char rx = 0x00;
    int x, y;

    char startup = 1;
    char recal = 1;
    char pseudo = 1;

    // state 2 variables
    int x_max, y_max;
    int x_step[4], y_step[4];
    int x_pixel[4], y_pixel[4];
    float x1, x2;
    float y1, y2;

    // state 4 variables
    int dand_num, dand_tot;
    int x_coordinates[0x10], y_coordinates[0x10];

    while (1) {
        set_led(state);
        rx = 0;

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
                home();
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
                //x_step[2] = 3*x_max/4;   y_step[3] = 3*y_max/4;
                //x_step[3] = x_max/4;     y_step[2] = 3*y_max/4;

                // Construct step locations
                x_step[0] = x_max/4;     y_step[0] = 3*y_max/8;
                x_step[1] = 3*x_max/4;   y_step[1] = 3*y_max/8;
                x_step[2] = 3*x_max/4;   y_step[3] = 5*y_max/8;
                x_step[3] = x_max/4;     y_step[2] = 5*y_max/8;

                // Send constructed steps
                //for (int i = 0; i < 4; i++) send_coordinate(x_step[i], y_step[i]);

                // Move to locations and read coordinate data
                for (int i = 0; i < 4; i++) {
                    move(x_step[i], y_step[i]);
                    uart_transmit(TX_Coordinate);
                    read_coordinate(&x_pixel[i], &y_pixel[i]);
                }

                // Calculate scalar & offset values
                x1 = (x_pixel[0] + x_pixel[3]) / 2.0;
                x2 = (x_pixel[1] + x_pixel[2]) / 2.0;

                x_scalar = (x_step[1] - x_step[0]) / (x2 - x1);
                x_offset = x_step[0] - x_scalar * x1;

                y1 = (y_pixel[0] + y_pixel[1]) / 2.0;
                y2 = (y_pixel[2] + y_pixel[3]) / 2.0;

                y_scalar = (y_step[2] - y_step[1]) / (y2 - y1);
                y_offset = y_step[0] - y_scalar * y1;

                // Return home
                home();

                // Move to roaming state / positioning state
                state = startup ? 3 : 5;
                startup = 0;
                break;

            case 3:
                // Clear Nav stop signal
                clear_nav();
                // Wait for dandelion detection signal from TK1
                while (rx != RX_Halt) rx = uart_receive();
                // Send Nav stop signal
                set_nav();
                // Delay
                _delay_ms(1000);
                // Tell TK1 that navigation has stopped
                uart_transmit(TX_Location);
                // Move to location state
                state = 4;
                break;

            case 4:
                // Receive dandelion coordinates
                while (rx != RX_Num_Ready) rx = uart_receive();
                dand_tot = uart_receive();

                // Check to make sure that dandelions were actually detected
                if (dand_tot == 0) {
                    state = 3;
                    break;
                }

                // Listen for dandelion coordinates
                for (int i = 0; i < dand_tot; i++) {
                    while (rx != RX_Coord_Ready) rx = uart_receive();
                    rx = 0;
                    read_coordinate(&x_coordinates[i], &y_coordinates[i]);
                }

                // Transition to positioning state once coordinates are received
                dand_num = 0;
                state = 5;
                break;

            case 5:
                // Position pseudocator
                pixel_move(x_coordinates[dand_num], y_coordinates[dand_num]);

                // Transition to verification state
                state = 6;
                break;

            case 6:
                // Request confirmation from TK1
                uart_transmit(TX_Confirmation);

                // Wait for confirmation/denial
                while (rx != RX_Recalibration && rx != RX_Pseudocation) rx = uart_receive();

                // Recalibrate if position is denied
        		if (rx == RX_Recalibration) {
                    state = (recal) ? 2 : 7;
                    pseudo = (recal) ? 1 : 0;
                    recal = (recal) ? 0 : 1;
                }

        		// Continue to pseudocation if location is correct
        		if (rx == RX_Pseudocation) {
                    state = 7;
                    pseudo = 1;
                }

                break;

            case 7:
                // Pseudocation
                if (pseudo) pseudocate();

                // If no dandelions left, transition to roaming state
                dand_num++;
                if (dand_num >= dand_tot) {
                    home();
                    uart_transmit(TX_Roaming);
                    state = 3;
                    break;
                }

                // If dandelions remain, transition to positioning state
                state = 5;
                break;

        }
    }

    return 0;
}
