#ifndef _UART_H
#define _UART_H

enum {
  UART_1,
  UART_2,
  UART_3,
  UART_4,
  UART_5,
  UART_6,
};

void UART_Init(int idx, int baud);
void UART_Send(int idx, unsigned char ch);
unsigned char UART_Recv(int idx);

#endif