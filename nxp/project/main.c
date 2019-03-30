/*
 * Main code for driving car
 *
 * Author: Austin B & Susan M
 * Created: 3/30/2019
 * Modified:
 */

#include "MK64F12.h"
#include "uart.h"
#include "pwm.h"
#include "filter.c"

#define CAMERA_LENGTH 128
void initialize(void);
void delay(int del);

int capcnt = 0;

int main(void)
{
	// Initialize everything
	initialize();
	uint16_t * line;

	// Print welcome over serial
	for(;;){
    line = read_camera();
		uint16_t output[128];
		float conv = {1.0, 1.0, 1.0};
		convolve(line, output, CAMERA_LENGTH, &conv, 3);

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

	// Initialize the FlexTimer for motors
	InitPWM0();
	InitPWM3();

  //camera
  init_GPIO(); // For CLK and SI output on GPIO
	init_FTM2(); // To generate CLK, SI, and trigger ADC
	init_ADC0();
	init_PIT(); // To trigger camera read based on integration time
}
