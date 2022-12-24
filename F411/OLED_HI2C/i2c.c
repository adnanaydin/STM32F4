#include <stdio.h>
#include <string.h>

#include "system.h"
#include "io.h"
#include "i2c.h"

static I2C_TypeDef *_i2c[] = {
  I2C1, 
  I2C2, 
  I2C3
};

static int _i2cRCC[] = {
  RCC_APB1Periph_I2C1,
  RCC_APB1Periph_I2C2,
  RCC_APB1Periph_I2C3,
};

int _i2cSCL[] = {
  IOP_I2C1_SCL,
  IOP_I2C2_SCL,
  IOP_I2C3_SCL
};

int _i2cSDA[] = {
  IOP_I2C1_SDA,
  IOP_I2C2_SDA,
  IOP_I2C3_SDA
};

int _i2cAF_SCL[] = {
  GPIO_AF_I2C1,
  GPIO_AF_I2C2,
  GPIO_AF_I2C3,
};

int _i2cAF_SDA[] = {
  GPIO_AF_I2C1,
  GPIO_AF9_I2C2,
  GPIO_AF9_I2C3,
};

enum { FALSE, TRUE };

// Hardware _i2c[nI2C] portu baþlangýç
void HI2C_Init(int nI2C, int bps)
{
  int i;
  
  I2C_InitTypeDef I2C_InitStruct;

  // 1) I2C SCL ve SDA uçlarý yapýlandýrmalarý
  i = _i2cSCL[nI2C];
  IO_Write(i, 1);
  IO_Init(i, IO_MODE_ALTERNATE_OD);  
  GPIO_PinAFConfig(GPIO_Ports[_ios[i].port],
    _ios[i].pin, _i2cAF_SCL[nI2C]);
  
  i = _i2cSDA[nI2C];
  IO_Write(i, 1);
  IO_Init(i, IO_MODE_ALTERNATE_OD);
  GPIO_PinAFConfig(GPIO_Ports[_ios[i].port],
    _ios[i].pin, _i2cAF_SDA[nI2C]);

  // 2) I2C Çevresel clock aktif olmalý
  RCC_APB1PeriphClockCmd(_i2cRCC[nI2C], ENABLE);
  
  // 3) I2C baþlangýç parametreleri belirlenecek
  I2C_InitStruct.I2C_ClockSpeed = bps;
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStruct.I2C_OwnAddress1 = 0x38; // Slave mode
  // Slave Device adsress size
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; // ???
  
  I2C_Init(_i2c[nI2C], &I2C_InitStruct); 
  
  // 4) I2C Çevresel baþlangýç
  I2C_Cmd(_i2c[nI2C], ENABLE);
}

#define I2C_TIMEOUT     10000

int HI2C_Event(int nI2C, int event)
{
  unsigned long timeout = I2C_TIMEOUT;
  
  do {
    if (I2C_CheckEvent(_i2c[nI2C], event) == SUCCESS)
      break;
  } while (--timeout);
  
  return (timeout != 0);
}

void HI2C_Stop(int nI2C)
{
  I2C_GenerateSTOP(_i2c[nI2C], ENABLE);
}

int _HI2C_Start(int nI2C, unsigned char ctl)
{
  I2C_GenerateSTART(_i2c[nI2C], ENABLE);
  
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_MODE_SELECT)) {
    //HI2C_Stop(nI2C);
    //I2C_SoftwareResetCmd(_i2c[nI2C], ENABLE); 
    return FALSE;
  }
  
  I2C_SendData(_i2c[nI2C], ctl);
  
  return HI2C_Event(nI2C,
    ctl & 1 ? 
      I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :
      I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
}

int HI2C_Start(int nI2C, unsigned char ctl)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  int stat, nTry;
  
  nTry = 5;
  do {
    I2C_GenerateSTART(pI2C, ENABLE);      // Start condition
    
    if (HI2C_Event(nI2C, I2C_EVENT_MASTER_MODE_SELECT)) {    
      I2C_SendData(pI2C, ctl);     // !!
      
      stat = HI2C_Event(nI2C, ctl & 1 ? 
        I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :
        I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
      
      if (stat)
        break;
    }
  } while (--nTry);
    
  return stat;
}

// Geri dönüþ: TRUE, FALSE
// Parametre: Gönderilece deðer
int HI2C_Send(int nI2C, unsigned char val)
{
  I2C_SendData(_i2c[nI2C], val);
  
  return HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
}

// Geri dönüþ: TRUE, FALSE
// Parametre okunan deðerin aktarýlacaðý adres
int HI2C_Recv(int nI2C, unsigned char *pVal)
{
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
    return FALSE;
  
  *pVal = I2C_ReceiveData(_i2c[nI2C]);
  
  return TRUE;
}

// ack: ack biti
void HI2C_Acknowledge(int nI2C, int ack)
{
  I2C_AcknowledgeConfig(_i2c[nI2C], 
    ack ? DISABLE : ENABLE);
}

////////////////////////////////////////////////////
// I2C Master -> Slave veri gönderme fonksiyonu
// devAdr: 7-bit slave device address + 0b
// buf: Yazýlacak verinin baþlangýç adresi
// len: Yazýlacak byte sayýsý
int HI2C_Write(int nI2C, unsigned char devAdr, const void *buf, int len)
{
    const unsigned char *ptr = (const unsigned char *)buf;
    
    if (!HI2C_Start(nI2C, devAdr)) 
      return FALSE;
    
    while (len) {
      if (!HI2C_Send(nI2C, *ptr++)) 
        break;
      
      --len;
    }
    
    HI2C_Stop(nI2C);   
    return (len == 0);
}

////////////////////////////////////////////////////
// I2C Master <- Slave'den veri alma fonksiyonu
// devAdr: 7-bit slave device address + 0b
// buf: Okunacak verinin yükleneceði adres
// len: Okunacak byte sayýsý
int HI2C_Read(int nI2C, unsigned char devAdr, void *buf, int len)
{
    unsigned char *ptr = (unsigned char *)buf;
    int ack;    // Master ack

    if (len <= 0)
      return FALSE;
    
    if (!HI2C_Start(nI2C, devAdr | 1)) {
ERR:      
      HI2C_Acknowledge(nI2C, 1);
      HI2C_Stop(nI2C);     
      return FALSE;
    }
    
    do {
      ack = (--len == 0);
      HI2C_Acknowledge(nI2C, ack);    // Ack config
      
      if (!HI2C_Recv(nI2C, ptr++))
        goto ERR;
    } while (len);
    
    HI2C_Acknowledge(nI2C, 1);
    HI2C_Stop(nI2C);   
    return TRUE;
}

////////////////////////////////////////////////////

#define SZ_I2CBUF       250

int HI2C_WriteA(int nI2C, unsigned char devAdr, unsigned char regAdr,
                const void *buf, int len)
{
  unsigned char i2cbuf[SZ_I2CBUF + 1];
  
  if (len > SZ_I2CBUF)
    return FALSE;
  
  i2cbuf[0] = regAdr;
  memcpy(i2cbuf + 1, buf, len);
  
  return HI2C_Write(nI2C, devAdr, i2cbuf, len + 1);
}

int HI2C_ReadA(int nI2C, unsigned char devAdr, unsigned char regAdr,
                void *buf, int len)
{
  if (len > SZ_I2CBUF)
    return FALSE;
  
  if (!HI2C_Write(nI2C, devAdr, &regAdr, 1))
    return FALSE;
  
  return HI2C_Read(nI2C, devAdr, buf, len);
}


