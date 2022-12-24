#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "io.h"
#include "system.h"
#include "uart.h"
#include "iuart.h"
#include "fifo.h"
#include "oled.h"

#define GNSS_ST        USART6
#define GNSS_P         UART_6
#define GNSS_IRQn      USART6_IRQn

static unsigned char _RxBuf[SZ_BUF];
static unsigned char _TxBuf[SZ_BUF];

static FIFO     _RxFifo;
static FIFO     _TxFifo;
static int      _TxFlag;

#define SZ_TOKEN        63

void GNSS_Init(int baud)
{
  UART_Init(GNSS_P, baud);
  
  // FIFO baþlangýç
  FIFO_Init(&_RxFifo, _RxBuf, SZ_BUF);
  FIFO_Init(&_TxFifo, _TxBuf, SZ_BUF);
  
  // Interrupt ayarlarý
  // a) ST yakasý
  USART_ITConfig(GNSS_ST, USART_IT_RXNE, ENABLE);
  USART_ITConfig(GNSS_ST, USART_IT_TXE, DISABLE);
  
  // b) ARM yakasý
  NVIC_SetPriority(GNSS_IRQn, 1);
  NVIC_EnableIRQ(GNSS_IRQn);
}

void USART6_IRQHandler(void)
{
  unsigned char c;
  
  if (USART_GetITStatus(GNSS_ST, USART_IT_RXNE)) {
    c = USART_ReceiveData(GNSS_ST);
    
    FIFO_SetData(&_RxFifo, c);
  }
  
  if (USART_GetITStatus(GNSS_ST, USART_IT_TXE)) {
    // Veriyi FIFO gönderme buffer'ýndan çek
    c = FIFO_GetData(&_TxFifo);
    
    // Çekilen veriyi TDR'a yükle
    USART_SendData(GNSS_ST, c);
    
    if (FIFO_IsEmpty(&_TxFifo)) {
      USART_ITConfig(GNSS_ST, USART_IT_TXE, DISABLE);
      _TxFlag = 0;
    }
  }
}

int GNSS_GetLine(unsigned char *LineBuf)
{
  return FIFO_GetLine(&_RxFifo, LineBuf);
}

// Interrupt yöntemi ile UART üzerinden blok veri gönderir
void GNSS_SendData(const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  // Önce Tx FIFO'ya göndereceðimiz veri yýðýnýný yüklüyoruz
  while (!FIFO_IsFull(&_TxFifo) && (len-- > 0))
    FIFO_SetData(&_TxFifo, *ptr++);
  
  _TxFlag = TRUE;
  USART_ITConfig(GNSS_ST, USART_IT_TXE, ENABLE);
}

int GNSS_TxActive(void)
{
  return _TxFlag;
}

//////////////////////////////////////////////////////////

// Parametresindeki hex text karakteri binary deðere
// çevirir, geri dönüþ deðeri olarak
// '0', '1', .., '9', 'A', .., 'F' 
unsigned char hex2bin(unsigned char c)
{
  if (isdigit(c))
    return c - '0';
  
  c = toupper(c);
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  
  return 0;
}

// NMEA cümlesi hata denetiminden baþarýyla geçerse
// TRUE döner
int NMEA_Check(void *nmBuf)
{
  unsigned char c, chk;
  unsigned char *ptr = (unsigned char *)nmBuf;
  
  if (*ptr++ != '$')
    return FALSE;
  
  chk = 0;
  while ((c = *ptr++) != '\0') {
    if (c == '*') {
      c = hex2bin(*ptr++) << 4;
      c |= hex2bin(*ptr);
      
      return (c == chk);
    }
    
    chk ^= c;
  }
  
  return FALSE;
}

char *NMEA_Token(void *nmBuf, int iToken)
{
    char c, *s, *p;
    int len;
    static char token[SZ_TOKEN + 1];
    
    s = (char *)nmBuf;
    while (iToken-- > 0) {
      while ((c = *s++) != ',')
        if (c == '\0')
          return NULL;
    }
    
    len = SZ_TOKEN;
    p = token;
    
    while (1) {
      if (len-- == 0)
        return NULL;
      
      c = *s++;
      if (c == '\0')
        return NULL;
      
      if ((c == ',') || (c == '*') || (c == '\r'))
        break;
      
      *p++ = c;
    }
    
    *p = '\0';
    
    return token;
}

void DisplayRMC(char *lbuf)
{
  static char strTime[8], strDate[8], strLat[16], strLon[16];
  static char strSpeed[8], strCourse[8];
  //static char strGP[8];
  char cLat, cLon, cWrn;

  cWrn = *NMEA_Token(lbuf, 2);
  if (cWrn != 'A')
    return;
  
  //strncpy(strGP, NMEA_Token(lbuf, 0), 8);
  strncpy(strTime, NMEA_Token(lbuf, 1), 6);
  strncpy(strDate, NMEA_Token(lbuf, 9), 6);
  
  strcpy(strLat, NMEA_Token(lbuf, 3));
  strcpy(strLon, NMEA_Token(lbuf, 5));
  if (*strLon == '0')
    *strLon = ' ';
  
  cLat = *NMEA_Token(lbuf, 4);
  cLon = *NMEA_Token(lbuf, 6);

  strcpy(strSpeed, NMEA_Token(lbuf, 7));
  strcpy(strCourse, NMEA_Token(lbuf, 8));
  
  OLED_SetCursor(0, 0);
  printf("%2.2s:%2.2s:%2.2s", strTime, strTime + 2, strTime + 4);
  printf("  %2.2s/%2.2s/20%2.2s\n\n", strDate, strDate + 2, strDate + 4);
  
  printf("%3.2s\370%s' %c\n"
         "%3.3s\370%s' %c\n\n",
         strLat, strLat + 2, cLat,
         strLon, strLon + 3, cLon);
  
  printf("Hýz: %s\nYön: %s", strSpeed, strCourse);
}

void Task_GNSS(void)
{
  static enum {
    S_INIT,
    S_LINE,
    S_GLL,
    S_RMC,
  } state = S_INIT;
  
  static unsigned char lbuf[SZ_BUF];
  
  if (FIFO_IsFull(&_RxFifo)) {
    OLED_SetFont(FNT_BIG);
    OLED_ClearDisplay();
    OLED_SetCursor(0, 0);
    
    printf("ERROR!");
    while (1) ;
  }
  
  switch (state) {
  case S_INIT:  
    GNSS_Init(9600);
    state = S_LINE;
    break;
    
  case S_LINE:
    if (!GNSS_GetLine(lbuf))
      break;
    
    //printf("%s", lbuf);
    
    if (!strncmp((const char *)lbuf, "$GPGLL", 6))
      state = S_GLL;
    else if (!strncmp((const char *)lbuf, "$GPRMC", 6))
      state = S_RMC;
    else
      break;

    //printf("%s", lbuf);
    
    if (!NMEA_Check(lbuf)) 
      state = S_LINE;
    
    break;
    
  case S_GLL:
    // GLL cümlesini iþleyeceðiz
    DelayMs(100);
    state = S_LINE;
    break;

  case S_RMC:
    // RMC cümlesini iþleyeceðiz
    DisplayRMC((char *)lbuf);
    DelayMs(100);
    state = S_LINE;
    break;
  }
}

