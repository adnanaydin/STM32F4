#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "io.h"
#include "system.h"
#include "timer.h"

static TIM_TypeDef *_TimTab[] = {
  TIM1, TIM2, TIM3, TIM4, TIM5
};

static uint32_t _TimRccTab[] = {
  RCC_APB2Periph_TIM1,
  RCC_APB1Periph_TIM2,
  RCC_APB1Periph_TIM3,
  RCC_APB1Periph_TIM4,
  RCC_APB1Periph_TIM5,
};

static IRQn_Type _TimIrqTab[] = {
  TIM1_UP_TIM10_IRQn,
  TIM2_IRQn,
  TIM3_IRQn,
  TIM4_IRQn,
  TIM5_IRQn,
};

void Timer_Reset(TIMER_t tmNo)
{
  TIM_SetCounter(_TimTab[tmNo], 0);
}

// Belirtilen timer'ý prescale ve period deðerlerine uygun olarak
// yapýlandýrýr
void Timer_Init(TIMER_t tmNo, unsigned prescale, unsigned period, unsigned repeat)
{
  TIM_TimeBaseInitTypeDef tmInit;

  // 1) Timer birimine clock iþareti saðlýyoruz
  if (tmNo == TIMER_1)
    RCC_APB2PeriphClockCmd(_TimRccTab[tmNo], ENABLE);
  else
    RCC_APB1PeriphClockCmd(_TimRccTab[tmNo], ENABLE);
  
  // 2) Timer parametrelerini ayarlýyoruz
  tmInit.TIM_ClockDivision = TIM_CKD_DIV1;
  tmInit.TIM_CounterMode = TIM_CounterMode_Up;
  tmInit.TIM_Period = period - 1;
  tmInit.TIM_Prescaler = prescale - 1;
  tmInit.TIM_RepetitionCounter = repeat - 1;
  TIM_TimeBaseInit(_TimTab[tmNo], &tmInit);
  
  // Timer counter register'ýný sýfýrlýyoruz
  Timer_Reset(tmNo);  
}

void Timer_Start(TIMER_t tmNo, int bStart)
{
  TIM_Cmd(_TimTab[tmNo], bStart ? ENABLE : DISABLE);
}

void Timer_IntConfig(int tmNo, int priority)
{
  // Çevresel birim tarafý
  TIM_ClearITPendingBit(_TimTab[tmNo], TIM_IT_Update);
  TIM_ITConfig(_TimTab[tmNo], TIM_IT_Update, ENABLE);
  
  // Çekirdek NVIC tarafý
  // a) Öncelik
  NVIC_SetPriority(_TimIrqTab[tmNo], priority);
  // b) IRQn izini aktive edilecek (kesme izni)
  NVIC_EnableIRQ(_TimIrqTab[tmNo]);
}

////////////////////////////////////////////////////////////////////////////////

// PWM periodu: period * 10ns
// Duty cycle: duty * 10 ns
void PWM_Init(uint32_t period, uint32_t duty)
{
  // 1) Çýkýþ kanalýnýn I/O ayarlarý
  IO_Init(IOP_PWM, IO_MODE_ALTERNATE);
  GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_PWM].port], _ios[IOP_PWM].pin, GPIO_AF_TIM2);

  // 2) Timer ayarlarý (PWM frekansýný belirler)
  Timer_Init(TIMER_2, 1, period, 1);
  
  // 3) PWM ayarlarý (OC ayarlarý)
  TIM_OCInitTypeDef ocInit;
  
  ocInit.TIM_OCIdleState = TIM_OCIdleState_Reset;
  ocInit.TIM_OCMode = TIM_OCMode_PWM1;
  ocInit.TIM_OCPolarity = TIM_OCPolarity_High;
  ocInit.TIM_OutputState = TIM_OutputState_Enable;
  ocInit.TIM_Pulse = duty;
  
  TIM_OC3Init(TIM2, &ocInit);
  
  // Compare Reg double buf.
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
  
  // 4) Timer'ý çalýþtýrýyoruz (PWM çalýþmaya baþlýyor)
  Timer_Start(TIMER_2, 1);
}

// freq: PWM frekansý
// duty: % olarak duty cycle
void PWM_Setup(float freq, float duty)
{
  uint32_t AutoReload, DutyCount;
  
  if (freq <= 0) {
    freq = 1000;
    duty = 0;
  }
  
  if (duty < 0 || duty > 100)
    duty = 0;
  
  AutoReload = (uint32_t)(SystemCoreClock / freq + 0.5);
  if (AutoReload == 0) 
    return;
    
  DutyCount = (uint32_t)(AutoReload * duty / 100.0);
  
  TIM_Cmd(TIM2, DISABLE);
  
  TIM_SetAutoreload(TIM2, AutoReload);
  TIM_SetCompare3(TIM2, DutyCount);
  
  TIM_SetCounter(TIM2, 0);
  
  TIM_Cmd(TIM2, ENABLE);
}

////////////////////////////////////////////////////////////////////////////////

volatile unsigned long g_T1Count;
volatile unsigned long g_T2Count;

// TIM1 Interrupt Service Routine (ISR)
// TIM1 Interrupt Handler
void TIM1_UP_TIM10_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
      ++g_T1Count;
      
      TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  }
}


#define PI              3.14159265359
#define FREQ_TIMER      SystemCoreClock
#define FREQ_SIGNAL     440.0

#define DUTY_MAX        (FREQ_TIMER / FREQ_SAMPLING - 10)

// TIM2 Interrupt Service Routine (ISR)
// TIM2 Interrupt Handler
void TIM2_IRQHandler(void)
{
  static float y, t = 0;
  uint32_t val;
  
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
    // y = A/2 + A/2 * sin(2 * PI * f * t)
    y = (DUTY_MAX / 2) + (DUTY_MAX / 2) * sin(2 * PI * FREQ_SIGNAL * t);
    val = (uint32_t)(y + 0.5);
    
    TIM_SetCompare3(TIM2, val);
    
    t += 1.0 / FREQ_SAMPLING;
    if (t >= 2 * PI)
      t -= 2 * PI;
      
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}
