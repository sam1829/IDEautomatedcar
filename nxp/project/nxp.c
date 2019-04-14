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

#define HARD_RIGHT 8.5
#define CENTER 6.75
#define HARD_LEFT 4.75

#define TRUE_CENTER 60
#define CENTER_RANGE 6
#define CENTER_MAX (TRUE_CENTER + CENTER_RANGE)
#define CENTER_MIN (TRUE_CENTER - CENTER_RANGE)

#define min(x, y) (x < y) ? 1 : 0
#define max(x, y) (x > y) ? 1 : 0

#define Kp 0.1
#define Kp_speed 0.4

#define MAX_SPEED 75
#define MIN_SPEED 50

void initialize(void);
void delay(int del);
void convolve(int *input, int *output, int length, int *h, int h_length);
int *findMinMax(int *input, int length, int *output);
void debugCamera(void);
void turn(int center);
float clip(float input, float min, float max);

int main(void)
{
	// Initialize everything
	initialize();

	// Print welcome over serial
	uart_putchar3('t');
	//int i = 3; i>=0; i--
	int i = 3;
	SetDutyCycle0(MAX_SPEED, 10e3, 0);
	for (;;)
	{

		//read camera
		read_camera();
		if (i == 0)
		{
			debugCamera();
		}

		//filter output
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

		/*char str[100];
			sprintf(str, "min:%i\n\r", min_max[0]);
			put0(str);
			sprintf(str, "max:%i\n\r", min_max[1]);
			put0(str);
			sprintf(str, "center:%i\n\r", center);
			put0(str);
		  //delay(100); */

		//PID
		float err = center - TRUE_CENTER;
		float turn = (float) CENTER + (float) Kp * (err);
		turn = clip(turn, HARD_LEFT, HARD_RIGHT);
		SetDutyCycle3(turn, 50);
		
		//speed
		float speed = (float) MAX_SPEED - (float) Kp_speed * fabsf(turn - (float) CENTER);
		speed = clip(speed, MIN_SPEED,MAX_SPEED);
		SetDutyCycle0(MAX_SPEED, 10e3, 0);
	}
}

float clip(float input, float min, float max)
{
	if (input >  max)
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

void turn_simple(int center)
{
	if (center > CENTER_MAX)
	{
		SetDutyCycle3(HARD_RIGHT, 50);
	}
	else if (center < CENTER_MIN)
	{
		SetDutyCycle3(HARD_LEFT, 50);
	}
	else
	{
		SetDutyCycle3(CENTER, 50);
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
}
