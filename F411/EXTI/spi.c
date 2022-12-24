#include "io.h"
#include "spi.h"

////////////////////////////////////////////////////////////////
// SOFTWARE SPI

static int _CPOL = 0;
static int _CPHA = 0;

void SSPI_Delay(void)
{
}

void SSPI_Start(int mode)
{
  mode &= 3;
  
  _CPOL = (mode >> 1);
  _CPHA = mode & 1;
  
  // SPI uçlarý yapýlandýrýlacak
  IO_Write(IOP_SPI_SCK, _CPOL);
  IO_Init(IOP_SPI_SCK, IO_MODE_OUTPUT);
  
  IO_Init(IOP_SPI_MOSI, IO_MODE_OUTPUT);
  IO_Init(IOP_SPI_MISO, IO_MODE_INPUT);
}

// SPI birimi üzerinden 1-byte veri gönderir ve alýr
// Parametre val: gönderilen veri
// Geri dönüþ deðeri: alýnan veri
unsigned char SSPI_Data(unsigned char val)
{
  int i;
  
  for (i = 0; i < 8; ++i) {
    // 1) Data setup: Gönderilecek verinin hazýrlanmasý
    IO_Write(IOP_SPI_MOSI, val & 0x80 ? 1 : 0);
    val <<= 1;
    SSPI_Delay();    // Data setup time
    
    // 2a) Clock generation: Saat iþareti üretimi
    IO_Write(IOP_SPI_SCK, !_CPOL);
    SSPI_Delay();
    
    // 3a) Sampling: Veri okuma
    if (!_CPHA)
      if (IO_Read(IOP_SPI_MISO))
          val |= 1;

    // 2b) Clock incative
    IO_Write(IOP_SPI_SCK, _CPOL);
    SSPI_Delay();
      
    // 3b) Sampling: Veri okuma
    if (_CPHA)
      if (IO_Read(IOP_SPI_MISO))
          val |= 1;    
  }
          
  return val;
}

#if 0
unsigned char _SSPI_Data(unsigned char val)
{
  int i;
  
  for (i = 0; i < 8; ++i) {
    // 1) Data setup: Gönderilecek verinin hazýrlanmasý
    IO_Write(IOP_SPI_MOSI, val & 0x80 ? 1 : 0);
    val <<= 1;
    SSPI_Delay();    // Data setup time
    
    // 2a) Clock generation: Saat iþareti üretimi
    IO_Write(IOP_SPI_SCK, !_CPOL);
    SSPI_Delay();

    // 2b) Clock incative
    if (_CPHA) {
      IO_Write(IOP_SPI_SCK, _CPOL);
      SSPI_Delay();
    }
      
    // 3b) Sampling: Veri okuma
    if (IO_Read(IOP_SPI_MISO))
      val |= 1;

    // 2b) Clock incative
    if (!_CPHA) {
      IO_Write(IOP_SPI_SCK, _CPOL);
      SSPI_Delay();
    }
  }
          
  return val;
}
#endif

////////////////////////////////////////////////////////////////
// HARDWARE SPI

static SPI_TypeDef *_pSPI[] = {
  SPI1,
  SPI2
};

void HSPI_Start(SPI_t nSPI, int mode)
{
  SPI_TypeDef *pSPI = _pSPI[nSPI];
  SPI_InitTypeDef spiInit;
  
  
  if (nSPI == SPI_1) {
    // 1) SPI clock aktif olacak
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
    // 2) SPI uçlarý yapýlandýrýlacak
    IO_Init(IOP_SPI1_SCK, IO_MODE_ALTERNATE);
    IO_Init(IOP_SPI1_MISO, IO_MODE_ALTERNATE);
    IO_Init(IOP_SPI1_MOSI, IO_MODE_ALTERNATE);
    
    // GPIO Mux ayarlarý
    //GPIO_PinAFConfig(GPIOA, 5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI1_SCK].port], _ios[IOP_SPI1_SCK].pin, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI1_MISO].port], _ios[IOP_SPI1_MISO].pin, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI1_MOSI].port], _ios[IOP_SPI1_MOSI].pin, GPIO_AF_SPI1);
  }
  else if (nSPI == SPI_2) {
    // 1) SPI clock aktif olacak
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // 2) SPI uçlarý yapýlandýrýlacak
    IO_Init(IOP_SPI2_SCK, IO_MODE_ALTERNATE);
    IO_Init(IOP_SPI2_MISO, IO_MODE_ALTERNATE);
    IO_Init(IOP_SPI2_MOSI, IO_MODE_ALTERNATE);

    // GPIO Mux ayarlarý
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI2_SCK].port], _ios[IOP_SPI2_SCK].pin, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI2_MISO].port], _ios[IOP_SPI2_MISO].pin, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIO_Ports[_ios[IOP_SPI2_MOSI].port], _ios[IOP_SPI2_MOSI].pin, GPIO_AF_SPI2);
  }
  
  // 3) SPI parametreleri yapýlandýrýlacak
  spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  spiInit.SPI_CPOL = (mode & 2) ? SPI_CPOL_High : SPI_CPOL_Low; 
  spiInit.SPI_CPHA = (mode & 1) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
  spiInit.SPI_DataSize = SPI_DataSize_8b;
  spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInit.SPI_Mode = SPI_Mode_Master;
  spiInit.SPI_NSS = SPI_NSS_Soft;
  
  SPI_Init(pSPI, &spiInit);
  
  // 4) SPI çevresel birimi baþlatýlacak
  SPI_Cmd(pSPI, ENABLE);
}

unsigned char HSPI_Data(SPI_t nSPI, unsigned char val)
{
  SPI_TypeDef *pSPI = _pSPI[nSPI];
  
  while (!SPI_I2S_GetFlagStatus(pSPI, SPI_I2S_FLAG_TXE)) ;
  
  SPI_I2S_SendData(pSPI, val);

  while (!SPI_I2S_GetFlagStatus(pSPI, SPI_I2S_FLAG_RXNE)) ;
  
  val = SPI_I2S_ReceiveData(pSPI);
  return val;
}


