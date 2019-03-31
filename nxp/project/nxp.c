/*
 * Main code for driving car
 *
 * Author: Austin B & Susan M
 * Created: 3/30/2019
 * Modified:
 */

#include "MK64F12.h"
#include "stdio.h"
#include "uart.h"
#include "uart3.h"
#include "pwm.h"
#include "camera_FTM.h"

void initialize(void);
void delay(int del);
void convolve(uint16_t *input, double *output, int length, double *h, int h_length);

int main(void)
{
	// Initialize everything
	initialize();
	uint16_t *line;

	char str[100];
	int i;
	// Print welcome over serial
	for (;;)
	{
		line = read_camera();
		double output[128];
		double conv[3] = {1.0, 1.0, 1.0};
		convolve(line, output, 128, conv, 3);
		//GPIOB_PCOR |= (1 << 22);
		// send the array over uart
		sprintf(str, "%i\n\r", -1); // start value
		uart3_put(str);
		for (i = 0; i < 127; i++)
		{
			sprintf(str, "%i\n", line[i]);
			uart3_put(str);
		}
		sprintf(str, "%i\n\r", -2); // end value
		uart3_put(str);
		//GPIOB_PSOR |= (1 << 22);
	}
}

void convolve(uint16_t *input, double *output, int length, double *h, int h_length)
{
	for (int i = 0; i <= length - h_length; i++)
	{
		output[i] = 0.0;
		for (int j = 0; j < h_length; j++)
		{
			output[i] += input[i + j] * h[h_length - j - 1];
		}
	}
}

/**
 * Waits for a delay (in milliseconds)
 *
 * del - The delay in milliseconds
 */
void delay(int del)
{
	int i;
	for (i = 0; i < del * 50000; i++)
	{
		// Do nothing
	}
}

void initialize()
{
	// Initialize UART
	uart0_init();
	uart3_init();

	// Initialize the FlexTimer for motors
	InitPWM0();
	InitPWM3();
	/*
  //camera
  init_GPIO(); // For CLK and SI output on GPIO
	init_FTM2(); // To generate CLK, SI, and trigger ADC
	init_ADC0();
	init_PIT(); // To trigger camera read based on integration time
	*/
}

