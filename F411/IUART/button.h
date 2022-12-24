#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  IO_IDX   ioIdx;  // input pin index
  int   cState; // current state (kompanze edilmiþ iþaretin durum)
  int   aState; // active state
  int   dbc;    // debounce counter
} BTN_PIN;

enum {
  BTN_SET,
  BTN_UP,
  BTN_DN,
};

void BTN_Init(void);
void BTN_ScanButtons(void);

extern volatile int g_sButtons[];  // Button semafor iþareti

#ifdef __cplusplus
}
#endif

#endif
