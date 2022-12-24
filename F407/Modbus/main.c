#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "modbus.h"

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

      t0 = t1;     // Son ON olma baþlangýç zamaný
      state = S_LED_ON;
      //break;
    
  case S_LED_ON:
    if (t1 - t0 >= CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
    
  case I_LED_OFF:
      IO_Write(IOP_LED, 0);     // LED off

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
  static unsigned long count;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  OLED_SetCursor(0, 0);
  printf("SAYI=%10lu", ++count);
  OLED_SetCursor(row, col);
}

void Task_MBU(void)
{
  static unsigned char str[256];
  int len;
  static int count;

  /*
  if (IUART_GetLine(LineBuf))
    printf("%s", LineBuf);
  */
  
  if (!g_TxFlag) {
    len = sprintf((char *)str, 
      "Bu deneme amacli kesmeli gonderme islemi no: %lu\r\n",
      ++count);
    MB_SendData(str, len);
  }
}

void Task_Button(void)
{
  static int bRotate = 0;
  
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
    
  //MB_UartInit(9600);
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    //Task_Print();
    //Task_Button();
    
    Task_MB();
  }
  
  //return 0;
}
