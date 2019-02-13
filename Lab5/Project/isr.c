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
	PORTA_PCR4 |= PORT_PCR_ISF_MASK; 

	/* the timer is enabled,  disable the timer;  else,
	enable the timer and start it with a trigger */
	if(PDB0_SC & PDB_SC_PDBEN_MASK ){
		PDB0_SC &= ~PDB_SC_PDBEN_MASK;
	} 
	else {
		PDB0_SC |= PDB_SC_PDBEN_MASK | PDB_SC_SWTRIG_MASK;

	}
	return;
}
	
void PORTC_IRQHandler(void){ //For switch 2
	
	//reset interrupt
	FTM0_SC &= ~FTM_SC_TOF_MASK;

	if(GPIOC_PDIR & (1<<6) == 0){
		buttonPress = 1;
		// reset flextimer
		//TODO: add
		// reset timer counter
		FTM0_CNT = FTM_CNT_COUNT(0);

		//turn on blue led
		GPIOB_PDDR |= (1 << 21); //blue
		GPIOB_PSOR |= (1UL << 22); //red
	} else {
		//reset variable
		buttonPress = 0;
		//turn blue led off
		GPIOB_PSOR |= (1UL << 21);
		//output time held
		put("Button held for xx milliseconds"); //TODO: add xx
	}
	
	return;
}
