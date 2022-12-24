#ifndef __SPI_H
#define __SPI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SPI_1,
  SPI_2,
} SPI_t;

void SSPI_Start(int mode);
unsigned char SSPI_Data(unsigned char val);

void HSPI_Start(SPI_t nSPI, int mode);
unsigned char HSPI_Data(SPI_t nSPI, unsigned char val);


#ifdef __cplusplus
}
#endif

#endif
