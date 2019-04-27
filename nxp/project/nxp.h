

#define HARD_RIGHT 8.5
#define CENTER 6.75
#define HARD_LEFT 4.75

#define TRUE_CENTER 60
#define CENTER_RANGE 6
#define CENTER_MAX (TRUE_CENTER + CENTER_RANGE)
#define CENTER_MIN (TRUE_CENTER - CENTER_RANGE)

#define min(x, y) (x < y) ? 1 : 0
#define max(x, y) (x > y) ? 1 : 0

#define Kp 0.1
#define Kp_speed 0.4

//Fast
#define MAX_SPEED0 80
#define MIN_SPEED0 50

//Normal
#define MAX_SPEED1 75//75
#define MIN_SPEED1 50

//Slow
#define MAX_SPEED2 65
#define MIN_SPEED2 50

extern  int max_speed;
extern  int min_speed;
extern  int mode;

void initialize(void);
void delay(int del);
void convolve(int *input, int *output, int length, int *h, int h_length);
int *findMinMax(int *input, int start, int end, int *output);
void debugCamera(void);
void turn(int center);
float clip(float input, float min, float max);
void LED_Init(void);
void Button_Init(void);
int countPeaks(int *input, int length, int max, int min);
