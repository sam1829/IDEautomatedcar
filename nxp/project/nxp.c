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

#define HARD_RIGHT 8.5
#define CENTER 6.75
#define HARD_LEFT 4.75

#define CENTER_MAX 70
#define CENTER_MIN 58

#define min(x,y) (x<y) ?1:0
#define max(x,y) (x>y) ?1:0

void initialize(void);
void delay(int del);
void convolve(int *input, int *output, int length, int *h, int h_length);
int * findMinMax(int *input, int length, int *output);
void debugCamera(void);
void turn(int center);

int main(void)
{
	// Initialize everything
	initialize();

	// Print welcome over serial
	uart_putchar3('t');
	//int i = 3; i>0; i--
	
	SetDutyCycle0(50, 10e3, 0);
	for (;;)
	{
		
		//read camera
		read_camera();
		/* if(0){
			debugCamera();
		}*/
		
		//filter output
		int output[128];
		int conv[3] = {1, 1, 1};
		convolve(line, output, 128, conv, 3);
		int conv2[3] = {2, 4, 2};
		convolve(output, line, 128, conv2, 3);
		int conv3[5] = {1, 2, 4, 2, 1};
		convolve(line, output, 128, conv3, 3);
		int conv4[5] = {-1, 0 ,1};
		convolve(output, line, 128, conv4, 3);
		memcpy(output, line, sizeof output);
		/*if(0){
			put0("After Filter:\n\r");
			debugCamera();
		}*/
		//min and max
		int min_max[2]={0,0};
		findMinMax(output, 128, min_max);
		int bandwith = min_max[1]-min_max[0];
		int center = (bandwith/2)+min_max[0];
		/*
			char str[100];
			sprintf(str, "min:%i\n\r", min_max[0]);
			put0(str);
			sprintf(str, "max:%i\n\r", min_max[1]);
			put0(str);
		sprintf(str, "center:%i\n\r", center);
			put0(str);
		//delay(100);
		*/
		
		//turn(center);
		if(center > 64+6){
			SetDutyCycle3(HARD_RIGHT, 50);
		}
		else if (center < 64-6){
			SetDutyCycle3(HARD_LEFT, 50);
		} else {
			SetDutyCycle3(CENTER, 50);
		}
	}
}

void turn(int center){
	float turn_val = CENTER;
	if(center > CENTER_MAX){
			SetDutyCycle3(HARD_RIGHT, 50);
		}
		else if (center < CENTER_MIN){
			SetDutyCycle3(HARD_LEFT, 50);
		} else {
			float transform = (HARD_RIGHT - HARD_LEFT)*(CENTER_MAX-CENTER_MIN);
			turn_val = ((center - (float) CENTER_MIN) * transform) + (float) HARD_LEFT;
		}
	SetDutyCycle3(turn_val, 50);
	
}

void debugCamera() {
	char str[100];
	// send the array over uart
		for (int i = 0; i < 128; i++)
		{
			sprintf(str, "%i\n\r", line[i]);
			put0(str);
		}
	
}

int * findMinMax(int *input, int length, int *output)
{
	for (int i = 0; i < length; i++) {
		if(min(input[i], input[output[0]])){
			output[0]= i;
		}
		if(max(input[i], input[output[1]])){
			output[1]=i;
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

