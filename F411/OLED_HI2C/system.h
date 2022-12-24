#ifndef __SYSTEM_H
#define __SYSTEM_H


#define CLOCKS_PER_SEC          1000

typedef unsigned long   clock_t;

void Sys_IoInit(void);
void Sys_ClockInit(void);
void Sys_ConsoleInit(void);
void Sys_IntEnable(int bEnable);

clock_t clock(void);

void __delay(unsigned long ncy);
int __ints(int stat);

#define CLOCK_FREQ      SystemCoreClock
#define INSTR_FREQ      (CLOCK_FREQ * 4 / 3)
#define US_CYCLES       (INSTR_FREQ / 1000000)
#define MS_CYCLES       (INSTR_FREQ / 1000)

#define DelayUs(us)     __delay((us) * US_CYCLES)
#define DelayMs(ms)     __delay((ms) * MS_CYCLES)


#endif
