//--------------------------------------------------
// Include
//--------------------------------------------------
#include "clCRC.h"
#include "stdafx.h"

//--------------------------------------------------
// Constructor
//--------------------------------------------------
clCRC::clCRC() {
  for (int i = 0; i < 0x100; i++) {
    unsigned int crc = i;
    for (int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    _table[i] = crc;
  }
}

//--------------------------------------------------
// Get CRC
//--------------------------------------------------
unsigned int clCRC::GetCRC32(unsigned char *data, int size, unsigned int crc) {
  crc ^= -1;
  if (data) {
    for (; size > 0; size--) {
      crc = _table[(*(data++) ^ crc) & 0xFF] ^ (crc >> 8);
    }
  }
  return crc ^ -1;
}
