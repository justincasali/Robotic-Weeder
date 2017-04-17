#include "fwa.h"
int dandTot;
int x_coordinates[10], y_coordinates[10];

int main(){
	dandTot = uart_receive();
	int i;
	for(i = 1; i <= dandTot; i++){
		read_coordinates(&x_coordinates(i),&y_coordinates(i));
	}
	
	for(i = 1; i <= dandTot; i++){
		uart_transmit(x_coordinates(i));
		uart_transmit(y_coordinates(i));
	}
}
