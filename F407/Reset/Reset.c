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
  // 1) IWDG reg.larýna yazma izni
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  // 2) IWDG prescale ayarý
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  
  // 3) Reload deðeri
  IWDG_SetReload(period);
  
  // 4) Sayacý reload deðerine (max) alýyoruz
  IWDG_ReloadCounter();
  
  // 5) IWDG birimini aktif ediyoruz
  IWDG_Enable();
}

void WWDG_Init(uint8_t winVal)
{
  // 1) WWDG clock aktif olmalý
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
  
  // 2) WWDG prescale deðeri
  WWDG_SetPrescaler(WWDG_Prescaler_8);
  
  // 3) WWDG Window deðeri
  WWDG_SetWindowValue(winVal);
  
  // 4) WWDG aktif edilecek
  WWDG_Enable(127);
}

