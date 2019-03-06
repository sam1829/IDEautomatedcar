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

void initialize();
void en_interrupts();
void delay();

int main(void)
{
	// Initialize UART and PWM
	initialize();

	// Print welcome over serial
	put("Running... \n\r");
	
	int forward = 1;
	int phase = 0;
	while(1){
		// Turn  off  all coils , Set  GPIO  pins to 0
		//Set  one  pin  high at a time
		if(forward ){
			if(phase == 0){/*turn on coil A*/; phase ++;} //A, 1a
			else if(phase  == 1){ /*turn on coil B*/; phase ++;} //B,2a
			else if (phase  == 2) { /*turn on coil C*/; phase ++;} //C,1b
			else { /*turn on coil D*/; phase =0;} //D,2b
		}else    {// reverse
			if (phase == 0) { /*turn on coil D*/; phase ++;} //D,2b
			else if (phase  == 1) { /*turn on coil C*/; phase ++;} //C,1b
			else if (phase  == 2) { /*turn on coil B*/; phase ++;} //B,2a
			else { /*turn on coil A*/; phase =0;} //A,1a
		}
	//Note - you  need to  write  your  own  delay  function
		delay (10);   // smaller  values=faster  speed}
	
	//Step 3
	//Generate 20% duty cycle at 10kHz
	//for(;;){  //then loop forever
	//	SetDutyCycle(70, 10e3, 0);
	//}
	//Step 9
	for(;;)  //loop forever
	{
		uint16_t dc = 0;
		uint16_t freq = 10000; /* Frequency = 10 kHz */
		uint16_t dir = 0;
		char c = 48;
		int i=0;
		
		// 0 to 100% duty cycle in forward direction
		for (i=0; i<100; i++){
			SetDutyCycle(i, 10e3, 0);
			delay(10);
		}
		
		// 100% down to 0% duty cycle in the forward direction
		for (i=100; i>=0; i--){
			SetDutyCycle(i, 10e3, 0);
			delay(10);
		}
		
		// 0 to 100% duty cycle in reverse direction
		for (i=0; i<100; i++){
			SetDutyCycle(i, 10e3, 1);
			delay(10);
		}
		
		// 100% down to 0% duty cycle in the reverse direction
		for (i=100; i>=0; i--){
			SetDutyCycle(i, 10e3, 1);
			delay(10);
		}

	}
	return 0;
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
	InitPWM();
}
