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
  // 1) DMA clock aktif olmalý
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  // 2) UART2 TXE (DR yükleme için uygun) ile DMA tetikleme
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  
  // 3) NVIC tarafý kesme ayarlarý
  NVIC_SetPriority(DMA1_Stream6_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

// UART2 üzerinden len byte uzunlukta veri gönderir.
// Gönderme iþkleminde DMA birimini kullanýr.
void UART2_SendDMA(const void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  // DMA1 stream6 reset
  DMA_DeInit(DMA1_Stream6);
  
  // DMA1 çevresel tarafý kesme ayarlarý
  DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
  
  // 1) DMA genel ayarlarý
  DMA_StructInit(&dmaInit);     // Ýyi bir alýþkanlýk
  
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Normal;
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dmaInit.DMA_Channel = DMA_Channel_4;
  
  // 2) DMA Kaynak (source) ayarlarý
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  // 3) DMA Hedef (destination) ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // DMA init struct 
  DMA_Init(DMA1_Stream6, &dmaInit);
  
  // DMA programlama tamamlandý, ancak transfer baþlatýlmadý
  // Transferin baþlatýlmasý için DMA_Cmd göndermemiz ferekiyor
  // DMA stream'i aktive etmeliyiz
  g_UartTx = 0;
  DMA_Cmd(DMA1_Stream6, ENABLE);
}

///////////////////////////////////////////////////////////////

void UART2_InitRxDMA(void)
{
  // 1) DMA clock aktif olmalý
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  // 2) UART2 RXNE (DR de yeni veri var) ile DMA tetikleme
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  
  // 3) NVIC tarafý kesme ayarlarý
  NVIC_SetPriority(DMA1_Stream5_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

// DMA aracýlýðýyla UART2'den blok veri alýr
// Circular buffer kullanýlacak
void UART2_RecvDMA(void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Stream5);
  
  // DMA kesmesi için çevresel taraf ayarlarý
  DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular; // Sürekli transfer
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dmaInit.DMA_Channel = DMA_Channel_4;
    
  // Kaynak ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // Hedef ayarlarý
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  DMA_Init(DMA1_Stream5, &dmaInit);
  
  // Transferi baþlatýyoruz
  DMA_Cmd(DMA1_Stream5, ENABLE);
}

void _UART2_RecvDMA(void *buf, uint16_t len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Stream5);
  
  // DMA kesmesi için çevresel taraf ayarlarý
  DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TC);
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len;
  dmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular; // Sürekli transfer
  dmaInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
  dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  dmaInit.DMA_Channel = DMA_Channel_4;
    
  // Kaynak ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  // Hedef ayarlarý
  dmaInit.DMA_Memory0BaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dmaInit.DMA_MemoryBurst = DMA_MemoryBurst_INC16;
  
  DMA_Init(DMA1_Stream5, &dmaInit);
  
  // Transferi baþlatýyoruz
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
