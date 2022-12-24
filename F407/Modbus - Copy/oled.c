#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "io.h"
#include "system.h"
#include "spi.h"
#include "oled.h"

#ifdef OL_FNT_SMALL
#include "font_5x8.h"
#endif
#ifdef OL_FNT_LARGE
#include "font_7x16.h"
#endif
#ifdef OL_FNT_BIG
#include "font_15x32.h"
#endif

#define NPGS    8
#define NSEG    128

static unsigned char _DspRam[NPGS * NSEG];
static unsigned char _page, _segment;

static int      _szWidth;       // Font geniþliði (pixel)
static int      _szHeight;      // Font yüksekliði (pixel)
static const unsigned char *_pFont;

static unsigned char    _row, _col;     // Text satýr, sütun
static FNT_t    _font;          // Aktif font

#define T_COLS  (NSEG / (_szWidth + 1))
#define T_ROWS  (NPGS / _szHeight)

static void OLED_Command(unsigned char cmd)
{
  IO_Write(IOP_OLED_DC, 0);     // DC=0 Command mode
  
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 0);     // CS=0
#endif  
  SPI_Data(cmd);
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);     // CS=1
#endif  
}

void OLED_Data(unsigned char data)
{
  IO_Write(IOP_OLED_DC, 1);     // DC=1 Data mode
  
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 0);     // CS=0
#endif  
  SPI_Data(data);
#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);     // CS=1
#endif  
  
  _DspRam[_page * NSEG + _segment] = data;
  
  if (++_segment >= NSEG)
    _segment = 0;
}

void OLED_Rotate(int bRotate)
{
  unsigned char remap, scan;
  
  if (bRotate) {
    remap = 0xA0;       // segment remap 0->127
    scan = 0xC0;          // com scan 0->63
  }
  else {
    remap = 0xA1;       // segment remap 127->0
    scan = 0xC8;          // com scan 63->0
  }
  
  OLED_Command(remap);
  OLED_Command(scan);
  
  OLED_UpdateDisplay();
}

void SSD1306_Init(int bRotate)
{
  OLED_Command(0xAE);   // Set display off
  
  OLED_Command(0xD5);   // Set display clock divide ratio
  OLED_Command(0x20);
  
  OLED_Command(0xA8);   // Set multiplex ratio
  OLED_Command(0x3F);
  
  OLED_Command(0xD3);   // Set display offset
  OLED_Command(0x00);
  
  OLED_Command(0x40);   // Set display start line
  
  OLED_Command(0x8D);   // Set charge pump
  OLED_Command(0x14);   // internal
  
  // Orientation
  memset(_DspRam, 0, NSEG * NPGS);
  OLED_Rotate(bRotate);
  
  OLED_Command(0xDA);   // Set COM hardware configuration
  OLED_Command(0x12);
  
  OLED_Command(0x81);   // Set contrast
  OLED_Command(0x80);
  
  OLED_Command(0xA4);   // Set all pixels off
  
  OLED_Command(0xA6);   // Set display not inverted

  //OLED_ClearDisplay();
  OLED_Command(0xAF);  // Set display on
}

void OLED_Start(int bRotate)
{
  // SPI çevresel birimini Mode 0 ile baþlatýyoruz
  SPI_Start(0); 
  
#ifdef IO_OLED_GND
  IO_Write(IOP_OLED_GND, 0);
  IO_Init(IOP_OLED_GND, IO_MODE_OUTPUT);
#endif  
#ifdef IO_OLED_VDD
  IO_Write(IOP_OLED_VDD, 1);
  IO_Init(IOP_OLED_VDD, IO_MODE_OUTPUT);
#endif  

#ifdef IO_OLED_CS
  IO_Write(IOP_OLED_CS, 1);
  IO_Init(IOP_OLED_CS, IO_MODE_OUTPUT);
#endif  

  IO_Init(IOP_OLED_DC, IO_MODE_OUTPUT);  
  
#ifdef IO_OLED_RES
  // SSD1306 Reset Pulse
  IO_Write(IOP_OLED_RES, 0);
  IO_Init(IOP_OLED_RES, IO_MODE_OUTPUT);
  DelayMs(10);
  IO_Write(IOP_OLED_RES, 1);
#endif  
  
  DelayMs(80);
  
  SSD1306_Init(bRotate);
  
#if defined(OL_FNT_SMALL)
  OLED_SetFont(FNT_SMALL);
#elif defined(OL_FNT_LARGE)
  OLED_SetFont(FNT_LARGE);  
#elif defined(OL_FNT_BIG)
  OLED_SetFont(FNT_BIG);  
#else
  #error "At least one font must be defined!"  
#endif  
}

void OLED_SetPage(unsigned char page)
{
  page &= 7;    // 0000 0111
  
  _page = page;
  
  OLED_Command(0xB0 | page);
}

void OLED_SetSegment(unsigned char segment)
{
  segment &= 0x7F;      // 0111 1111
  
  _segment = segment;
  
  OLED_Command(segment & 0x0F);         // set segment lower nibble
  OLED_Command(0x10 | (segment >> 4));  // set segment higher nibble
}

void OLED_FillPage(unsigned char page, unsigned char c)
{
  int i;
  
  OLED_SetPage(page);
  OLED_SetSegment(0);
  
  for (i = 0; i < NSEG; ++i)
    OLED_Data(c);
}

void OLED_FillDisplay(unsigned char c)
{
  int i;
  
  for (i = 0; i < NPGS; ++i)
    OLED_FillPage(i, c);
}

void OLED_ClearDisplay(void)
{
  OLED_FillDisplay(0);
}

////////////////////////////////////////////////////////////////////////////////

FNT_t OLED_GetFont(void)
{
  return _font;
}

void OLED_SetFont(FNT_t font)
{
  switch (font) {
#ifdef OL_FNT_SMALL    
  case FNT_SMALL:
    _szWidth = 5;
    _szHeight = 1;
    _pFont = g_ChrTab;
    break;
#endif    
    
#ifdef OL_FNT_LARGE
  case FNT_LARGE:
    _szWidth = 7;
    _szHeight = 2;
    _pFont = g_ChrTab2;
    break;
#endif    
    
#ifdef OL_FNT_BIG    
  case FNT_BIG:
    _szWidth = 15;
    _szHeight = 4;
    _pFont = g_ChrTab3;
    break;
#endif    
  }
  
  _font = font;
}

/////////////////////////////////////////////////////////////////
// Text cursor'ý konumlandýrýr
// row: satýr 0..20
// col: sütun 0..7
void OLED_SetCursor(unsigned char row, unsigned char col)
{
  _row = row;
  _col = col;
}

void OLED_GetCursor(int *pRow, int *pCol)
{
  *pRow = _row;
  *pCol = _col;
}

void OLED_UpdateDisplay(void)
{
  int i, j, k;
  
  for (i = k = 0; i < NPGS; ++i) {
    OLED_SetPage(i);
    OLED_SetSegment(0);
    
    for (j = 0; j < NSEG; ++j)
      OLED_Data(_DspRam[k++]);
  }
}

void OLED_Scroll(int nPgs)
{
  int i, j, k;
  
  // Ýþlemi display memory'de yap
  i = 0;        // hedef index
  j = nPgs * NSEG;
  
  for (k = 0; k < (NPGS - nPgs) * NSEG; ++k)
    _DspRam[i++] = _DspRam[j++];
  
  j = nPgs * NSEG;
  for (k = 0; k < j; ++k)
    _DspRam[i++] = 0;
  
  // Display memory'yi güncelle
  OLED_UpdateDisplay();
}
                 
void OLED_Return(void)
{
  _col = 0;
}

void OLED_NewLine(void)
{
  if (++_row >= T_ROWS) {
    OLED_Scroll(_szHeight);
    _row = T_ROWS - 1;
  }
}

// Text cursor konumuna parametredeki karakteri görüntüler
// Cursor konumunu artýrýr. Artým sonunda yeni satýr / scroll
// gerekiyorsa uygular
void OLED_PutChar(char ch)
{
  int c , i, j, k;
  
  c = (unsigned char)ch;
  
  if (_col >= T_COLS) {
    OLED_Return();
    OLED_NewLine();
  }
  
#ifdef OL_FNT_BIG  
  if (_font == FNT_BIG) {
    if (c < 32 || c > 127)
      c = 32;
    
    c -= 32;
  }
#endif  
  
  for (k = 0; k < _szHeight; ++k) {
    OLED_SetPage(_row * _szHeight + k);
    OLED_SetSegment(_col * (_szWidth + 1));
    
    i = c * _szWidth * _szHeight + k;
    
    for (j = 0; j < _szWidth; ++j) {
      OLED_Data(_pFont[i]);
      i += _szHeight;
    }
      
    OLED_Data(0);
  }
  
  ++_col;
}

////////////////////////////////////////////////////////////////////////////////

COLOR_t OLED_GetPixel(int x, int y)
{
  unsigned char page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  bitIdx = y & 7;

  val = _DspRam[page * NSEG + x];
  
  if (val & (1 << bitIdx))
    return SET_PIXEL;
  
  return CLR_PIXEL;
}

void OLED_SetPixel(int x, int y, COLOR_t c)
{
  unsigned char page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  bitIdx = y & 7;
  
  val = _DspRam[page * NSEG + x];
  
  switch (c) {
  case CLR_PIXEL:
    val &= ~(1 << bitIdx);
    break;
    
  case SET_PIXEL:
    val |= (1 << bitIdx);
    break;
    
  case INV_PIXEL:
    val ^= (1 << bitIdx);
    break;
  }
  
  OLED_SetPage(page);
  OLED_SetSegment(x);
  OLED_Data(val);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef OL_LINE
#define abs(a)      (((a) > 0) ? (a) : -(a))

void OLED_Line(int x0, int y0, int x1, int y1, COLOR_t c)
{
     int steep, t ;
     int deltax, deltay, error;
     int x, y;
     int ystep;

     steep = abs(y1 - y0) > abs(x1 - x0);

     if (steep)
     { // swap x and y
         t = x0; x0 = y0; y0 = t;
         t = x1; x1 = y1; y1 = t;
     }

     if (x0 > x1)
     {  // swap ends
         t = x0; x0 = x1; x1 = t;
         t = y0; y0 = y1; y1 = t;
     }

     deltax = x1 - x0;
     deltay = abs(y1 - y0);
     error = 0;
     y = y0;

     if (y0 < y1) 
         ystep = 1;
     else
         ystep = -1;

     for (x = x0; x < x1; x++)
     {
         if (steep)
            OLED_SetPixel(y, x, c);
         else
            OLED_SetPixel(x, y, c);

         error += deltay;
         if ((error << 1) >= deltax)
         {
             y += ystep;
             error -= deltax;
         } // if
     } // for
} // line
#endif

#ifdef OL_CIRCLE
#define PI      3.141592654

void OLED_Circle(int x, int y, int r, COLOR_t c)
{
    float step, t;
    int dx, dy;

    step = PI / 2 / 64;

    for (t = 0; t <= PI / 2; t += step) {
        dx = (int)(r * cos(t) + 0.5);
        dy = (int)(r * sin(t) + 0.5);

        if (x + dx < 128) {
            if (y + dy < 64)
                OLED_SetPixel(x + dx, y + dy, c);
            if (y - dy >= 0)
                OLED_SetPixel(x + dx, y - dy, c);
        }
        if (x - dx >= 0) {
            if (y + dy < 64)
                OLED_SetPixel(x - dx, y + dy, c);
            if (y - dy >= 0)
                OLED_SetPixel(x - dx, y - dy, c);
        }
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////

void OLED_putch(unsigned char c)
{
  switch (c) {
  case '\n':
    OLED_NewLine();
    
  case '\r':
    OLED_Return();
    break;
    
  case '\f':
    OLED_ClearDisplay();
    break;
    
  default:
    OLED_PutChar(c);
    break;
  }
}