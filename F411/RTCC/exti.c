#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "system.h"
#include "exti.h"

static uint32_t _EXTI_Line[] = {
  EXTI_Line0,
  EXTI_Line1,
  EXTI_Line2,
  EXTI_Line3,
  EXTI_Line4,
  EXTI_Line5,
  EXTI_Line6,
  EXTI_Line7,
  EXTI_Line8,
  EXTI_Line9,
  EXTI_Line10,
  EXTI_Line11,
  EXTI_Line12,
  EXTI_Line13,
  EXTI_Line14,
  EXTI_Line15,
};

static IRQn_Type _EXTI_IRQn[] = {
  EXTI0_IRQn,
  EXTI1_IRQn,
  EXTI2_IRQn,
  EXTI3_IRQn,
  EXTI4_IRQn,
  
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
};

static EXTITrigger_TypeDef _Trigger[] = {
  EXTI_Trigger_Rising,
  EXTI_Trigger_Falling,
  EXTI_Trigger_Rising_Falling,
};

// idx: Kesme alýnacak I/O ucu
// trigger: kesme koþulu
// priority: kesme önceliði
void EXTI_Config(IO_IDX idx, int trigger, int priority)
{
    EXTI_InitTypeDef eInit;
    int port, pin, line;
    IRQn_Type IRQn;
    
    port = _ios[idx].port;
    pin = _ios[idx].pin;
    line = _EXTI_Line[pin];
    IRQn = _EXTI_IRQn[pin];
    
    // 1) IO config
    // Ýlgili uç input olmalý!
    // Tasarým farklý yapýlabilir
    // Gözden geçirilmeli
    if (trigger == I_RISING)
      IO_Init(idx, IO_MODE_INPUT_PD);
    else if (trigger == I_FALLING)
      IO_Init(idx, IO_MODE_INPUT_PU);
    else
      IO_Init(idx, IO_MODE_INPUT);
    
    // 2) SYSCFG birimi clock aktif olmalý
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    // 3) Kesme alýnacak ucu (port, pin) ilgili EXTI hattýna baðlýyoruz
    SYSCFG_EXTILineConfig(port, pin);
    
    // 4) EXTI yapýlandýrma 
    eInit.EXTI_Line = line;
    eInit.EXTI_Mode = EXTI_Mode_Interrupt;      // Interrupt enable
    eInit.EXTI_Trigger = _Trigger[trigger];
    eInit.EXTI_LineCmd = ENABLE;        // Birim baþlatýlacak
    
    EXTI_Init(&eInit);          // Baþlatma Init fonksiyonunda yapýlýyor
    //EXTI_Cmd(ENABLE);         // Bu fonksiyon yok

    ////////////////////////////////////////////////////////////////////////////
    
    // 5) EXTI Interrupt ayarlarý
    // a) Çevresel birim tarafý
    EXTI_ClearITPendingBit(line);
    
    // b) NVIC tarafý
    NVIC_SetPriority(IRQn, priority);
    NVIC_EnableIRQ(IRQn);
}

////////////////////////////////////////////////////////////////////////////////

void EXTI0_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line0) == SET) {
    //...
          
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

void EXTI1_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line1) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

void EXTI2_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line2) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

void EXTI3_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line3) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line3);
  }
}

void EXTI4_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line4) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line4);
  }
}

void EXTI9_5_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line5) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line5);
  }

  if (EXTI_GetITStatus(EXTI_Line6) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line6);
  }

  if (EXTI_GetITStatus(EXTI_Line7) == SET) {
    //...

    EXTI_ClearITPendingBit(EXTI_Line7);
  }

  if (EXTI_GetITStatus(EXTI_Line8) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line8);
  }

  if (EXTI_GetITStatus(EXTI_Line9) == SET) {
    //...
    
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
}

void EXTI15_10_IRQHandler(void)
{
}
