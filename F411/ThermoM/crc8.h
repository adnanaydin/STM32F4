#ifndef __CRC8_H
#define __CRC8_H

#ifdef __cplusplus
extern "C" {
#endif

#define CRC_TABLE
  
unsigned char CRC8(const void *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
