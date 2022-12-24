#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "timer.h"

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
  OLED_SetCursor(1, 0);
  printf("%8lu", g_T1Count);
  
  OLED_SetFont(font);
  OLED_SetCursor(row, col);  
}

void Task_Button(void)
{
  static int bRotate = 1;
  
  if (g_sButtons[0]) {
    g_sButtons[0] = 0;
    
    bRotate = !bRotate;
    OLED_Rotate(bRotate);
  }
}

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();
  
  printf("Hello, world!");
  
  // TIMER_1 1s periyotlu kesme oluþtursun
  // ftmr = 2000 Hz, period = 2000
  // Taþma frekansý: Fbus / prescale / period
  // = Fbus / (prescale * period)
  // Taþma zamaný saniye olarak
  // = (prescale * period) / Fbus
  Timer_Init(TIMER_1, 10000, 10, 1);
  //Timer_Init(TIMER_1, 1000, 1000, 1);
  
  Timer_IntConfig(TIMER_1, 3);  // !!!
  Timer_Start(TIMER_1, 1);
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    Task_Print();
    Task_Button();
    
    if (g_T1Count >= 17000)
      Timer_Start(TIMER_1, 0);
  }
  
  //return 0;
}
