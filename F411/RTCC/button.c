#include "io.h"
#include "system.h"
#include "button.h"

static BTN_PIN _bts[] = {
  { IOP_KEY, 0, 1, 0 },
  { IOP_BTN, 1, 0, 0 },
};

#define N_BUTTONS (sizeof(_bts) / sizeof(BTN_PIN))

int g_dbcMax = 5;       // Max debounce sayýsý
static int _bScan = 0;  // Button'lar baþlatýldý mý?

volatile int g_sButtons[N_BUTTONS] = { 0 };  // Button semafor iþareti

void BTN_Init(void)
{
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    IO_Init(_bts[i].ioIdx, _bts[i].aState ? IO_MODE_INPUT_PD : IO_MODE_INPUT_PU);
  
  _bScan = 1;
}

// Tek button tarama
static void BTN_Scan(int idx)
{
  int r; // anlýk okuma deðeri
  
  r = IO_Read(_bts[idx].ioIdx);
              
  if (r != _bts[idx].cState) {
    if (++_bts[idx].dbc >= g_dbcMax) {
      _bts[idx].cState = r;
      _bts[idx].dbc = 0;
      
      if (_bts[idx].cState == _bts[idx].aState) {
        // signal
        //g_sButtons[idx] = 1;    // Binary semaphore
        ++g_sButtons[idx];        // Counting semaphore
      }
    }
  }
  else {
    _bts[idx].dbc = 0;
  }
}

// Tüm button'lar tarama
void BTN_ScanButtons(void)
{
  int i;
  
  if (!_bScan)
    return;
  
  for (i = 0; i < N_BUTTONS; ++i)
    BTN_Scan(i);
}

