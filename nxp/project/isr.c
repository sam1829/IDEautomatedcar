/*
 * isr.c
 */

#include "isr.h"
#include "MK64F12.h"
#include <stdio.h>
#include "uart.h"
#include "nxp.h"

//variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
int buttonPressed = 0;
int milliCount = 0;

int timerActive = 0;

void PDB0_IRQHandler(void)
{ //For PDB timer

	// clear interupt in register PDB0_SC
	PDB0_SC &= ~PDB_SC_PDBIF_MASK;
	if (timerActive)
	{
		GPIOB_PTOR |= (1 << 22);
	}
	else
	{
		GPIOB_PSOR = (1UL << 22);
	}

	return;
}

void FTM0_IRQHandler(void)
{ //For FTM timer

	//clear interrupt in register FTM0_SC
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	//increment if button2 pressed
	if (buttonPressed)
	{
		milliCount++;
	}

	return;
}

void PORTA_IRQHandler(void)
{ //For switch 3
	//Clear interrupt
	PORTA_PCR4 |= PORT_PCR_ISF_MASK;

	mode++;
	if (mode > 2)
	{
		mode = 0;
	}

	switch (mode)
	{
	case 0: //Fast
		// Red LED
		GPIOB_PSOR = (1UL << 21);
		GPIOB_PCOR = (1UL << 22);
		GPIOE_PSOR = (1UL << 26);
		max_speed = MAX_SPEED0;
		min_speed = MIN_SPEED0;
		break;

	case 1: //normal
		// Yellow LED
		GPIOB_PSOR = (1UL << 21);
		GPIOB_PCOR = (1UL << 22);
		GPIOE_PCOR = (1UL << 26);
		max_speed = MAX_SPEED1;
		min_speed = MIN_SPEED1;
		break;

	case 2: //slow
		// Green LED
		GPIOB_PSOR = (1UL << 21);
		GPIOB_PSOR = (1UL << 22);
		GPIOE_PCOR = (1UL << 26);
		max_speed = MAX_SPEED2;
		min_speed = MIN_SPEED2;
		break;

	default:
		mode = 0;
	}

	return;
}

void PORTC_IRQHandler(void)
{ //For switch 2

	//reset interrupt
	PORTC_PCR6 |= PORT_PCR_ISF_MASK;

	if (!buttonPressed)
	{
		buttonPressed = 1;

		// reset flextimer
		milliCount = 0;

		// reset timer counter
		FTM0_CNT = FTM_CNT_COUNT(0);

		//turn on blue led
		GPIOB_PCOR |= (1 << 21); //blue
	}
	else
	{
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
