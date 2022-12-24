#ifndef __1_WIRE_H
#define __1_WIRE_H

#ifdef __cplusplus
extern "C" {
#endif

#define TMP_INVALID     (-32768)
  
void OW_Init(void);  
int OW_Reset(void);
unsigned char OW_Byte(unsigned char val);

int OW_ReadROM(void *buf);

#define OW_SetByte(n)   OW_Byte(n)
#define OW_GetByte()    OW_Byte(0xFF)

int DS_ReadTemp(int *pTemp);
int Task_Thermo(void);

#ifdef __cplusplus
}
#endif

#endif
