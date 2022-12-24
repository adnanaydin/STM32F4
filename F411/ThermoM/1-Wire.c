#include <stdio.h>

#include "io.h"
#include "system.h"
#include "1-Wire.h"
#include "crc8.h"
#include "oled.h"

enum { FALSE, TRUE };

void OW_Init(void)
{
  // DQ hattý open drain
  IO_Write(IOP_DQ, 1);
  IO_Init(IOP_DQ, IO_MODE_OUTPUT_OD);
}

// 1-Wire bus'a reset pulse gönderir
// Geri dönüþ TRUE ise presence pulse 
// alýnmýþ demektir
int OW_Reset(void)
{
  int tmOut;
  //int is;
  
  // Enter criticak section
  // Seçenek 1) CMSIS
  //is = __get_PRIMASK();
  //__disable_irq();
  
  // Seçenek 2) ASM Library   
  // is = __ints(0);
  
  // Seçenek 3) C Library
  Sys_EnterCritical();
  /////////////////////////////////////////////
 
  IO_Write(IOP_DQ, 0);  // DQ -> Çýkýþ-0
  
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
  // Seçenek 1)
  /*
  if (is)
    __enable_irq();  
  */
  //__set_PRIMASK(is);
  
  // Seçenek 2)
  //__ints(is);
  
  // Seçenek 3)
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

int OW_SearchROM(unsigned char *data, int totDev)
{
  int i, j, k, ocf, ncf, nDev;
  int b0, b1, b2;

  nDev = 0;
  ocf = -1;

  do {
    if (!OW_Reset())
        break;

    OW_SetByte(0xF0);

    ncf = -1;
    for (i = 0; i < 64; ++i) {
      j = i >> 3;
      k = i & 7;
      if (ocf != -1)
        b0 = (data[j - 8] & (1 << k)) != 0;

      b1 = OW_Bit(1);
      b2 = OW_Bit(1);

      if (b1 == 1 && b2 == 1) {
        nDev = 0;
        break;
      }

      if (b1 == 0 && b2 == 0) {
        if (ocf > i)
          b1 = b0;
        else if (ocf == i)
          b1 = 1;
        else 
          b1 = 0;

        if (b1 == 0 && i > ncf)
          ncf = i;
      }

      OW_Bit(b1);
      if (b1)
        data[j] |= (1 << k);
      else
        data[j] &= ~(1 << k);
    }

    if (CRC8(data, 8)) {
      nDev = 0;
      break;
    }
    else {
      if (++nDev >= totDev)
        break;

      data += 8;
      ocf = ncf;
    }
  } while (ncf != -1);

  return nDev;
}

/////////////////////////////////////////////////////////////////////////

// Geri dönüþ TRUE/FALSE
// *pTemp: Sýcaklýk deðerinin 16 katý
int DS_ReadTemp(int *pTemp)
{
  unsigned char i, sp[9];
  int tmOut;
  
  ////////////////////////////////////////
  // 1. Cümle: Sýcaklýðý hesaplama
  
  // Reset/Presence
  if (!OW_Reset())
    return FALSE;
  
  // ROM komutu
  OW_SetByte(0xCC);     // Skip ROM
  
  // Fonksiyon komutu
  OW_SetByte(0x44);     // ConvertT komutu
  
  // 1s boyunca iþlemin bitmesini sorgulama ile bekleyeceðiz
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
  // 1. Cümle: Sýcaklýðý hesaplama
  
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
#define TM_RESET        100
#define TM_POWER        10

// Sýcaklýk hesaplandýðýnda sýcaklýk * 16 deðerine
// geri döner. Hesaplanmýþ sýcaklýk yoksa TMP_INVALID döner
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

#define NMAXDEV 20
unsigned char _romTab[NMAXDEV * 8];

void Task_ThermoM(void)
{
  static enum {
    S_INIT,
    S_NEXT,
    S_RESET_1,
    S_WAIT_1,
    S_CONVERT,
    S_WAIT_C,
    S_RESET_2,
    S_READ_CMD,
    S_READ,
    S_CRC,
  } state = S_INIT;
  
  static clock_t t0, t1;
  static int i, j, temp, nDevs, oDevs;
  static unsigned char sp[9];
  
  t1 = clock();
  
  switch (state) {
  case S_INIT:
    nDevs = OW_SearchROM(_romTab, NMAXDEV);
    
    if (nDevs != oDevs) {
      oDevs = nDevs;
      OLED_ClearDisplay();
    }
    
    j = -1;
    state = S_NEXT;
    break;
    
  case S_NEXT:
    if (++j == nDevs) {
      state = S_INIT;
      break;
    }
      
    t0 = t1;
    state = S_RESET_1;
    //break;
    
  case S_RESET_1:
    if (OW_Reset()) 
      state = S_CONVERT;
    else {
      t0 = t1;
      state = S_WAIT_1;
    }
    break;
    
  case S_WAIT_1:  
    if (t1 - t0 >= TM_RESET) 
      state = S_RESET_1;
    break;
    
  case S_CONVERT:
    // ROM komutu
    OW_SetByte(0x55);     // Match ROM komutu
  
    for (i = 0; i < 8; ++i)
      OW_SetByte(_romTab[8 * j + i]);
      
    // Fonksiyon komutu
    OW_SetByte(0x44);     // ConvertT komutu
    
    t0 = t1;
    state = S_WAIT_C;
    //break;
    
  case S_WAIT_C:
    if (t1 - t0 >= TM_CONVERT) 
      state = S_RESET_1;
    else if (IO_Read(IOP_DQ)) 
      state = S_RESET_2;
    break;
    
  case S_RESET_2:
    if (OW_Reset()) 
        state = S_READ_CMD;
    else 
        state = S_NEXT;
    break;
    
  case S_READ_CMD:
    // ROM komutu
    OW_SetByte(0x55);     // Match ROM komutu
  
    for (i = 0; i < 8; ++i)
      OW_SetByte(_romTab[8 * j + i]);
      
    // Fonksiyon komutu
    OW_SetByte(0xBE);     // Read Scratchpad komutu
    
    i = 0;
    state = S_READ;
    break;
    
  case S_READ:    
    sp[i++] = OW_GetByte();
    sp[i++] = OW_GetByte();
    sp[i++] = OW_GetByte();
    
    if (i >= 9)
      state = S_CRC;
    break;
    
  case S_CRC:
    if (!CRC8(sp, 9)) {
      temp = (int)(*(int16_t *)sp);
      
      OLED_SetCursor(j, 0);
      for (i = 7; i >= 0; --i)
        printf("%02X", _romTab[j * 8 + i]);
      
      printf("%5.1f", (float)temp / 16);
    }
    
    state = S_NEXT;
    break;
  }
}


