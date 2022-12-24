#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "timer.h"

#define TMR1_FREQ       100000000
#define TMR2_FREQ       100000000

float g_Freq = 1000.0;

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  BTN_Init();
  
  Sys_ConsoleInit();
  
  IO_Write(IOP_LED, 1);
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
      IO_Write(IOP_LED, 0);     // LED on

      t0 = t1;     // Son ON olma baþlangýç zamaný
      state = S_LED_ON;
      //break;
    
  case S_LED_ON:
    if (t1 - t0 >= CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
    
  case I_LED_OFF:
      IO_Write(IOP_LED, 1);     // LED off

      t0 = t1;     // Son OFF olma baþlangýç zamaný
      state = S_LED_OFF;
      //break;

  case S_LED_OFF:
    if (t1 - t0 >= 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  }
}

void Task_Print(void)
{
  //static unsigned long count;
  FNT_t font;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  font = OLED_GetFont();
  
  OLED_SetFont(FNT_BIG);
  OLED_SetCursor(0, 0);
  printf("%8lu", g_T1Count);
  OLED_SetCursor(1, 0);
  printf("%8lu", g_T2Count);
  
  OLED_SetFont(font);
  OLED_SetCursor(row, col);  
}

void DisplayFreq(void)
{
  OLED_SetCursor(0, 0);
  printf("%8.0f", g_Freq);
}

void Task_Button(void)
{
  if (g_sButtons[0]) {
    g_sButtons[0] = 0;

    g_Freq += 1000;
    
    PWM_Setup(g_Freq, 50);
    DisplayFreq();
  }
}

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();
  
  OLED_SetFont(FNT_BIG);
  
  // 1,000,000 ns period, 500,000 ns duty
  PWM_Init(100000, 50000);
  //PWM_Init(2, 1);     // Olasý en büyük frekans
  //PWM_Init(4294967295, 2000000000); // olasý en düþük frekans
  
  //PWM_Setup(19000, 25.7);
  PWM_Setup(FREQ_SAMPLING, 0);
  Timer_IntConfig(TIMER_2, 3);

  DisplayFreq();
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    //Task_Print();
    Task_Button();
  }
  
  //return 0;
}
