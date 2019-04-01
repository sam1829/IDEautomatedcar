#ifndef PWM_H_
#define PWM_H_

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */
#define CLOCK 20485760u
#define PWM_FREQUENCY0 10000
#define FTM0_MOD_VALUE (CLOCK / PWM_FREQUENCY0)
#define PWM_FREQUENCY3 50
#define FTM3_MOD_VALUE (CLOCK / (1 << 7) / PWM_FREQUENCY3)

static volatile unsigned int PWMTick0 = 0;
static volatile unsigned int PWMTick3 = 0;

void SetDutyCycle0(unsigned int DutyCycle, unsigned int Frequency, int dir);
void InitPWM0(void);
void PWM_ISR0(void);
void SetDutyCycle3(float DutyCycle, unsigned int Frequency);
void SetDutyCycle0(unsigned int DutyCycle, unsigned int Frequency, int dir);
void InitPWM3(void);
void PWM_ISR3(void);

#endif /* PWM_H_ */
