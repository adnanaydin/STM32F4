#include <stdio.h>

#include "stm32f4xx.h"
#include "system_stm32f4xx.h"

#include "system.h"
#include "io.h"
#include "oled.h"

#define _STDIN  0
#define _STDOUT 1
#define _STDERR 2

volatile clock_t        g_TmTick = 0;

////////////////////////////////////////////////////

// GPIO clock enable/disable maske deðerleri
static const int GPIO_Clocks[] = {
  RCC_AHB1Periph_GPIOA,
  RCC_AHB1Periph_GPIOB,
  RCC_AHB1Periph_GPIOC,
  RCC_AHB1Periph_GPIOD,
  RCC_AHB1Periph_GPIOE,
  RCC_AHB1Periph_GPIOF,
  RCC_AHB1Periph_GPIOG,
  RCC_AHB1Periph_GPIOH,
  RCC_AHB1Periph_GPIOI,
  RCC_AHB1Periph_GPIOJ,
  RCC_AHB1Periph_GPIOK,
};

#define NPORTS  (sizeof(GPIO_Clocks) / sizeof(int))

void Sys_IoInit(void)
{
  int i;
  
  for (i = 0; i < NPORTS; ++i)
    RCC_AHB1PeriphClockCmd(GPIO_Clocks[i], ENABLE);
}

void Sys_ClockTick(void)
{
    ++g_TmTick;

    IO_ScanButtons();
}


void Sys_ClockInit(void)
{
  SysTick_Config(SystemCoreClock / CLOCKS_PER_SEC);
}

///////////////////////////////////////////////////

clock_t clock(void)
{
  return g_TmTick;
}

// Kritik olmayan bekleme
// tw >= k * c
void _DelayMs(unsigned long ms)
{
  clock_t t0;
  
  t0 = clock();
  
  while (t0 + CLOCKS_PER_SEC / 1000 * ms > clock()) ;
}

// stdout dosyasýný LCD modüle yönlendirmek için
// baþlangýç iþlemleri
void Sys_ConsoleInit(void)
{
  OLED_Start(0);
}

///////////////////////////////////////////////
// Console output (stdout yönlendirme)
// 
void _putch(unsigned char c)
{
  OLED_putch(c);
}

size_t __write(int handle, const unsigned char *buffer, size_t size)
{
  size_t nChars = 0;
  
  if (buffer == NULL)
    return 0;
  
  if (handle != _STDOUT && handle != _STDERR)
    return 0;
  
  while (size--) {
    _putch(*buffer++);
    ++nChars;
  }
  
  return nChars;
}






