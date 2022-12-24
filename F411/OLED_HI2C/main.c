#include <stdio.h>

#include "system.h"
#include "io.h"
#include "oled.h"


void init(void)
{
  // Core tick timer baþlangýç
  Sys_ClockInit();
  
  // I/O portlarý için baþlangýç iþlemleri
  Sys_IoInit();
    
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  //IO_Write(IOP_LED, 0);
  
  Sys_ConsoleInit();
}

void Task_LED(void)
{
  static enum {
    S_LED_OFF,
    S_LED_ON
  } state = S_LED_OFF;
  
  static clock_t t0 = 0, t1;
  
  t1 = clock();
  
  switch (state) {
    case S_LED_OFF:
      if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) {
        IO_Write(IOP_LED, 0);
        t0 = t1;
        state = S_LED_ON;
      }
      break;
      
    case S_LED_ON:
      if (t1 >= t0 + CLOCKS_PER_SEC / 10) {
        IO_Write(IOP_LED, 1);
        t0 = t1;
        state = S_LED_OFF;
      }
      break;
  }
}

// Test amaçlý görev fonksiyonu
void _Task_Print(void)
{
  static clock_t c0;
  clock_t c1;
  
  c1 = clock();
  if (c1 != c0) {
    c0 = c1;
    printf("Clock:%10lu\r", c1);
  }
}

void Task_Print(void)
{
  static unsigned count;
  
  printf("%8lu\r", ++count);
}

void main(void)
{
  // Baþlangýç yapýlandýrmalarý
  init();
      
  /*
  OLED_SetFont(FNT_LARGE);
  printf("Hello, world!\n");

  OLED_SetFont(FNT_SMALL);
  OLED_SetCursor(2, 0);
  printf("ABCÇDEFGÐHIÝJKLMNOÖPRSÞTUÜVYZ 0123456789-+\n");
  printf("abcçdefgðhýijklmnoöprsþtuüvyz =!^#${}[]*%&\n");

  OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(3, 0);
  */
  //OLED_ClearDisplay();
  
  OLED_SetFont(FNT_BIG);
  printf("Hello!\n");

  // Görev çevrimi
  while (1)
  {
    Task_LED();
    Task_Print();
  }
}
