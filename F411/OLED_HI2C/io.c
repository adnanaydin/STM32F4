#define _IOS_
#include "io.h"

// GPIO portlarý dizisi
// Her eleman bir struct göstericisi
GPIO_TypeDef *GPIO_Ports[] = {
  GPIOA,
  GPIOB,
  GPIOC,
  GPIOD,
  GPIOE,
  GPIOF,
  GPIOG,
  GPIOH,
  GPIOI,
  GPIOJ,
  GPIOK,
};

////////////////////////////////////////////////////

void IO_Init(int idx, int mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  switch (mode) {
  case IO_MODE_INPUT:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_INPUT_PD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;
    
  case IO_MODE_INPUT_PU:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_OUTPUT:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_OUTPUT_OD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    break;

  case IO_MODE_ALTERNATE:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;

  case IO_MODE_ALTERNATE_OD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    break;

  case IO_MODE_ALTERNATE_OD_PU:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    break;

  case IO_MODE_ANALOG:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;  
  }
  
  GPIO_InitStructure.GPIO_Pin = (1 << _ios[idx].pin);
  GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
  
  GPIO_Init(GPIO_Ports[_ios[idx].port], &GPIO_InitStructure);  
}

void IO_Write(int idx, int val)
{
  if (val)
    GPIO_Ports[_ios[idx].port]->ODR |= (1 << _ios[idx].pin);
  else
    GPIO_Ports[_ios[idx].port]->ODR &= ~(1 << _ios[idx].pin);
}

int IO_Read(int idx)
{
  GPIO_TypeDef *port = GPIO_Ports[_ios[idx].port];
  
  return (port->IDR & (1 << _ios[idx].pin)) != 0;
}


///////////////////////////////////////////////
// PUSH BUTTON FONKSÝYONLARI

int g_dbcMax = 50;
unsigned char g_Buttons[N_BUTTONS] = { IOP_KEY };

void IO_Scan(int btIdx)
{
  int r;
  
  r = IO_Read(_bts[btIdx].idx);
  
  if (r != _bts[btIdx].cState) {
    if (++_bts[btIdx].dbc >= g_dbcMax) {
      _bts[btIdx].cState = r;
      _bts[btIdx].dbc = 0;
      
      if (_bts[btIdx].cState == _bts[btIdx].aState) {
        // Sinyal
        //g_Buttons[btIdx] = 1; // Binary semaphore
        ++g_Buttons[btIdx];     // Counting semaphore
      }
    }
  }
  else 
    _bts[btIdx].dbc = 0;
}

void IO_ScanButtons(void)
{
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    IO_Scan(i);
}










