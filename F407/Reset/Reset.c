#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "io.h"
#include "system.h"
#include "oled.h"

extern unsigned long g_count;

void Task_Reset(void)
{
  char *msg;
  
  if (RCC_GetFlagStatus(RCC_FLAG_PORRST)) {
    g_count = 0;
    
    msg = "Power On";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST))
    msg = "Software";
  else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
    msg = "Indp WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
    msg = "Wind WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST)) {
    g_count = 0;
    
    msg = "LowPower";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_PINRST))
    msg = "External";      
  
  OLED_SetCursor(1, 0);
  printf(msg);
  
  RCC_ClearFlag();
}

void IWDG_Init(uint16_t period)
{
  // 1) IWDG reg.lar�na yazma izni
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  // 2) IWDG prescale ayar�
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  
  // 3) Reload de�eri
  IWDG_SetReload(period);
  
  // 4) Sayac� reload de�erine (max) al�yoruz
  IWDG_ReloadCounter();
  
  // 5) IWDG birimini aktif ediyoruz
  IWDG_Enable();
}

void WWDG_Init(uint8_t winVal)
{
  // 1) WWDG clock aktif olmal�
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
  
  // 2) WWDG prescale de�eri
  WWDG_SetPrescaler(WWDG_Prescaler_8);
  
  // 3) WWDG Window de�eri
  WWDG_SetWindowValue(winVal);
  
  // 4) WWDG aktif edilecek
  WWDG_Enable(127);
}

