#define _IOS_
  #include "io.h"

void IO_Init(IO_IDX idx, IO_MODE mode)
{
  int i;
  GPIO_InitTypeDef ioInit;

  switch (mode) {
  case IO_MODE_INPUT:
    ioInit.GPIO_Mode = GPIO_Mode_IN;
    ioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_OUTPUT:
    ioInit.GPIO_Mode = GPIO_Mode_OUT;
    ioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_ALTERNATE:
    ioInit.GPIO_Mode = GPIO_Mode_AF;
    ioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;
    
  case IO_MODE_ANALOG:
    ioInit.GPIO_Mode = GPIO_Mode_AN;
    ioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;
  
  case IO_MODE_INPUT_PD:
    ioInit.GPIO_Mode = GPIO_Mode_IN;
    ioInit.GPIO_PuPd = GPIO_PuPd_DOWN;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_INPUT_PU:
    ioInit.GPIO_Mode = GPIO_Mode_IN;
    ioInit.GPIO_PuPd = GPIO_PuPd_UP;
    ioInit.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_OUTPUT_OD:
    ioInit.GPIO_Mode = GPIO_Mode_OUT;
    ioInit.GPIO_PuPd = GPIO_PuPd_UP;
    ioInit.GPIO_OType = GPIO_OType_OD;
    break;

  case IO_MODE_ALTERNATE_OD:
    ioInit.GPIO_Mode = GPIO_Mode_AF;
    ioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    ioInit.GPIO_OType = GPIO_OType_OD;
    break;
  }
  
  ioInit.GPIO_Pin = (1 << _ios[idx].pin); 
  ioInit.GPIO_Speed = GPIO_High_Speed;
  
  i = _ios[idx].port;
  GPIO_Init(GPIO_Ports[i], &ioInit);
}

void IO_Write(IO_IDX idx, int val)
{
  GPIO_TypeDef *GPIOx;
  int i, mask;
  
  i = _ios[idx].port;
  GPIOx = GPIO_Ports[i];
  
  mask = (1 << _ios[idx].pin);
  if (val)
    GPIOx->BSRR = mask;
  else
    GPIOx->BSRR = (mask << 16);    
}

int IO_Read(IO_IDX idx)
{
  GPIO_TypeDef *GPIOx;
  int i, mask;
  
  i = _ios[idx].port;
  GPIOx = GPIO_Ports[i];
  
  mask = (1 << _ios[idx].pin);
  return (GPIOx->IDR & mask) != 0;
}



