#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#undef  CLOCKS_PER_SEC
#define CLOCKS_PER_SEC  1000
  
#define _STDIN  0
#define _STDOUT 1
#define _STDERR 2
      
typedef uint32_t clock_t;

clock_t clock(void);

void __delay(unsigned long ncy);

#define CLOCK_FREQ      100000000
#define INSTR_FREQ      ((CLOCK_FREQ) * 8 / 9)
#define US_CYCLES       (INSTR_FREQ / 1000000)
#define MS_CYCLES       (INSTR_FREQ / 1000)

#define DelayUs(us)     __delay((us) * US_CYCLES)
#define DelayMs(ms)     __delay((ms) * MS_CYCLES)
  
void Sys_IoInit(void);
void Sys_TickInit(void);
void Sys_ConsoleInit(void);


#ifdef __cplusplus
}
#endif

#endif
