#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"

#include "oled.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
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

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();

  /*
  OLED_SetFont(FNT_SMALL);
  OLED_SetCursor(0, 0);
  
  OLED_PutChar('H');
  OLED_PutChar('e');
  OLED_PutChar('l');
  OLED_PutChar('l');
  OLED_PutChar('o');
  OLED_PutChar(' ');
  OLED_PutChar('w');
  OLED_PutChar('o');
  OLED_PutChar('r');
  OLED_PutChar('l');
  OLED_PutChar('d');
  OLED_PutChar('!');
  */
  
  OLED_SetFont(FNT_BIG);
  OLED_SetCursor(0, 0);
  
  OLED_PutChar('0');
  OLED_PutChar('1');
  OLED_PutChar('2');
  OLED_PutChar('3');
  OLED_PutChar('4');
  OLED_PutChar('5');
  OLED_PutChar('6');
  OLED_PutChar('7');

  OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(2, 0);
  
  OLED_PutChar('M');
  OLED_PutChar('e');
  OLED_PutChar('r');
  OLED_PutChar('h');
  OLED_PutChar('a');
  OLED_PutChar('b');
  OLED_PutChar('a');
  OLED_PutChar(' ');
  OLED_PutChar('d');
  OLED_PutChar('ü');
  OLED_PutChar('n');
  OLED_PutChar('y');
  OLED_PutChar('a');

  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
  }
  
  //return 0;
}
