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

void put3(char *ptr_str);
void uart_init3(void);
uint8_t uart_getchar3(void);
void uart_putchar3(char ch);

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
		uint16_t output[128];
		float conv = {1.0, 1.0, 1.0};
		convolve(line, output, CAMERA_LENGTH, &conv, 3);
		//GPIOB_PCOR |= (1 << 22);
		// send the array over uart
		sprintf(str, "%i\n\r", -1); // start value
		put(str);
		for (i = 0; i < 127; i++)
		{
			sprintf(str, "%i\n", line[i]);
			put(str);
		}
		sprintf(str, "%i\n\r", -2); // end value
		put(str);
		//GPIOB_PSOR |= (1 << 22);
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
	uart_init();
	uart_init3();

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

void put3(char *ptr_str)
{
	while (*ptr_str)
		uart_putchar3(*ptr_str++);
}
