#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "1-Wire.h"
#include "crc8.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  BTN_Init();
  
  Sys_ConsoleInit();
  
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // 1-Wire bus ba�lang��
  OW_Init();
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

      t0 = t1;     // Son ON olma ba�lang�� zaman�
      state = S_LED_ON;
      //break;
    
  case S_LED_ON:
    if (t1 - t0 >= CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
    
  case I_LED_OFF:
      IO_Write(IOP_LED, 1);     // LED off

      t0 = t1;     // Son OFF olma ba�lang�� zaman�
      state = S_LED_OFF;
      //break;

  case S_LED_OFF:
    if (t1 - t0 >= 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  }
}

void Task_OWReset(void)
{
  FNT_t font;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  font = OLED_GetFont();
  
  OLED_SetFont(FNT_BIG);
  OLED_SetCursor(0, 0);
  printf("OWRes:%d", OW_Reset());
  
  OLED_SetFont(font);
  OLED_SetCursor(row, col);  
}

void Task_Print(void)
{
  FNT_t font;
  int row, col;
  static unsigned long count;
  
  OLED_GetCursor(&row, &col);
  font = OLED_GetFont();
  
  OLED_SetFont(FNT_BIG);
  OLED_SetCursor(0, 0);
  printf("%8lu", ++count);
  
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

void Task_ThermoB(void)
{
  int temp;
  
  OLED_SetCursor(1, 0);
  if (!DS_ReadTemp(&temp))
    printf("ERROR!  ");
  else
    printf("%6.1f C", (float)temp / 16.0);
}

void DisplayTemp(int temp)
{
  OLED_SetCursor(1, 0);
  printf("%6.1f C", (float)temp / 16.0);
}

int main()
{
  //&unsigned char serial[8];
  //int i;
  int temp;
  
  // �al��ma zaman� yap�land�rmalar�
  init();
  
  OLED_SetFont(FNT_BIG);
  //printf("Hello!");
    
  /*
  if (OW_ReadROM(serial)) {
        for (i = 0; i < 8; ++i)
          printf("%02X", serial[7 - i]);
  }
  else
    printf("Error!");
  */
  
  /*
  unsigned char crc = CRC8(serial, 8);
  OLED_SetCursor(1, 0);
  printf("CRC = %02X", crc);
  */
  
  /*
  {
    const unsigned char data[] = { 0x5F, 0x38, 0x2A, 0xEE, 0x17, 0xE7 };
    unsigned char crc;
    
    crc = CRC8(data, 6);
    printf("CRC=%02X", crc);
  }
  */
  
  /*
  {
    for (i = 0; i < 50; ++i) {
      printf("%02X ", CRC8(&i, 1));
    }
  }
  */
    
  
  // G�rev �evrimi
  while (1) 
  {
    Task_LED(); 
    Task_Print();
    Task_Button();
    
    temp = Task_Thermo();
    if (temp != TMP_INVALID) 
      DisplayTemp(temp);
  }
  
  //return 0;
}