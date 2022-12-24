#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo.h"

// Buffer'� bo� durumuna getirir (empty)
// G�stericileri reset'ler
void FIFO_Clear(FIFO *pFifo)
{
    pFifo->pHead = pFifo->pTail = pFifo->Buffer;
    
    pFifo->bFull = FALSE;
    pFifo->nLines = 0;
}

void FIFO_Init(FIFO *pFifo, unsigned char *buf, int size)
{
  pFifo->Buffer = buf;
  pFifo->Size = size;
  
  FIFO_Clear(pFifo);
}

int FIFO_IsEmpty(FIFO *pFifo)
{
  return (pFifo->pHead == pFifo->pTail);
}

int FIFO_IsFull(FIFO *pFifo)
{
  return pFifo->bFull;
}

int FIFO_IsFullA(FIFO *pFifo)
{
  unsigned char *ptr;
  
  ptr = pFifo->pTail;
  
  if (++ptr == pFifo->Buffer + pFifo->Size)
    ptr = pFifo->Buffer;
  
  return (ptr == pFifo->pHead);    
}

// FIFO buffer'�n pTail konumuna parametredeki
// karakteri ekler. Buffer tam doluysa FALSE d�ner
int FIFO_SetData(FIFO *pFifo, unsigned char val)
{
  unsigned char *ptr = pFifo->pTail;
  
  // Dairesel buffer kural�na uygun art�rma
  if (++ptr == pFifo->Buffer + pFifo->Size)
    ptr = pFifo->Buffer;
  
  if (ptr == pFifo->pHead) {
    pFifo->bFull = TRUE;
    return FALSE;
  }
  
  *pFifo->pTail = val;  
  pFifo->pTail = ptr;
  
  /////////////////////////////////////////
  ptr = pFifo->pTail;
  if (++ptr == pFifo->Buffer + pFifo->Size)
    ptr = pFifo->Buffer;
  
  if (ptr == pFifo->pHead)
    pFifo->bFull = TRUE;
  /////////////////////////////////////////
  
  if (val == '\n')
    ++pFifo->nLines;
  
  return TRUE;
}

// FIFO buffer'dan pHead konumundaki karakteri �eker
// pHead'i ilerletir
// Buffer bo�sa bloke bekler
unsigned char FIFO_GetData(FIFO *pFifo)
{
  unsigned char val;
  
  // FIFO buffer bo� oldu�u m�ddet�e bekle
  while (FIFO_IsEmpty(pFifo)) ;
  
  val = *pFifo->pHead;
  
  // pHead g�stericisini dairesel buffer kural�na 
  // uygun olarak ilerletiyoruz
  if (++pFifo->pHead == pFifo->Buffer + pFifo->Size)
      pFifo->pHead = pFifo->Buffer;
  
  if (val == '\n')
    --pFifo->nLines;
  
  pFifo->bFull = FALSE;
  
  return val;
}

// FIFO buffer'dan line �eker. '\n' ile biten veri dizisi
// �ekilecek line yoksa FALSE d�ner
// LineBuf sonuna '\0' eklenir
// LineBuf'ta FIFO'nun buffer b�y�kl��� (pFifo->Size)
// kadar yer olmal�!
int FIFO_GetLine(FIFO *pFifo, unsigned char *LineBuf)
{
  unsigned char val;
  
  if (pFifo->bFull) {
    while (!FIFO_IsEmpty(pFifo)) {
      val = FIFO_GetData(pFifo);
      *LineBuf++ = val;
    }

    *LineBuf = '\0';
    return TRUE;
  }  
  else if (pFifo->nLines == 0)
    return FALSE;
  
  do {
    val = FIFO_GetData(pFifo);
    *LineBuf++ = val;
  } while (val != '\n');
  
  *LineBuf = '\0';
  return TRUE;
}

int FIFO_GetCount(FIFO *pFifo)
{
  int count;
  
  if (pFifo->pTail >= pFifo->pHead)
    count = pFifo->pTail - pFifo->pHead;
  else
    count = (pFifo->pTail + pFifo->Size) - pFifo->pHead;
  
  return count;
}
