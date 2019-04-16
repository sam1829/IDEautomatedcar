/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* Spring 2016
*
* Filename: main_A2D_template.c
*/

#include "uart.h"
#include "MK64F12.h"
#include "stdio.h"
#define BUFFER_LENGTH 5
#define BUFFER_INDEX (BUFFER_LENGTH - 1)
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */
int milliCount = 0;

void PDB_INIT(void)
{
    //Enable PDB Clock
    SIM_SCGC6 |= SIM_SCGC6_PDB_MASK;
    //PDB0_CNT = 0x0000;
    PDB0_MOD = 50000; // 50,000,000 / 50,000 = 1000

    PDB0_SC = PDB_SC_PDBEN_MASK | PDB_SC_CONT_MASK | PDB_SC_TRGSEL(0xf) | PDB_SC_LDOK_MASK;
    PDB0_CH1C1 = PDB_C1_EN(0x01) | PDB_C1_TOS(0x01);
}

void ADC1_INIT(void)
{
    unsigned int calib;

    // Turn on ADC1 clock
    SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;

    // Configure CFG Registers
    // Configure ADC to divide 50 MHz down to 6.25 MHz AD Clock, 16-bit single ended
    ADC1_CFG1 = ADC_CFG1_ADIV(3) | ADC_CFG1_MODE(3);

    // Do ADC Calibration for Singled Ended ADC. Do not touch.
    ADC1_SC3 = ADC_SC3_CAL_MASK;
    while ((ADC1_SC3 & ADC_SC3_CAL_MASK) != 0)
        ;
    calib = ADC1_CLP0;
    calib += ADC1_CLP1;
    calib += ADC1_CLP2;
    calib += ADC1_CLP3;
    calib += ADC1_CLP4;
    calib += ADC1_CLPS;
    calib = calib >> 1;
    calib |= 0x8000;
    ADC1_PG = calib;

    // Configure SC registers.
    // Select hardware trigger.
    ADC1_SC2 |= ADC_SC2_ADTRG_MASK;

    // Configure SC1A register.
    // Select ADC Channel and enable interrupts. Use ADC1 channel DADP3  in single ended mode.
    ADC1_SC1A |= ADC_SC1_AIEN_MASK;
    ADC1_SC1A &= ~ADC_SC1_ADCH_MASK;
    ADC1_SC1A |= 0x3 << ADC_SC1_ADCH_SHIFT;
    ADC1_SC1A |= ADC_SC1_DIFF_MASK;

    // Enable NVIC interrupt
    NVIC_EnableIRQ(ADC1_IRQn);
}

// ADC1 Conversion Complete ISR
void ADC1_IRQHandler(void)
{
    // Read the result (upper 12-bits). This also clears the Conversion complete flag.
    unsigned short i = ADC1_RA >> 4;

    //Set DAC output value (12bit)
    DAC0_DAT0H = i >> 8;
    DAC0_DAT0L = i;
}

void DAC0_INIT(void)
{
    //enable DAC clock
    SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;
    DAC0_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK;
    DAC0_C1 = 0;
}

void initFTM(void)
{
	//Enable clock for FTM module (use FTM0)
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

	//turn off FTM Mode to  write protection;
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;

	//divide the input clock down by 128,
	FTM0_SC |= FTM_SC_PS(7);

	//reset the counter to zero
	FTM0_CNT = 0;

	//Set the overflow rate
	//(Sysclock/128)- clock after prescaler
	//(Sysclock/128)/1000- slow down by a factor of 1000 to go from
	//Mhz to Khz, then 1/KHz = msec
	//Every 1msec, the FTM counter will set the overflow flag (TOF) and
	FTM0_MOD = (DEFAULT_SYSTEM_CLOCK / (1 << 7)) / 1000;

	//Select the System Clock
	FTM0_SC |= FTM_SC_CLKS(1);

	//Enable the interrupt mask. Timer overflow Interrupt enable
	FTM0_SC |= FTM_SC_TOIE_MASK;
	
	NVIC_EnableIRQ(FTM0_IRQn);

	return;
}

void FTM0_IRQHandler(void)
{ //For FTM timer

	//clear interrupt in register FTM0_SC
	FTM0_SC &= ~FTM_SC_TOF_MASK;
	//increment if button2 pressed
	milliCount++;

	return;
}

int main(void)
{
    char str[100];
    int buff[BUFFER_LENGTH];
    int beat_detection = 0;
    int state = 0;
    /* state machine
        1 rising beat
        2 falling
        0 else
    */
    int heart_beat = 0;
	  int diff = 0;
    // Initialize UART
    uart0_init();

    DAC0_INIT();
    ADC1_INIT();
    PDB_INIT();
		initFTM();
		// Start the PDB (ADC Conversions)
    PDB0_SC |= PDB_SC_SWTRIG_MASK;
    for (;;)
    {
        for (int j = BUFFER_INDEX; j > 0; j--)
        {
            buff[j - 1] = buff[j];
        }
        buff[BUFFER_INDEX] = ADC1_RA;
				
        //buff[BUFFER_INDEX - 1] = (buff[BUFFER_INDEX - 2] + buff[BUFFER_INDEX]) / 2; //median
				//sprintf(str, "%d\n\r", buff[BUFFER_INDEX-1]);
        //put0(str);
				
        if (state == 0 && buff[BUFFER_INDEX - 1] > 5400)
        {
            state = 1;
        }
        else if (state == 1 && buff[BUFFER_INDEX - 1] < 4000)
        {
            state = 0;
					  heart_beat++;
					  if(heart_beat == 6){
							sprintf(str, "%f BPM\n\r", ( 60.0/((float) milliCount/1000.0))*heart_beat);
							put0(str);
							heart_beat = 0;
							milliCount = 0;
						}
        }
				if (buff[BUFFER_INDEX - 1] > 50000)
				{
					 //put0("Got a heartbeat!");
				}
    }

    return 0;
}