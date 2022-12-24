#ifndef _HI2C_H
#define _HI2C_H

#define I2C_WRITE       0
#define I2C_READ        1

enum {
  I2C_1,
  I2C_2,
  I2C_3,
};

void HI2C_Init(int nI2C, int bRate);
int HI2C_Start(int nI2C, unsigned char ctl);
void HI2C_Stop(int nI2C);

int HI2C_Write(int nI2C, unsigned char devAdr, const void *buf, int len);
int HI2C_Read(int nI2C, unsigned char devAdr, void *buf, int len);

int HI2C_WriteA(int nI2C, unsigned char devAdr, unsigned char regAdr, const void *buf, int len);
int HI2C_ReadA(int nI2C, unsigned char devAdr, unsigned char regAdr, void *buf, int len);

#endif
