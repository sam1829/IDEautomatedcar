/*
 * Main Method for testing the PWM Code for the K64F
 * PWM signal can be connected to output pins are PC3 and PC4
 *
 * Author:
 * Created:
 * Modified:
 */

#include "MK64F12.h"
#include "uart.h"
#include "pwm.h"

void initialize(void);
void delay(int del);

int main(void)
{
	// Initialize UART and PWM
	initialize();

	// Print welcome over serial
	put("Running... \n\r");

	while(1){
    //DC Motor
		SetDutyCycle0(50, 10e3, 0);
    //Servo motor
    SetDutyCycle3(20, 50);
	}
}


/**
 * Waits for a delay (in milliseconds)
 *
 * del - The delay in milliseconds
 */
void delay(int del){
	int i;
	for (i=0; i<del*50000; i++){
		// Do nothing
	}
}

void initialize()
{
	// Initialize UART
	uart_init();

	// Initialize the FlexTimer
	InitPWM0();
	InitPWM3();
}
