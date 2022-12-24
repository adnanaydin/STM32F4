#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "reset.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  BTN_Init();
  
  Sys_ConsoleInit();
  
  IO_Write(IOP_LED, 0);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
}

void Task_LED(void)
{
  static enum {
    I_LED_ON,
    S_LED_ON,
    
    I_LED_OFF,
    S_LED_OFF
  } state = I_LED_ON;
  
  static clock_t t0, t1;
  
  t1 = clock();
  
  switch (state) {
  case I_LED_ON:
      IO_Write(IOP_LED, 1);     // LED on

      t0 = t1;     // Son ON olma ba�lang�� zaman�
      state = S_LED_ON;
      //break;
    
  case S_LED_ON:
    if (t1 - t0 >= CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
    
  case I_LED_OFF:
      IO_Write(IOP_LED, 0);     // LED off

      t0 = t1;     // Son OFF olma ba�lang�� zaman�
      state = S_LED_OFF;
      //break;

  case S_LED_OFF:
    if (t1 - t0 >= 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  }
}

/* __no_init */ unsigned long g_count;

void Task_Print(void)
{
  int row, col;
  
  OLED_GetCursor(&row, &col);
  OLED_SetCursor(0, 0);
  printf("%8lu", ++g_count);
  OLED_SetCursor(row, col);
}

void Task_Clock(void)
{
  int row, col;
  clock_t t;
  
  t = clock() / 100;
  OLED_GetCursor(&row, &col);
  OLED_SetCursor(0, 0);
  printf("%8lu", t);
  OLED_SetCursor(row, col);
}

void Task_Button(void)
{
  //static int bRotate = 1;
  
  if (g_sButtons[0]) {
    g_sButtons[0] = 0;
    
    NVIC_SystemReset();
        
    //IWDG_ReloadCounter();

    //bRotate = !bRotate;
    //OLED_Rotate(bRotate);
  }
}

int main()
{
  // �al��ma zaman� yap�land�rmalar�
  init();
  
  OLED_SetFont(FNT_BIG);
  Task_Reset();

  IWDG_Init(1000);
  //WWDG_Init(80);
  
  // PCLK1 = 84 Mhz
  // t = (4096 * ps) / fpclk1
  // WWDG clock period (4096 * 8) / 84000000 = 39 ms
  
    // G�rev �evrimi
  while (1) 
  {
    Task_LED(); 
    Task_Clock();
    Task_Button();  

    //DelayMs(20);
    //WWDG_SetCounter(127);
        
    IWDG_ReloadCounter();
  }
  
  //return 0;
}
