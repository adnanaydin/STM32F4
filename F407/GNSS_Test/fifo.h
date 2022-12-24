#ifndef __FIFO_H
#define __FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

enum { FALSE, TRUE };
  
typedef struct {
  unsigned char         *Buffer;        // Buffer ba�lang�� adresi
  unsigned char         *pHead;         // Kuyruk ba�� g�stericisi
  unsigned char         *pTail;         // Kuyruk sonu g�stericisi
  int                   Size;           // Buffer b�y�kl���
  
  int                   bFull;          // Buffer full flag
  int                   nLines;         // Kuyruktaki c�mle say�s� '\n'
} FIFO;

void FIFO_Clear(FIFO *pFifo);
void FIFO_Init(FIFO *pFifo, unsigned char *buf, int size);
int FIFO_IsEmpty(FIFO *pFifo);
int FIFO_IsFull(FIFO *pFifo);
int FIFO_SetData(FIFO *pFifo, unsigned char val);
unsigned char FIFO_GetData(FIFO *pFifo);
int FIFO_GetLine(FIFO *pFifo, unsigned char *LineBuf);

#ifdef __cplusplus
}
#endif

#endif
