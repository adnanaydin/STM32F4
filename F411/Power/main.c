#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "system.h"
#include "button.h"
#include "oled.h"
#include "timer.h"
#include "exti.h"

#define TMR1_FREQ       100000000
#define TMR2_FREQ       100000000

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
  printf("%8lu", clock());
  
  OLED_SetFont(font);
  OLED_SetCursor(row, col);  
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

int main()
{
  // Çalýþma zamaný yapýlandýrmalarý
  init();
  
  // PWR modül clock iþaretini aktive ediyoruz
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  
  // Test amaçlý timer
  Timer_Init(TIMER_1, 2000, TMR1_FREQ / 2000, 15);
  Timer_IntConfig(TIMER_1, 2);
  Timer_Start(TIMER_1, 1);
  
  // Test amaçlý EXTI
  EXTI_Config(IOP_BTN, I_FALLING, 1);
  
  OLED_SetFont(FNT_BIG);
  
  Task_Reset();
  
  g_sButtons[0] = 0;
  
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    Task_Print();
    //Task_Button();
    
    //Task_SLEEP();
    Task_STOP();
    Task_STANDBY();
  }
  
  //return 0;
}
