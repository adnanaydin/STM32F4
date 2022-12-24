#ifndef __RESET_H
#define __RESET_H

#ifdef __cplusplus
extern "C" {
#endif

void Task_Reset(void);
void IWDG_Init(uint16_t period);
void WWDG_Init(uint8_t winVal);

#ifdef __cplusplus
}
#endif

#endif
