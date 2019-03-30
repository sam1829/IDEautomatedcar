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

void initialize(void);
void delay(int del);

int capcnt = 0;

int main(void)
{
	// Initialize everything
	initialize();

	// Print welcome over serial
	for(;;){
    read_camera()

	}
}

// should uart_init type be a float* as well??
void convolve(uart_init input, float* output, int length, float* h, int h_length){
	for (int i = 0; i <= length - h_length; i++){
		output[i] = 0.0;
		for (int j = 0; j <= h_length; j++){
			output[i] += input[i + j] * h[h_length - j - 1];
		}
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
