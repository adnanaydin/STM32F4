#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "io.h"
#include "system.h"
#include "rtcc.h"

void _RTCC_Init(void)
{
#define RTC_SIGN        0x1EF3B79A
  
  RTC_InitTypeDef rtcInit;
  RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;
  
  // PWR modül clock iþaretini aktive ediyoruz
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  // Backup domain eriþimi
  PWR_BackupAccessCmd(ENABLE);
  
  // Eðer LSE çalýþýyorsa saat önceden ayarlanmýþtýr,
  // bu durumda yapacak iþimiz kalmadý
    
  if (RTC_ReadBackupRegister(RTC_BKP_DR0) == RTC_SIGN)
    return;
  
  ///////////////////////////////////////////////////////
  
  // Reset Backup domain 
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
  
  // LSE'yi aktive edeceðiz
  RCC_LSEConfig(RCC_LSE_ON);
  
  // LSE stabil çalýþana kadar bekle
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) ;
  
  // RTC clock kaynaðý olarak LSE'yi seçiyoruz
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  
  // RTC clk freq (1 Hz olmalý) = fRTC / ((APD +1) * (SPD + 1))
  rtcInit.RTC_AsynchPrediv = 0x7F;
  rtcInit.RTC_SynchPrediv = 0xFF;
  rtcInit.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&rtcInit);
  
  // Enable RTC clock
  RCC_RTCCLKCmd(ENABLE);
  
  RTC_WaitForSynchro();
  
  rtcDate.RTC_Date = 1;
  rtcDate.RTC_Month = 1;
  rtcDate.RTC_Year = 0;
  rtcDate.RTC_WeekDay = 6;
  RTC_SetDate(RTC_Format_BIN, &rtcDate);
  
  rtcTime.RTC_H12 = RTC_H12_AM;
  rtcTime.RTC_Hours = 0;
  rtcTime.RTC_Minutes = 0;
  rtcTime.RTC_Seconds = 0;
  RTC_SetTime(RTC_Format_BIN, &rtcTime);
  
  RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_SIGN);
}

void RTCC_Init(void)
{
  RTC_InitTypeDef rtcInit;
  RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;
  
  // PWR modül clock iþaretini aktive ediyoruz
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  // Backup domain eriþimi
  PWR_BackupAccessCmd(ENABLE);
  
  // Eðer LSE çalýþýyorsa saat önceden ayarlanmýþtýr,
  // bu durumda yapacak iþimiz kalmadý
  if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == SET)
    return;
  
  ///////////////////////////////////////////////////////
  
  // Reset Backup domain 
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
  
  // LSE'yi aktive edeceðiz
  RCC_LSEConfig(RCC_LSE_ON);
  
  // LSE stabil çalýþana kadar bekle
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) ;
  
  // RTC clock kaynaðý olarak LSE'yi seçiyoruz
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  
  // RTC clk freq (1 Hz olmalý) = fRTC / ((APD +1) * (SPD + 1))
  rtcInit.RTC_AsynchPrediv = 0x7F;
  rtcInit.RTC_SynchPrediv = 0xFF;
  rtcInit.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&rtcInit);
  
  // Enable RTC clock
  RCC_RTCCLKCmd(ENABLE);
  
  RTC_WaitForSynchro();
  
  rtcDate.RTC_Date = 1;
  rtcDate.RTC_Month = 1;
  rtcDate.RTC_Year = 0;
  rtcDate.RTC_WeekDay = 6;
  RTC_SetDate(RTC_Format_BIN, &rtcDate);
  
  rtcTime.RTC_H12 = RTC_H12_AM;
  rtcTime.RTC_Hours = 0;
  rtcTime.RTC_Minutes = 0;
  rtcTime.RTC_Seconds = 0;
  RTC_SetTime(RTC_Format_BIN, &rtcTime);
}

void RTCC_GetTime(TIME_t *pTime)
{
  RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;
  
  RTC_GetTime(RTC_Format_BIN, &rtcTime);
  RTC_GetDate(RTC_Format_BIN, &rtcDate);
  
  pTime->date = rtcDate.RTC_Date;
  pTime->day = rtcDate.RTC_WeekDay;
  pTime->mon = rtcDate.RTC_Month;
  pTime->year = rtcDate.RTC_Year;
  
  pTime->hour = rtcTime.RTC_Hours;
  pTime->min = rtcTime.RTC_Minutes;
  pTime->sec = rtcTime.RTC_Seconds;
}

void RTCC_SetTime(TIME_t *pTime)
{
  RTC_TimeTypeDef rtcTime;
  RTC_DateTypeDef rtcDate;

  rtcDate.RTC_Date = pTime->date;
  rtcDate.RTC_WeekDay = pTime->day;
  rtcDate.RTC_Month = pTime->mon;
  rtcDate.RTC_Year = pTime->year;
  
  rtcTime.RTC_Hours = pTime->hour;
  rtcTime.RTC_Minutes = pTime->min;
  rtcTime.RTC_Seconds = pTime->sec;
  
  RTC_WaitForSynchro();

  RTC_SetDate(RTC_Format_BIN, &rtcDate);
  RTC_SetTime(RTC_Format_BIN, &rtcTime);
}

void RTCC_SetAlarm(TIME_t *pTime, int mask)
{
  EXTI_InitTypeDef eInit;
  RTC_AlarmTypeDef rtcAlarm;
  
  RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
  
  // Alarm A tüm maske deðerleri set edilecek
  // Yani tüm alarm koþullarý kapalý
  rtcAlarm.RTC_AlarmMask = RTC_AlarmMask_All;
  
  if (mask & RTCC_ALR_SEC)
    rtcAlarm.RTC_AlarmMask &= ~RTC_AlarmMask_Seconds;

  if (mask & RTCC_ALR_MIN)
    rtcAlarm.RTC_AlarmMask &= ~RTC_AlarmMask_Minutes;

  if (mask & RTCC_ALR_HOUR)
    rtcAlarm.RTC_AlarmMask &= ~RTC_AlarmMask_Hours;
  
  if (mask == RTCC_ALR_DAY) {
    rtcAlarm.RTC_AlarmMask &= ~RTC_AlarmMask_DateWeekDay;
    rtcAlarm.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay;
    rtcAlarm.RTC_AlarmDateWeekDay = pTime->day;
  }
  else if (mask == RTCC_ALR_DATE) {
    rtcAlarm.RTC_AlarmMask &= ~RTC_AlarmMask_DateWeekDay;
    rtcAlarm.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    rtcAlarm.RTC_AlarmDateWeekDay = pTime->date;
  }
  
  rtcAlarm.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
  rtcAlarm.RTC_AlarmTime.RTC_Hours = pTime->hour;
  rtcAlarm.RTC_AlarmTime.RTC_Minutes = pTime->min;
  rtcAlarm.RTC_AlarmTime.RTC_Seconds = pTime->sec;
  
  RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A,  &rtcAlarm);
  
  ////////////////////////////////////////////////////////////////
  // RTC interrupt ayarlarý
  EXTI_ClearITPendingBit(EXTI_Line17);
  
  eInit.EXTI_Line = EXTI_Line17;
  eInit.EXTI_LineCmd = ENABLE;
  eInit.EXTI_Mode = EXTI_Mode_Interrupt;
  eInit.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&eInit);
  
  RTC_ClearITPendingBit(RTC_IT_ALRA);
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
  
  NVIC_SetPriority(RTC_Alarm_IRQn, 0);
  NVIC_EnableIRQ(RTC_Alarm_IRQn);
  
  RTC_ClearFlag(RTC_FLAG_ALRAF);
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}

//////////////////////////////////////////////////////////////////

volatile int g_AlarmA = 0, g_AlarmB = 0;

void RTC_Alarm_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_ALRA)) {
    g_AlarmA = 1;
    
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }

  if (RTC_GetITStatus(RTC_IT_ALRB)) {
    g_AlarmB = 1;
    
    RTC_ClearITPendingBit(RTC_IT_ALRB);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }
}