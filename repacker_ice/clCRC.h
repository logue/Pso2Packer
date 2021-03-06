#pragma once

//--------------------------------------------------
// CRC class
//--------------------------------------------------
class clCRC {
public:
  clCRC();
  unsigned int GetCRC32(unsigned char *data, int size, unsigned int crc = 0);

private:
  unsigned int _table[0x100];
};
