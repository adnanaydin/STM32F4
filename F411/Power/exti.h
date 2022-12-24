#ifndef __EXTI_H
#define __EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

enum {  
  I_RISING,
  I_FALLING,
  I_RISING_FALLING
};

void EXTI_Config(IO_IDX idx, int trigger, int priority);


#ifdef __cplusplus
}
#endif

#endif
