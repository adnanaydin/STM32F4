#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "uart.h"
#include "duart.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  BTN_Init();
  
  Sys_ConsoleInit();
  
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  UART_Init(UART_2, 115200);
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
  static unsigned long count;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  OLED_SetCursor(0, 0);
  printf("SAYI=%10lu", ++count);
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

void Task_UartTx(void)
{
  static unsigned long count;
  static char buf[256];
  int len;
  
  if (g_UartTx) {
    len = sprintf(buf, "Bu yazi DMA araciligi ile gonderiliyor: %lu\r\n",
                  ++count);
    
    UART2_SendDMA(buf, len);
  }
}

int g_RxLen = 16;
unsigned char g_RxBuf[SZ_BUF];

void Task_UartRx(void)
{
  if (g_UartRx) {
    g_RxBuf[g_RxLen] = '\0';
    printf("%s", g_RxBuf);
    
    g_UartRx = 0;
  }
}

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();
  
  // DMA ile veri gönderim ayarlarý
  UART2_InitTxDMA();
  
  // DMA ile veri alma ayarlarý
  UART2_InitRxDMA();
  UART2_RecvDMA(g_RxBuf, g_RxLen);
  
  OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(1, 0);
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    Task_Print();
    Task_Button();
    
    Task_UartTx();
    Task_UartRx();
  }
  
  //return 0;
}
