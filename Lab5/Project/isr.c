/*
 * isr.c
 */

#include "isr.h"
#include "MK64F12.h"
#include <stdio.h>
#include "uart.h"

//variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
int buttonPress = 0;
int buttonCount = 0;

void PDB0_IRQHandler(void){ //For PDB timer
	
	// clear interupt in register PDB0_SC
	PDB0_SC &= ~PDB_SC_PDBIF_MASK;

	// toggle output state for LED1
	GPIOB_PTOR |= (1 << 22);
	GPIOB_PTOR |= (1 << 21);

	return;
}
	
void FTM0_IRQHandler(void){ //For FTM timer
	
	//clear interrupt in register FTM0_SC
	FTM0_SC = ~FTM_SC_TOF_MASK;

	//increment if button2 pressed
	if(buttonPress){
		buttonCount++;
	}
	
	return;
}
	
void PORTA_IRQHandler(void){ //For switch 3
	//Clear interrupt

	/* the timer is enabled,  disable the timer;  else,
	enable the timer and start it with a trigger */
	if( ){

	}else {
		
	}
	return;
}
	
void PORTC_IRQHandler(void){ //For switch 2
	
	
	
	return;
}
