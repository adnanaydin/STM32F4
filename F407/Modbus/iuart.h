#ifndef __IUART_H
#define __IUART_H

#ifdef __cplusplus
extern "C" {
#endif

#define SZ_BUF  512


void IUART_Init(int baud);
int IUART_GetLine(unsigned char *LineBuf);

void IUART_SendData(const void *buf, int len);
int IUART_TxActive(void);

#ifdef __cplusplus
}
#endif

#endif
