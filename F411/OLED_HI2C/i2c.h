#ifndef _I2C_H
#define _I2C_H

enum {
  I2C_1,
  I2C_2,
  I2C_3,
};

#define I2C_WRITE       0
#define I2C_READ        1

void HI2C_Init(int nI2C, int bps);
int HI2C_Write(int nI2C, unsigned char devAdr, const void *buf, int len);

int HI2C_WriteA(int nI2C, unsigned char devAdr, unsigned char regAdr,
                const void *buf, int len);
int HI2C_ReadA(int nI2C, unsigned char devAdr, unsigned char regAdr,
                void *buf, int len);

#endif