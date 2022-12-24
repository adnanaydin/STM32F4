#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "system.h"
#include "io.h"

void init(void)
{
  Sys_IoInit();
  
  Sys_TickInit();
  
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
}

void Task_LED(void)
{
  static enum {
    S_INIT,
    S_LED_ON,
    S_LED_OFF
  } state = S_INIT;
  
  static clock_t t0, t1;
  
  t1 = clock();
  
  switch (state) {
  case S_INIT:
      IO_Write(IOP_LED, 0);

      t0 = clock();
      state = S_LED_ON;
      break;
    
  case S_LED_ON:
    if (t1 - t0 >= CLOCKS_PER_SEC / 10) {
      IO_Write(IOP_LED, 1);
      
      t0 = t1;
      state = S_LED_OFF;
    }      
    break;
    
  case S_LED_OFF:
    if (t1 - t0 >= 9 * CLOCKS_PER_SEC / 10) {
      IO_Write(IOP_LED, 0);

      t0 = t1;
      state = S_LED_ON;
    }      
    break;
  }
}

void Task_A(void)
{
}

void Task_B(void)
{
  static clock_t t0;
  
  if (clock() - t0 > 10000)
    while (1) ;
}

void Task_C(void)
{
}

void Task_D(void)
{
}

void Task_E(void)
{
}

int main()
{
  
  // Çalýþma zamaný yapýlandýrmalarý
  init();
    
  // Görev çevrimi
  while (1) 
  {
    Task_LED(); 
    
    Task_A();
    
    //Task_B();
    
    Task_C();
    
    Task_D();
    
    Task_E();
  }
  
  //return 0;
}
