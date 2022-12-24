#ifndef __MODBUS_H
#define __MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif

void MB_UartInit(int baud);
void MB_SendData(const void *buf, int len);
void MB_Init(void);

void Task_MB(void);


extern volatile int    g_TxFlag;

#ifdef __cplusplus
}
#endif

#endif
