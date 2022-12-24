#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo.h"

// Buffer'ý boþ durumuna getirir (empty)
// Göstericileri reset'ler
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

// FIFO buffer'ýn pTail konumuna parametredeki
// karakteri ekler. Buffer tam doluysa FALSE döner
int FIFO_SetData(FIFO *pFifo, unsigned char val)
{
  unsigned char *ptr = pFifo->pTail;
  
  // Dairesel buffer kuralýna uygun artýrma
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

// FIFO buffer'dan pHead konumundaki karakteri çeker
// pHead'i ilerletir
// Buffer boþsa bloke bekler
unsigned char FIFO_GetData(FIFO *pFifo)
{
  unsigned char val;
  
  // FIFO buffer boþ olduðu müddetçe bekle
  while (FIFO_IsEmpty(pFifo)) ;
  
  val = *pFifo->pHead;
  
  // pHead göstericisini dairesel buffer kuralýna 
  // uygun olarak ilerletiyoruz
  if (++pFifo->pHead == pFifo->Buffer + pFifo->Size)
      pFifo->pHead = pFifo->Buffer;
  
  if (val == '\n')
    --pFifo->nLines;
  
  pFifo->bFull = FALSE;
  
  return val;
}

// FIFO buffer'dan line çeker. '\n' ile biten veri dizisi
// Çekilecek line yoksa FALSE döner
// LineBuf sonuna '\0' eklenir
// LineBuf'ta FIFO'nun buffer büyüklüðü (pFifo->Size)
// kadar yer olmalý!
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
