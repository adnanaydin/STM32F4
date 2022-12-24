#include "io.h"
#include "system.h"
#include "1-Wire.h"
#include "crc8.h"

enum { FALSE, TRUE };

void OW_Init(void)
{
  // DQ hatt� open drain
  IO_Write(IOP_DQ, 1);
  IO_Init(IOP_DQ, IO_MODE_OUTPUT_OD);
}

// 1-Wire bus'a reset pulse g�nderir
// Geri d�n�� TRUE ise presence pulse 
// al�nm�� demektir
int OW_Reset(void)
{
  int tmOut;
  //int is;
  
  // Enter criticak section
  // Se�enek 1) CMSIS
  //is = __get_PRIMASK();
  //__disable_irq();
  
  // Se�enek 2) ASM Library   
  // is = __ints(0);
  
  // Se�enek 3) C Library
  Sys_EnterCritical();
  /////////////////////////////////////////////
 
  IO_Write(IOP_DQ, 0);  // DQ -> ��k��-0
  
  DelayUs(500);
  
  IO_Write(IOP_DQ, 1);  // DQ -> Pull-up 1
  
  DelayUs(10);
  
  tmOut = 100;
  do {
    if (!IO_Read(IOP_DQ))
      break;
    
    DelayUs(1);
  } while (--tmOut > 0);
  
  if (tmOut <= 0)
    goto END;
  
  tmOut = 250;
  do {
    if (IO_Read(IOP_DQ))
      break;

    DelayUs(1);
  } while (--tmOut > 0);
  
END:  
  /////////////////////////////////////////////
  // Exit critical section
  // Se�enek 1)
  /*
  if (is)
    __enable_irq();  
  */
  //__set_PRIMASK(is);
  
  // Se�enek 2)
  //__ints(is);
  
  // Se�enek 3)
  Sys_ExitCritical();
  
  return (tmOut > 0);
}

int OW_Bit(int val)
{
  Sys_EnterCritical();
  
  IO_Write(IOP_DQ, 0);  // DQ=0 Clock
  
  DelayUs(1);
  
  if (val)
    IO_Write(IOP_DQ, 1);
  
  DelayUs(13);
  
  val = IO_Read(IOP_DQ);
  
  DelayUs(46);
  
  IO_Write(IOP_DQ, 1);
  
  DelayUs(1);

  Sys_ExitCritical();
  
  return val;
}

unsigned char OW_Byte(unsigned char val)
{
  int i, b;
  
  for (i = 0; i < 8; ++i) {
    b = OW_Bit(val & 1);
    
    val >>= 1;
    
    if (b)
      val |= 0x80;
  }
  
  return val;
}

int OW_ReadROM(void *buf)
{
  int i, stat;
  unsigned char *ptr = (unsigned char *)buf;
  
  stat = OW_Reset();
  if (!stat)
    return FALSE;
  
  OW_SetByte(0x33);     // Read ROM komutu
  
  for (i = 0; i < 8; ++i)
    *ptr++ = OW_GetByte();
  
  OW_Reset();
  
  //return (CRC8(buf, 8) == 0);
  return !CRC8(buf, 8);
}

/////////////////////////////////////////////////////////////////////////

// Geri d�n�� TRUE/FALSE
// *pTemp: S�cakl�k de�erinin 16 kat�
int DS_ReadTemp(int *pTemp)
{
  unsigned char i, sp[9];
  int tmOut;
  
  ////////////////////////////////////////
  // 1. C�mle: S�cakl��� hesaplama
  
  // Reset/Presence
  if (!OW_Reset())
    return FALSE;
  
  // ROM komutu
  OW_SetByte(0xCC);     // Skip ROM
  
  // Fonksiyon komutu
  OW_SetByte(0x44);     // ConvertT komutu
  
  // 1s boyunca i�lemin bitmesini sorgulama ile bekleyece�iz
  tmOut = 1000;
  do {
    if (IO_Read(IOP_DQ))
      break;
    
    DelayMs(1);
  } while (--tmOut);
  
  if (!tmOut) {
    OW_Reset();
    return FALSE;
  }
    
  ////////////////////////////////////////
  // 1. C�mle: S�cakl��� hesaplama
  
  if (!OW_Reset())
    return FALSE;
  
  // ROM komutu
  OW_SetByte(0xCC);     // Skip ROM
  
  // Fonksiyon komutu
  OW_SetByte(0xBE);     // Read SP komutu
  
  for (i = 0; i < 9; ++i)
    sp[i] = OW_GetByte();
  
  OW_Reset();
  
  if (CRC8(sp, 9))
    return FALSE;
  
  *pTemp = (int)(*(int16_t *)sp);
  return TRUE;
}

/////////////////////////////////////////////////////////////

#define TM_CONVERT      1000

// S�cakl�k hesapland���nda s�cakl�k * 16 de�erine
// geri d�ner. Hesaplanm�� s�cakl�k yoksa TMP_INVALID d�ner
int Task_Thermo(void)
{
  static enum {
    S_INIT,
    S_RESET_1,
    S_CONVERT,
    S_WAIT,
    S_RESET_2,
    S_FCMD,
    S_READSP,
    S_CRC,
  } state = S_INIT;
  
  static clock_t t0, t1;
  static int temp;
  static unsigned char i, sp[9];
  
  t1 = clock();
  
  switch (state) {
  case S_INIT:
    temp = TMP_INVALID;
    state = S_RESET_1;
    break;
    
  case S_RESET_1:
    if (OW_Reset())
      state = S_CONVERT;
    break;
    
  case S_CONVERT:
    OW_SetByte(0xCC);   // Skip ROM
    
    OW_SetByte(0x44);   // ConvertT
    
    t0 = t1;
    state = S_WAIT;
    break;
    
  case S_WAIT:
    if (IO_Read(IOP_DQ))
      state = S_RESET_2;
    else if (t1 - t0 > TM_CONVERT)
      state = S_INIT;
    break;
    
  case S_RESET_2:
    if (OW_Reset()) 
      state = S_FCMD;    
    break;
    
  case S_FCMD:
    OW_SetByte(0xCC);   // Skip ROM
    OW_SetByte(0xBE); // ReadSP      
    
    i = 0;
    state = S_READSP;
    break;

  case S_READSP:
    sp[i++] = OW_GetByte();
       
    if (i >= 9)
      state = S_CRC;
    break;
      
  case S_CRC:
    if (!CRC8(sp, 9))
      temp = (int)(*(int16_t *)sp);
       
    state = S_INIT;
    break;
  }
  
  return temp;
}


