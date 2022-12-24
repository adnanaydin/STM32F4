#ifndef __RTCC_H
#define __RTCC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char day;    // 1..7 1=Pazartesi, 7=Pazar
  unsigned char date;   // 1..31
  unsigned char mon;    // 1..12
  unsigned char year;   // 0..99 20xx
} TIME_t;
  

#define RTCC_ALR_SEC    0x01
#define RTCC_ALR_MIN    0x02
#define RTCC_ALR_HOUR   0x04
#define RTCC_ALR_TIME   0x07
#define RTCC_ALR_DAY    0x0F    // 0x08 | TIME
#define RTCC_ALR_DATE   0x17    // 0x10 | TIME

void RTCC_Init(void);
void RTCC_GetTime(TIME_t *pTime);
void RTCC_SetTime(TIME_t *pTime);
void RTCC_SetAlarm(TIME_t *pTime, int mask);

extern volatile int g_AlarmA, g_AlarmB;


#ifdef __cplusplus
}
#endif

#endif
