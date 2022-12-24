#include <stdio.h>
#include <stdarg.h>

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "IO.h"
#include "UART.h"

static USART_TypeDef *_tUART[] = {
  USART1,
  USART2,
  USART3,
  UART4,
  UART5,
  USART6
};

#define N_UARTS         (sizeof(_tUART) / sizeof(USART_TypeDef *))

struct UART_CFG {
  int           ioTx;
  int           ioRx;
  uint8_t       afUART;
  uint32_t      ckUART;
};

static struct UART_CFG _uCfg[] = {
  IOP_U1TX, IOP_U1RX, GPIO_AF_USART1, RCC_APB2Periph_USART1,
  IOP_U2TX, IOP_U2RX, GPIO_AF_USART2, RCC_APB1Periph_USART2,
  IOP_U3TX, IOP_U3RX, GPIO_AF_USART3, RCC_APB1Periph_USART3,
  IOP_U4TX, IOP_U4RX, GPIO_AF_UART4, RCC_APB1Periph_UART4,
  IOP_U5TX, IOP_U5RX, GPIO_AF_UART5, RCC_APB1Periph_UART5,
  IOP_U6TX, IOP_U6RX, GPIO_AF_USART6, RCC_APB2Periph_USART6,  
};

void UART_Init(int idx, int baud)
{
  USART_InitTypeDef uInit;
  int i, port, pin;
  
  if (idx >= N_UARTS)
    return;
  
  i = _uCfg[idx].ioTx;
  port = _ios[i].port;
  pin = _ios[i].pin;
  
  // 1) ilgili pin Alternate mode'a geçirilir
  IO_Init(i, IO_MODE_ALTERNATE);
  // 2) ilgili pin için alternate function seçilir
  GPIO_PinAFConfig(GPIO_Ports[port], pin, _uCfg[idx].afUART);

  i = _uCfg[idx].ioRx;
  port = _ios[i].port;
  pin = _ios[i].pin;
  
  // 1) ilgili pin Alteranate mode'a geçirilir
  IO_Init(i, IO_MODE_ALTERNATE);
  // 2) ilgili pin için alternate function seçilir
  GPIO_PinAFConfig(GPIO_Ports[port], pin, _uCfg[idx].afUART);
  
  // 3) UART çevresel birim için clock saðla
  if (idx == UART_1 || idx == UART_6)
    RCC_APB2PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);
  else
    RCC_APB1PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);
  
  // 4) Seri haberleþme parametrelerini belirle
  uInit.USART_BaudRate = baud;
  uInit.USART_WordLength = USART_WordLength_8b;
  uInit.USART_StopBits = USART_StopBits_1;
  uInit.USART_Parity = USART_Parity_No;
  uInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  uInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  
  USART_Init(_tUART[idx], &uInit);
  
  // 5) UART çevreselini aktive et
  USART_Cmd(_tUART[idx], ENABLE);
}

// 1. strateji
// Yükleme için DR uygun olana kadar bekle
// Yüklemeyi yap çýk
void UART_Send(int idx, unsigned char ch)
{
  while (!USART_GetFlagStatus(_tUART[idx], USART_FLAG_TXE)) ;

  USART_SendData(_tUART[idx], ch);  
}

// 2. strateji
// Yüklemeyi yap, verinin gitmesini bekle
void UART_Send2(int idx, unsigned char ch)
{
  USART_SendData(_tUART[idx], ch);
  
  while (!USART_GetFlagStatus(_tUART[idx], USART_FLAG_TC)) ;
}

// UART Veri alma fonksiyonu
// Receive data register boþ olduðu müddetçe bekle
// Veriyi oku ve geri dön
unsigned char UART_Recv(int idx)
{
  while (!USART_GetFlagStatus(_tUART[idx], USART_FLAG_RXNE)) ;
  
  // Hata kontrolü
  /*
  if (USART_GetFlagStatus(_tUART[idx], USART_FLAG_ORE)) {
    //...
  }
  else if (USART_GetFlagStatus(_tUART[idx], USART_FLAG_FE)) {
    //...
  }
  else if (USART_GetFlagStatus(_tUART[idx], USART_FLAG_NE)) {
    //...
  }
  */
  
  return (unsigned char)USART_ReceiveData(_tUART[idx]);
}

#define SZ_PRNBUF       512

void UART_putch(int idx, unsigned char c)
{
  if (c == '\n')
    UART_Send2(idx, '\r');
  
  UART_Send2(idx, c);
}

// str'deki stringi idx olarak belirtilen UART'a gönderir
// geri dönüþ deðeri: yazýlan karakter sayýsý
int UART_puts(int idx, const char *str)
{
  int i = 0;
  
  while (str[i])
    UART_putch(idx, str[i++]);
  
  return i;
}

int UART_printf(int idx, const char *fmt, ...)
{
  va_list args;
  char str[SZ_PRNBUF];
  
  va_start(args, fmt);
  vsnprintf(str, SZ_PRNBUF, fmt, args);
  
  return UART_puts(idx, str);
}


