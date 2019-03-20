#ifndef PWM_H_
#define PWM_H_

void SetDutyCycle0(unsigned int DutyCycle, unsigned int Frequency, int dir);
void InitPWM0();
void PWM_ISR0();
void SetDutyCycle3(unsigned int DutyCycle, unsigned int Frequency);
void InitPWM3();
void PWM_ISR3();

#endif /* PWM_H_ */
