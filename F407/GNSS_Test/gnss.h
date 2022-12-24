#ifndef __GNSS_H
#define __GNSS_H

#ifdef __cplusplus
extern "C" {
#endif

#define SZ_BUF  100


void GNSS_Init(int baud);
int GNSS_GetLine(unsigned char *LineBuf);

void Task_GNSS(void);

#ifdef __cplusplus
}
#endif

#endif
