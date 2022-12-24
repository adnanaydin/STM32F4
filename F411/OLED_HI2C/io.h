#ifndef __IO_H
#define __IO_H

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

// IO modlarý
enum {
  IO_MODE_INPUT,
  IO_MODE_OUTPUT,
  IO_MODE_ALTERNATE,
  IO_MODE_ANALOG,
  
  IO_MODE_INPUT_PD,
  IO_MODE_INPUT_PU,
  IO_MODE_OUTPUT_OD,
  IO_MODE_ALTERNATE_OD,
  IO_MODE_ALTERNATE_OD_PU,
};

// IO Port indeksleri
enum {
  IO_PORT_A,
  IO_PORT_B,
  IO_PORT_C,
  IO_PORT_D,
  IO_PORT_E,
  IO_PORT_F,
  IO_PORT_G,
  IO_PORT_H,
  IO_PORT_I,
  IO_PORT_J,
  IO_PORT_K,
};

// port<pin> yapýsý
typedef struct {
  int port;
  int pin;
} IO_PIN;

// Button kontrol yapýsý
typedef struct {
  int   idx;    // Input pin index
  int   cState; // Current state 0 : 1
  int   aState; // Active state  0 : 1
  int   dbc;    // Debounce counter
} BT_PIN;

//////////////////////////////////////////////////

void IO_Init(int idx, int mode);
void IO_Write(int idx, int val);
int IO_Read(int idx);

void IO_ScanButtons(void);

//////////////////////////////////////////////////

//#define IO_OLED_VDD
//#define IO_OLED_GND

//////////////////////////////////////////////////

enum {
  IOP_LED,
  IOP_KEY,
  
  // I2C
  IOP_I2C1_SCL,
  IOP_I2C1_SDA,
  IOP_I2C2_SCL,
  IOP_I2C2_SDA,
  IOP_I2C3_SCL,
  IOP_I2C3_SDA,

  // OLED 
#ifdef IO_OLED_VDD  
  IOP_OLED_VDD,
#endif
#ifdef IO_OLED_GND
  IOP_OLED_GND,
#endif  

};

#ifdef _IOS_
IO_PIN _ios[] = {
  IO_PORT_C, 13,        // LED
  IO_PORT_A, 0,         // KEY
  
  // I2C
  IO_PORT_B, 6,
  IO_PORT_B, 7,
  IO_PORT_B, 10,
  IO_PORT_B, 3,
  IO_PORT_A, 8,
  IO_PORT_B, 4,
  
  // OLED 
#ifdef IO_OLED_VDD 
  //IO_PORT_B, 1,
  IO_PORT_B, 5,
#endif  
#ifdef IO_OLED_GND  
  //IO_PORT_B, 0,
  IO_PORT_B, 4,
#endif

};

static BT_PIN _bts[] = {
  IOP_KEY, 0, 1, 0,
};

#define N_BUTTONS  (sizeof(_bts) / sizeof(BT_PIN))
#endif

extern unsigned char g_Buttons[];

extern GPIO_TypeDef *GPIO_Ports[];
extern IO_PIN _ios[];

#endif
