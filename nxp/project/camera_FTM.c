/*
 * Freescale Cup linescan camera code
 *
 *	This method of capturing data from the line
 *	scan cameras uses a flex timer module, periodic
 *	interrupt timer, an ADC, and some GPIOs.
 *	CLK and SI are driven with GPIO because the FTM2
 *	module used doesn't have any output pins on the
 * 	development board. The PIT timer is used to
 *  control the integration period. When it overflows
 * 	it enables interrupts from the FTM2 module and then
 *	the FTM2 and ADC are active for 128 clock cycles to
 *	generate the camera signals and read the camera
 *  output.
 *
 *	PTB8			- camera CLK
 *	PTB23 		- camera SI
 *  ADC0_DP1 	- camera AOut
 *
 * Author:  Alex Avery
 * Created:  11/20/15
 * Modified:  11/23/15
 */

#include "MK64F12.h"
#include "stdio.h"
#include "string.h"
#include "camera_FTM.h"

// Pixel counter for camera logic
// Starts at -2 so that the SI pulse occurs
//		ADC reads start
int pixcnt = -2;
// clkval toggles with each FTM interrupt
int clkval = 0;
// line stores the current array of camera data
int line[128];
int lineBuff[128];

// These variables are for streaming the camera
//	 data over UART
int debugcamdata = 0;
int capcnt = 0;
char str[100];

// ADC0VAL holds the current ADC value
uint16_t ADC0VAL;

void read_camera()
{
	memcpy(line, lineBuff, sizeof lineBuff);
}

/*�ADC0�Conversion�Complete�ISR� */
void ADC0_IRQHandler(void)
{
	// Reading ADC0_RA clears the conversion complete flag
	// Read the result (upper 12-bits). This also clears the Conversion complete flag.
	ADC0VAL = ADC0_RA;
}

/*
* FTM2 handles the camera driving logic
*	This ISR gets called once every integration period
*		by the periodic interrupt timer 0 (PIT0)
*	When it is triggered it gives the SI pulse,
*		toggles clk for 128 cycles, and stores the line
*		data from the ADC into the line variable
*/
void FTM2_IRQHandler(void)
{ //For FTM timer
	// Clear interrupt
	FTM2_SC &= ~FTM_SC_TOF_MASK;

	// Toggle clk
	GPIOB_PTOR |= (1 << 9);
	clkval = !clkval;

	// Line capture logic
	if ((pixcnt >= 2) && (pixcnt < 256))
	{
		if (!clkval)
		{ // check for falling edge
			// ADC read (note that integer division is
			//  occurring here for indexing the array)
			lineBuff[pixcnt / 2] = ADC0VAL;
		}
		pixcnt += 1;
	}
	else if (pixcnt < 2)
	{
		if (pixcnt == -1)
		{
			GPIOB_PSOR |= (1 << 23); // SI = 1
		}
		else if (pixcnt == 1)
		{
			GPIOB_PCOR |= (1 << 23); // SI = 0
			// ADC read
			lineBuff[0] = ADC0VAL;
		}
		pixcnt += 1;
	}
	else
	{
		GPIOB_PCOR |= (1 << 9); // CLK = 0
		clkval = 0;				// make sure clock variable = 0
		pixcnt = -2;			// reset counter
		// Disable FTM2 interrupts (until PIT0 overflows
		//   again and triggers another line capture)
		FTM2_SC &= ~(FTM_SC_TOIE_MASK);
	}
	return;
}

/* PIT0 determines the integration period
*		When it overflows, it triggers the clock logic from
*		FTM2. Note the requirement to set the MOD register
* 	to reset the FTM counter because the FTM counter is
*		always counting, I am just enabling/disabling FTM2
*		interrupts to control when the line capture occurs
*/
void PIT0_IRQHandler(void)
{
	if (debugcamdata)
	{
		// Increment capture counter so that we can only
		//	send line data once every ~2 seconds
		capcnt += 1;
	}
	// Clear interrupt
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

	// Setting mod resets the FTM counter
	FTM2_MOD = 100;

	// Enable FTM2 interrupts (camera)
	FTM2_SC |= FTM_SC_TOIE_MASK;

	return;
}

/* Initialization of FTM2 for camera */
void init_FTM2()
{
	// Enable clock
	SIM_SCGC6 |= SIM_SCGC6_FTM2_MASK;

	// Disable Write Protection
	FTM2_MODE |= FTM_MODE_WPDIS_MASK;

	// Set output to '1' on init
	FTM2_OUTINIT |= FTM_OUTINIT_CH0OI_MASK;

	// Initialize the CNT to 0 before writing to MOD
	FTM2_CNT = 0;

	// Set the Counter Initial Value to 0
	clkval = 0;
	FTM2_CNTIN = 0;

	// Set the period (~10us)
	FTM2_MOD = 200;

	// 50% duty
	FTM2_C0V = 100;

	// Set edge-aligned mode
	FTM2_C0SC |= FTM_CnSC_MSB_MASK;
	FTM2_C0SC |= FTM_CnSC_MSA_MASK;
	// Enable High-true pulses
	// ELSB = 1, ELSA = 0
	FTM2_C0SC |= FTM_CnSC_ELSB_MASK;
	FTM2_C0SC &= ~FTM_CnSC_ELSA_MASK;

	//�Enable�hardware�trigger�from�FTM2
	FTM2_EXTTRIG |= FTM_EXTTRIG_INITTRIGEN_MASK;

	// Don't enable interrupts yet (disable)
	FTM2_SC &= ~(FTM_SC_TOIE_MASK);

	// No prescalar, system clock
	FTM2_SC |= FTM_SC_PS(0) | FTM_SC_CLKS(1);

	// Set up interrupt
	NVIC_EnableIRQ(FTM2_IRQn);

	return;
}

/* Initialization of PIT timer to control
* 		integration period
*/
void init_PIT(void)
{
	// Setup periodic interrupt timer (PIT)
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	// Enable clock for timers
	PIT_MCR &= ~PIT_MCR_MDIS_MASK;

	// Enable timers to continue in debug mode
	PIT_MCR &= ~PIT_MCR_FRZ_MASK; // In case you need to debug

	// PIT clock frequency is the system clock
	// Load the value that the timer will count down from
	PIT_LDVAL0 = (INTEGRATION_TIME * DEFAULT_SYSTEM_CLOCK);

	// Enable timer interrupts
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;

	// Enable the timer
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;

	// Clear interrupt flag
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

	// Enable PIT interrupt in the interrupt controller
	NVIC_EnableIRQ(PIT0_IRQn);
	return;
}

/* Set up pins for GPIO
* 	PTB9 		- camera clk
*		PTB23		- camera SI
*		PTB22		- red LED
*/
void init_GPIO(void)
{
	// Enable LED and GPIO so we can see results
	LED_Init();
	//Enable
	PORTB_PCR9 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
	PORTB_PCR23 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
	//Output
	GPIOB_PDDR |= (1 << 9);
	GPIOB_PDDR |= (1 << 23);
	//Off
	GPIOB_PDOR |= (1 << 9);
	GPIOB_PDOR |= (1 << 23);

	return;
}

/* Set up ADC for capturing camera data */
void init_ADC0(void)
{
	unsigned int calib;
	// Turn on ADC0
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;

	// Single ended 16 bit conversion, no clock divider
	ADC0_CFG1 |= ADC_CFG1_ADIV(0) | ADC_CFG1_MODE(3);

	// Do ADC Calibration for Singled Ended ADC. Do not touch.
	ADC0_SC3 = ADC_SC3_CAL_MASK;
	while ((ADC0_SC3 & ADC_SC3_CAL_MASK) != 0)
		;
	calib = ADC0_CLP0;
	calib += ADC0_CLP1;
	calib += ADC0_CLP2;
	calib += ADC0_CLP3;
	calib += ADC0_CLP4;
	calib += ADC0_CLPS;
	calib = calib >> 1;
	calib |= 0x8000;
	ADC0_PG = calib;

	// Select hardware trigger.
	ADC0_SC2 |= ADC_SC2_ADTRG_MASK;

	// Set to single ended mode
	ADC0_SC1A = 0;
	ADC0_SC1A |= ADC_SC1_AIEN_MASK;
	ADC0_SC1A |= ADC_SC1_ADCH(0);
	ADC0_SC1A &= ~ADC_SC1_DIFF_MASK;

	// Set up FTM2 trigger on ADC0
	// FTM2 select
	SIM_SOPT7 |= SIM_SOPT7_ADC0TRGSEL(10);
	// Alternative trigger en.
	SIM_SOPT7 |= SIM_SOPT7_ADC0ALTTRGEN_MASK;
	// Pretrigger A
	SIM_SOPT7 &= ~SIM_SOPT7_ADC0PRETRGSEL_MASK;

	// Enable NVIC interrupt
	NVIC_EnableIRQ(ADC0_IRQn);
}
