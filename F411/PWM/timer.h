#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TIMER_1,
  TIMER_2,
  TIMER_3,
  TIMER_4,
  TIMER_5,
} TIMER_t;
    
extern volatile unsigned long g_T1Count;
extern volatile unsigned long g_T2Count;

void Timer_Reset(TIMER_t tmNo);
void Timer_Init(TIMER_t tmNo, unsigned prescale, unsigned period, unsigned repeat);
void Timer_Start(TIMER_t tmNo, int bStart);

void Timer_IntConfig(int tmNo, int priority);

void PWM_Init(uint32_t period, uint32_t duty);
void PWM_Setup(float freq, float duty);

#define FREQ_SAMPLING   16000


#ifdef __cplusplus
}
#endif

#endif
