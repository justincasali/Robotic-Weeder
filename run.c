#define PULSE_TIME 1000
#include "fwa.h"

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

    // state 4 variables
    int dandNum, dandTot;
    int x_coordinates[10], y_coordinates[10];

    while (1) {
        set_led(state);
        switch (state) {

            case 1:
		home();
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

		// Light LED Red

                // Move to locations, read coordinate data, & calculate scalar values
                for (int i = 0; i < 4; i++) {

                    home();
                    step(x_step[i], y_step[i]);
		    // Delay?

                    uart_transmit(TX_Coordinate);
                    // TODO LOOK FOR COORDINATE DATA CONTROL BYTE
                    read_coordinate(&x_pixel[i], &y_pixel[i]);

                    x_scalar += x_step[i] / (float)(x_pixel[i]);
                    y_scalar += y_step[i] / (float)(y_pixel[i]);

                }

                x_scalar /= 4;
                y_scalar /= 4;

                home();

		// Turn off LED

                // Move to roaming state
                state = 3;

                break;

            case 3:
		// Wait for dandelion detection signal from TK1
		while(rx != RX_Halt) rx = uart_receive();
		// Send Nav stop signal

		// Delay?
		// Tell TK1 that navigation has stopped
		uart_transmit(TX_Location);
		// Move to location state
		state = 4;
                break;
		
	    case 4:
		// Receive dandelion coordinates
		while(rx != numReady) rx = uart_receive();
		dandTot = uart_receive();
		// Check to make sure that dandelions were actually detected
		if(dandTot == 0)
			state == 3;
			break;
		}
		// Listen for dandelion coordinates
		for(i = dandNum; i > 0; i = i-1){
			while(rx != coordReady) rx = uart_receive();
			read_coordinate(&x_coordinates[i],&y_coordinates[i]);
		}
		// Transition to positioning state once coordinates are received
		dandNum = 1;
		state == 5;
		break;

	    case 5:
		// Position pseudocator to each coordinate
		home();
		// Translate coordinates to steps

		// Position pseudocator
		step(x_coordinates[dandNum], y_coordinates[dandNum]);
		// Transition to verification state
		state = 6;
		break;
        
            case 6:
		// Light LED red
		
		// Request confirmation from TK1
		uart_transmit(TX_Coordinate);

		// Wait for confirmation/denial
		while(rx != RX_Recalibration || rx != RX_Pseudocation) rx = uart_receive();
		// Recalibrate if position is denied
		if(rx == RX_Recalibration){
			state = 8;
			break;
		}
		// Continue to pseudocation if location is correct
		if(rx == RX_Pseudocation){
			state = 7;
			break;
		}

	    case 7:
		// Pseudocate dandelion
		// Turn off red LED
		//delay
		// Light Green LED
		// If no dandelions left, transition to roaming state
		dandNum++;
		if(dandNum > dandTot){
			uart_transmit(TX_Roaming);
			state = 3;
			break;
		}
		// If dandelions remain, transition to positioning state
		uart_transmit(TX_Positioning);
		state = 5;
		break;

	    case 8:
		// Recalibration State

		uart_transmit(TX_Positioning);
		state = 5;
		break;
        }
    }

    return 0;
}

