#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "timer.h"
#include "exti.h"
#include "rtcc.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  BTN_Init();
  
  Sys_ConsoleInit();
  
  // RTC ce backup domain baþlangýç
  RTCC_Init();
  
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

void Task_Button(void)
{
  if (g_sButtons[0]) {
    g_sButtons[0] = 0;
    
    OLED_SetCursor(1, 0);
    printf("A0");
  }
  else if (g_sButtons[1]) {
    g_sButtons[1] = 0;
    
    OLED_SetCursor(1, 0);
    printf("A1");
  }
}

void Task_Reset(void)
{
  char *msg;
  
  if (PWR_GetFlagStatus(PWR_FLAG_SB)) 
    PWR_ClearFlag(PWR_FLAG_SB);

  if (PWR_GetFlagStatus(PWR_FLAG_WU)) {
    PWR_ClearFlag(PWR_FLAG_WU);
    
    Sys_ResetClock();
    
    msg = "Wakeup  ";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_PORRST)) {
    Sys_ResetClock();
    
    msg = "Power On";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST)) {
    Sys_ResetClock();
    
    msg = "Software";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
    msg = "Indp WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
    msg = "Wind WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST)) {
    Sys_ResetClock();
    
    msg = "LowPower";
  }
  else if (RCC_GetFlagStatus(RCC_FLAG_PINRST))
    msg = "External";      
  
  OLED_SetCursor(1, 0);
  printf(msg);
  
  RCC_ClearFlag();
}

void Task_SLEEP(void)
{
  if (g_T1Count) {
    OLED_SetCursor(1, 0);
    printf("SLEEP   ");
    
    IO_Write(IOP_LED, 1);       // LED off
    
    SysTick->CTRL = 0;  // Sys tick timer duracak
    
    // Sleep mode
    __WFI();

    Sys_TickInit();
    
    OLED_SetCursor(1, 0);
    printf("Running ");
    
    g_T1Count = 0;
  }
}

void Task_STOP(void)
{
  if (g_sButtons[1]) {
    g_sButtons[1] = 0;

    OLED_SetCursor(1, 0);
    printf("STOP    ");
    
    IO_Write(IOP_LED, 1);       // LED off

    // Stop modu
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    
    // Çýkýþta kaldýðý yerden devam eder ama
    // clock iþareti internal RC'ye döner
    // SystemInit() fonksiyonunu çaðýrarak main clock olarak
    // XT-PLL'i aktive etmeliyiz
    SystemInit();

    OLED_SetCursor(1, 0);
    printf("Running ");
  }
}

// Deep sleep: Yazýlýmsal kapatma
void Task_STANDBY(void)
{
  if (g_sButtons[0]) {
    //g_sButtons[0] = 0; // Gerekmiyor çünkü baþtan baþlayacak
    
    // A0 butonu için uyandýrma fonksiyonu
    // Buton A0 ucu ile + besleme arasýnda olmalý
    PWR_WakeUpPinCmd(ENABLE);
    
    // Wakeup flag biti sýfýrlanmalý
    PWR_ClearFlag(PWR_FLAG_WU);
    
    PWR_EnterSTANDBYMode();
  }
}

void Task_RTCC(void)
{
  FNT_t font;
  int row, col;
  static int sec = -1, date, count;
  TIME_t tm0;
  
  OLED_GetCursor(&row, &col);
  font = OLED_GetFont();
  OLED_SetFont(FNT_BIG);
  
  RTCC_GetTime(&tm0);

  if (tm0.sec != sec) {
    sec = tm0.sec;
    
    OLED_SetCursor(0, 0);
    printf("%02u:%02u:%02u", tm0.hour, tm0.min, tm0.sec);
    
    if (count) {
      --count;
      if (count == 0)
        date = 0;
    }
  }
  
  if (g_AlarmA) {
    g_AlarmA = 0;
    
    OLED_SetCursor(1, 0);
    printf("ALARM A!");
    
    count = 5;
  }  
  else if (tm0.date != date) {
    date = tm0.date;
  
    OLED_SetCursor(1, 0);
    printf("%02u/%02u/%02u", tm0.date, tm0.mon, tm0.year);
  }
  
  OLED_SetFont(font);
  OLED_SetCursor(row, col);  
}

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();
  
#if 0  
  {
    TIME_t tm0;
    
    tm0.date = 5;
    tm0.day = 1;
    tm0.mon = 12;
    tm0.year = 22;
    
    tm0.hour = 21;
    tm0.min = 32;
    tm0.sec = 0;
    
    RTCC_SetTime(&tm0);
  }
#endif  
  
  OLED_SetFont(FNT_BIG);
  
  /*
  //RTC_WriteBackupRegister(RTC_BKP_DR0, 0x12345678);
  uint32_t val = RTC_ReadBackupRegister(RTC_BKP_DR0);
  OLED_SetCursor(1, 0);
  printf("%08lX", val);
  */
  
  // Dakikada bir alarm
  TIME_t tm0;
  
  tm0.date = 5;
  tm0.day = 2;
  tm0.mon = 12;
  tm0.year = 22;
  
  tm0.hour = 22;
  tm0.min = 44;
  tm0.sec = 15;

  RTCC_SetAlarm(&tm0, RTCC_ALR_SEC);
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    Task_RTCC();
  }
  
  //return 0;
}
