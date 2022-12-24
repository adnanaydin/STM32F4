#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "system.h"
#include "uart.h"
#include "duart.h"
#include "fifo.h"

#define IUART_ST        USART2
#define IUART           UART_2
#define IUART_IRQn      USART2_IRQn

volatile int g_UartRx = 0, g_UartTx = 1;

void UART2_InitTxDMA(void)
{
  // 1) DMA clock aktif olmal�
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  // 2) UART2 TXE (DR y�kleme i�in uygun) ile DMA tetikleme
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  
  // 3) NVIC taraf� kesme ayarlar�
  NVIC_SetPriority(DMA1_Stream6_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

// UART2 �zerinden len byte uzunlukta veri g�nderir.
// G�nderme i�kleminde DMA birimini kullan�r.
void UART2_SendDMA(const void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  // DMA1 stream6 reset
  DMA_DeInit(DMA1_Stream6);
  
  // DMA1 �evresel taraf� kesme ayarlar�
  DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
  
  // 1) DMA genel ayarlar�
  DMA_StructInit(&dmaInit);     // �yi bir al��kanl�k
  
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Normal;
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dmaInit.DMA_Channel = DMA_Channel_4;
  
  // 2) DMA Kaynak (source) ayarlar�
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  // 3) DMA Hedef (destination) ayarlar�
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // DMA init struct 
  DMA_Init(DMA1_Stream6, &dmaInit);
  
  // DMA programlama tamamland�, ancak transfer ba�lat�lmad�
  // Transferin ba�lat�lmas� i�in DMA_Cmd g�ndermemiz ferekiyor
  // DMA stream'i aktive etmeliyiz
  g_UartTx = 0;
  DMA_Cmd(DMA1_Stream6, ENABLE);
}

///////////////////////////////////////////////////////////////

void UART2_InitRxDMA(void)
{
  // 1) DMA clock aktif olmal�
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  // 2) UART2 RXNE (DR de yeni veri var) ile DMA tetikleme
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  
  // 3) NVIC taraf� kesme ayarlar�
  NVIC_SetPriority(DMA1_Stream5_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

// DMA arac�l���yla UART2'den blok veri al�r
// Circular buffer kullan�lacak
void UART2_RecvDMA(void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Stream5);
  
  // DMA kesmesi i�in �evresel taraf ayarlar�
  DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular; // S�rekli transfer
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dmaInit.DMA_Channel = DMA_Channel_4;
    
  // Kaynak ayarlar�
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // Hedef ayarlar�
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  DMA_Init(DMA1_Stream5, &dmaInit);
  
  // Transferi ba�lat�yoruz
  DMA_Cmd(DMA1_Stream5, ENABLE);
}

void _UART2_RecvDMA(void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Stream5);
  
  // DMA kesmesi i�in �evresel taraf ayarlar�
  DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular; // S�rekli transfer
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  dmaInit.DMA_Channel = DMA_Channel_4;
    
  // Kaynak ayarlar�
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // Hedef ayarlar�
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_INC16;
  
  DMA_Init(DMA1_Stream5, &dmaInit);
  
  // Transferi ba�lat�yoruz
  DMA_Cmd(DMA1_Stream5, ENABLE);
}

///////////////////////////////////////////////////////////////

void DMA1_Stream5_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) == SET) {
    g_UartRx = 1;
     
    DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
  }
}

void DMA1_Stream6_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) == SET) {
    g_UartTx = 1;
    
    DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
  }
}
