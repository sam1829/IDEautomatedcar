/*
 * isr.c
 */

#include "isr.h"
#include "MK64F12.h"
#include <stdio.h>
#include "uart.h"

//variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
int buttonPressed = 0;
int milliCount = 0;

int timerActive = 0;

void PDB0_IRQHandler(void){ //For PDB timer
	
	// clear interupt in register PDB0_SC
	PDB0_SC &= ~PDB_SC_PDBIF_MASK;
	if(timerActive) {
		GPIOB_PTOR |= (1 << 22);
	} else {
		GPIOB_PSOR = (1UL << 22);
	}
	

	return;
}
	
void FTM0_IRQHandler(void){ //For FTM timer
	
	//clear interrupt in register FTM0_SC
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	//increment if button2 pressed
	if(buttonPressed){
		milliCount++;
	}
	
	return;
}
	
void PORTA_IRQHandler(void){ //For switch 3
	//Clear interrupt
	PORTA_PCR4 |= PORT_PCR_ISF_MASK; 

	/* the timer is enabled,  disable the timer;  else,
	enable the timer and start it with a trigger */
	if(timerActive == 1){
		timerActive = 0;
	} 
	else {
		PDB0_SC |= PDB_SC_SWTRIG_MASK;
		timerActive =1;

	}
	return;
}
	
void PORTC_IRQHandler(void){ //For switch 2
	
	//reset interrupt
	PORTC_PCR6 |= PORT_PCR_ISF_MASK;

	if(!buttonPressed){
		buttonPressed = 1;
		
		// reset flextimer
		milliCount = 0;
		
		// reset timer counter
		FTM0_CNT = FTM_CNT_COUNT(0);

		//turn on blue led
		GPIOB_PCOR |= (1 << 21); //blue
		
	} else {
		//reset variable
		buttonPressed = 0;
		
		//turn blue led off
		GPIOB_PSOR |= (1UL << 21);
		
		//output time held
		char timer[25];
		snprintf(timer, sizeof(timer), "%d", milliCount);
		put("Button held for ");
		put(timer);
		put(" milliseconds\n\r");
	}
	
	return;
}
