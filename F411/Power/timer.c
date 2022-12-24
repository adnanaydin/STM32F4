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

void Timer_IntConfigX(int tmNo, int pre, int sub)
{
  // Çevresel birim tarafý
  TIM_ClearITPendingBit(_TimTab[tmNo], TIM_IT_Update);
  TIM_ITConfig(_TimTab[tmNo], TIM_IT_Update, ENABLE);
  
  // Çekirdek NVIC tarafý
  NVIC_InitTypeDef nvInit;
  
  nvInit.NVIC_IRQChannelPreemptionPriority = pre;
  nvInit.NVIC_IRQChannelSubPriority = sub;
  nvInit.NVIC_IRQChannel = _TimIrqTab[tmNo];
  nvInit.NVIC_IRQChannelCmd = ENABLE;
  
  NVIC_Init(&nvInit);
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

// TIM2 Interrupt Service Routine (ISR)
// TIM2 Interrupt Handler
void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
      ++g_T2Count;
      
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}
