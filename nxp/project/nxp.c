/*
 * Main code for driving car
 *
 * Author: Austin B & Susan M
 * Created: 3/30/2019
 * Modified:
 */

#include "MK64F12.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "uart3.h"
#include "pwm.h"
#include "camera_FTM.h"
#include "math.h"
#include "nxp.h"
#include "isr.h"

int max_speed = MAX_SPEED0;
int min_speed = MIN_SPEED0;
int mode = 0;

int iteration = 0;

int main(void)
{
	// Initialize everything
	initialize();
	delay(100);
	//int i = 3; i>=0; i--
	int i = 3;
	SetDutyCycle0(max_speed, 10e3, 0);
	for (;;)
	{
		iteration++;
		//read camera
		read_camera();
		if (i == 0)
		{
			debugCamera();
		}
		
		//filter input
		int output[128];
		int conv[3] = {1, 1, 1};
		convolve(line, output, 128, conv, 3);
		int conv2[3] = {2, 4, 2};
		convolve(output, line, 128, conv2, 3);
		int conv3[5] = {1, 2, 4, 2, 1};
		convolve(line, output, 128, conv3, 3);
		int conv4[5] = {-1, 0, 1};
		convolve(output, line, 128, conv4, 3);
		memcpy(output, line, sizeof output);

		if (i == 0)
		{
			put0("After Filter:\n\r");
			debugCamera();
		}

		//min and max
		int min_max[2] = {0, 0};
		findMinMax(output, 120, min_max);
		int bandwith = min_max[1] - min_max[0];
		int center = (bandwith / 2) + min_max[0];

		//line check
		if (iteration > 10 && countPeaks(output, 120, output[min_max[1]], output[min_max[0]]) > 5 )
		{
			SetDutyCycle0(0, 10e3, 0);
			return 1;
		}

		//PID
		float err = center - TRUE_CENTER;
		float turn = (float)CENTER + (float)Kp * (err);
		turn = clip(turn, HARD_LEFT, HARD_RIGHT);
		SetDutyCycle3(turn, 50);

		//speed
		float speed = (float)max_speed - (float)Kp_speed * fabsf(turn - (float)CENTER);
		speed = clip(speed, min_speed, max_speed);
		SetDutyCycle0(max_speed, 10e3, 0);
	}
}

int countPeaks(int *input, int length, int max, int min)
{
	int count = 0;
	int lower_bound_min = (float) min * (float) 1.2;
	int lower_bound_max = (float) max * (float) 0.8;
	int upper_bound_min = (float) min * (float) 0.8;
	int upper_bound_max = (float) max * (float) 1.2;
	
	for (int i = 0; i < length; i++)
	{
		if(input[i] < upper_bound_min && input[i] > lower_bound_min)
		{
			count++;
		} else if (input[i] < upper_bound_max && input[i] > lower_bound_max)
		{
			count++;
		}
	}
	return count;
}

float clip(float input, float min, float max)
{
	if (input > max)
	{
		return max;
	}
	else if (input < min)
	{
		return min;
	}
	else
	{
		return input;
	}
}

void turn_linear(int center)
{
	float turn_val;
	if (center >= CENTER_MAX)
	{
		turn_val = HARD_RIGHT;
	}
	else if (center <= CENTER_MIN)
	{
		turn_val = HARD_LEFT;
	}
	else
	{
		float old_range = CENTER_MAX - CENTER_MIN;
		float new_range = HARD_RIGHT - HARD_LEFT;
		turn_val = ((center - (float)CENTER_MIN) / old_range) * new_range + (float)HARD_LEFT;
	}
	SetDutyCycle3(turn_val, 50);
}

void debugCamera()
{
	char str[100];
	// send the array over uart
	for (int i = 0; i < 128; i++)
	{
		sprintf(str, "%i\n\r", line[i]);
		put0(str);
	}
}
//0 is min, 1 is max
int *findMinMax(int *input, int length, int *output)
{
	for (int i = 0; i < length; i++)
	{
		if (min(input[i], input[output[0]]))
		{
			output[0] = i;
		}
		if (max(input[i], input[output[1]]))
		{
			output[1] = i;
		}
	}
	return output;
}

void convolve(int *input, int *output, int length, int *h, int h_length)
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
	uart_init3();

	// Initialize the FlexTimer for motors
	InitPWM0();
	InitPWM3();

	//camera
	init_GPIO(); // For CLK and SI output on GPIO
	init_FTM2(); // To generate CLK, SI, and trigger ADC
	init_ADC0();
	init_PIT(); // To trigger camera read based on integration time

	//Button and LED
	LED_Init();
	Button_Init();
	NVIC_EnableIRQ(PORTA_IRQn);
	//NVIC_EnableIRQ(PORTC_IRQn);
	PORTA_PCR4 |= PORT_PCR_IRQC(0x9);

	//Init mode
	mode = 0;
	// Red LED
	GPIOB_PSOR = (1UL << 21);
	GPIOB_PCOR = (1UL << 22);
	GPIOE_PSOR = (1UL << 26);
	max_speed = MAX_SPEED0;
	min_speed = MIN_SPEED0;
}

/*  

    Port names
				Port    Pin     Name 
	Red LED     B       22      PTB22
	Green LED   E       26      PTE26
	Blue LED    B       21      PTB21
	Switch 2    C       6       PTC6

*/

void LED_Init(void)
{
	// Enable clocks on Ports B and E for LED timing
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

	// Configure the Signal Multiplexer for GPIO
	PORTB_PCR21 |= PORT_PCR_MUX(1);
	PORTB_PCR22 |= PORT_PCR_MUX(1);
	PORTE_PCR26 |= PORT_PCR_MUX(1);

	// Switch the GPIO pins to output mode
	GPIOB_PDDR |= (1 << 22);
	GPIOE_PDDR |= (1 << 26);
	GPIOB_PDDR |= (1 << 21);

	// Turn off the LEDs
	GPIOB_PSOR = (1UL << 21) | (1UL << 22);
	GPIOE_PSOR = 1UL << 26;
}

void Button_Init(void)
{
	// Enable clock for Port C PTC6 and PTA4 button
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

	// Configure the Mux for the button
	PORTC_PCR6 = PORT_PCR_MUX(1);
	PORTA_PCR4 = PORT_PCR_MUX(1);

	// Set the push button as an input
	GPIOC_PDDR = (0 << 6);
	GPIOA_PDDR = (0 << 4);
}
