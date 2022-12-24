#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "system.h"
#include "uart.h"
#include "iuart.h"
#include "fifo.h"

#define IUART_ST        USART6
#define IUART           UART_6
#define IUART_IRQn      USART6_IRQn

static unsigned char _RxBuf[SZ_BUF];
static unsigned char _TxBuf[SZ_BUF];

static FIFO     _RxFifo;
static FIFO     _TxFifo;
static int      _TxFlag;

void IUART_Init(int baud)
{
  UART_Init(IUART, baud);
  
  // FIFO ba�lang��
  FIFO_Init(&_RxFifo, _RxBuf, SZ_BUF);
  FIFO_Init(&_TxFifo, _TxBuf, SZ_BUF);
  
  // Interrupt ayarlar�
  // a) ST yakas�
  USART_ITConfig(IUART_ST, USART_IT_RXNE, ENABLE);
  USART_ITConfig(IUART_ST, USART_IT_TXE, DISABLE);
  
  // b) ARM yakas�
  NVIC_SetPriority(IUART_IRQn, 1);
  NVIC_EnableIRQ(IUART_IRQn);
}

void USART6_IRQHandler(void)
{
  unsigned char c;
  
  if (USART_GetITStatus(IUART_ST, USART_IT_RXNE)) {
    c = USART_ReceiveData(IUART_ST);
    
    FIFO_SetData(&_RxFifo, c);
  }
  
  if (USART_GetITStatus(IUART_ST, USART_IT_TXE)) {
    // Veriyi FIFO g�nderme buffer'�ndan �ek
    c = FIFO_GetData(&_TxFifo);
    
    // �ekilen veriyi TDR'a y�kle
    USART_SendData(IUART_ST, c);
    
    if (FIFO_IsEmpty(&_TxFifo)) {
      USART_ITConfig(IUART_ST, USART_IT_TXE, DISABLE);
      _TxFlag = 0;
    }
  }
}

int IUART_GetLine(unsigned char *LineBuf)
{
  return FIFO_GetLine(&_RxFifo, LineBuf);
}

// Interrupt y�ntemi ile UART �zerinden blok veri g�nderir
void IUART_SendData(const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  // �nce Tx FIFO'ya g�nderece�imiz veri y���n�n� y�kl�yoruz
  while (!FIFO_IsFull(&_TxFifo) && (len-- > 0))
    FIFO_SetData(&_TxFifo, *ptr++);
  
  _TxFlag = TRUE;
  USART_ITConfig(IUART_ST, USART_IT_TXE, ENABLE);
}

int IUART_TxActive(void)
{
  return _TxFlag;
}


