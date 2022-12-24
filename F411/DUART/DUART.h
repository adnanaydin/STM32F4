#ifndef __DUART_H
#define __DUART_H

#ifdef __cplusplus
extern "C" {
#endif

#define SZ_BUF  512

extern volatile int g_UartRx, g_UartTx;

void UART2_InitTxDMA(void);
void UART2_SendDMA(const void *buf, uint16_t len);

void UART2_InitRxDMA(void);
void UART2_RecvDMA(void *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
