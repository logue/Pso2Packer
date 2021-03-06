//--------------------------------------------------
// Include
//--------------------------------------------------
#include "clCompress.h"
#include "stdafx.h"

//================================================================================
// Compression class

//--------------------------------------------------
// Constructor
//--------------------------------------------------
clCompress::clCompress()
    : _count(0), _bit(0), _df(0), _dd(0), _s(0), _se(0), _dicCount(0) {}

//--------------------------------------------------
// Compress
//--------------------------------------------------
int clCompress::Compress(unsigned char *dst, unsigned char *src, int dstLen,
                         int srcLen) {

  // Check
  if (!(dst && src && dstLen >= srcLen + srcLen / 8 + 3))
    return 0;

  // Initialization
  _count = 0;
  _bit = 0;
  _df = dst;
  _dd = dst + 1;
  _s = src;
  _se = src + srcLen;
  _dicCount = 0;

  // Initialization of Dictionary
  stDictionary dic1[0x2000], dic2[0x2000];
  for (int i = 0; i < 0x2000; i++) {
    dic1[i].Init();
    dic2[i].Init();
  }

  // Compress
  while (_s < _se) {

    // Dictionary index determined from data value
    int index = GetIndex(_s);

    // Get the longest hit dictionary 1
    int longestSize, longestOffset;
    dic1[index].GetLongest(&longestSize, &longestOffset, _s);

    //
    if (longestSize > 1) {

      // Add dictionary
      dic1[index].InsertNext(&dic2[(_dicCount++) & 0x1FFF])->s = _s;

      // Get the longest hit dictionary 2
      int longestSize2, longestOffset2;
      dic1[GetIndex(_s + 1)].GetLongest(&longestSize2, &longestOffset2, _s + 1);

      // If the compression rate of the next data is higher, output with no
      // compression
      if (longestSize2 > longestSize + 1) {
        SetBit(1, 1);
        SetByte(*(_s++));
        longestSize = longestSize2;
        longestOffset = longestOffset2;
      } else {
        dic1[index].next->Disconnect();
        _dicCount--;
      }
    }

    // Write
    if (longestSize >= 2 && longestSize <= 5 && longestOffset >= -0x100) {
      SetBit(4, longestSize - 2);
      SetByte(longestOffset);
    } else if (longestSize >= 3) {
      SetBit(2, 1);
      if (longestSize <= 9) {
        SetByte((longestOffset << 3) | (longestSize - 2));
        SetByte(longestOffset >> 5);
      } else {
        SetByte(longestOffset << 3);
        SetByte(longestOffset >> 5);
        SetByte(longestSize - 10);
      }
    } else {
      SetBit(1, 1);
      SetByte(*_s);
      longestSize = 1;
    }

    // Add dictionary
    for (; longestSize > 0; longestSize--, _s++) {
      dic1[GetIndex(_s)].InsertNext(&dic2[(_dicCount++) & 0x1FFF])->s = _s;
    }
  }

  // Add finish flag
  SetBit(2, 1);
  SetByte(0);
  SetByte(0);

  // Write the remaining bit flag
  SetBit(9 - _count, 0);

  return _dd - 1 - dst;
}
int clCompress::GetIndex(unsigned char *s) {
  return s[0] | (((int)s[1] & 0x1F) << 8);
}
void clCompress::SetBit(int bitLen, int bit) {
  _count += bitLen;
  _bit = (_bit << bitLen) | bit;
  if (_count > 8) {
    _count -= 8;
    unsigned char d = _bit >> _count;
    unsigned char a = d;
    a = (a << 1) | ((d >> 1) & 1);
    a = (a << 1) | ((d >> 2) & 1);
    a = (a << 1) | ((d >> 3) & 1);
    a = (a << 1) | ((d >> 4) & 1);
    a = (a << 1) | ((d >> 5) & 1);
    a = (a << 1) | ((d >> 6) & 1);
    a = (a << 1) | ((d >> 7) & 1);
    *_df = a ^ 0x95;
    _df = (_dd++);
  }
}
void clCompress::SetByte(unsigned char v) { *(_dd++) = v ^ 0x95; }

//--------------------------------------------------
// Dictionary
//--------------------------------------------------
void clCompress::stDictionary::Init(void) { prev = next = this; }
void clCompress::stDictionary::Disconnect(void) {
  prev->next = next;
  next->prev = prev;
  prev = next = this;
}
clCompress::stDictionary *
clCompress::stDictionary::InsertNext(stDictionary *newDic) {

  // Detach
  newDic->prev->next = newDic->next;
  newDic->next->prev = newDic->prev;

  // Insert next
  newDic->prev = this;
  newDic->next = next;
  next->prev = newDic;
  next = newDic;

  return newDic;
}
void clCompress::stDictionary::GetLongest(int *rSize, int *rOffset,
                                          unsigned char *src) {
  int size = 0;
  unsigned char *s = src;
  for (stDictionary *dic = next; dic != this; dic = dic->next) {
    if (src - dic->s < 0x2000) {
      int len = 1;
      for (unsigned char *s1 = src, *s2 = dic->s; *(++s1) == *(++s2); len++)
        ;
      if (size < len) {
        size = len;
        s = dic->s;
        if (len >= 0x109) {
          size = 0x109;
          break;
        }
      }
    }
  }
  *rSize = size;
  *rOffset = s - src;
}

//================================================================================
// Uncompress class

//--------------------------------------------------
// Constructor
//--------------------------------------------------
clUncompress::clUncompress()
    : _count(0), _bit(0), _d(0), _s(0), _ds(0), _de(0), _ss(0), _se(0) {}

//--------------------------------------------------
// Uncompress
//--------------------------------------------------
int clUncompress::Uncompress(unsigned char *d, unsigned char *s, int dlen,
                             int slen) {

  // Check
  // if(!d)return 0;
  if (!s)
    return 0;

  // Initialization
  _count = 0;
  _bit = 0;
  _d = d;
  _s = s;
  _ds = d;
  _de = d + dlen;
  _ss = s;
  _se = s + slen;

  // Extract
  while (true) {

    // Uncompressed data output
    while (Get1Bit()) {
      Set1Byte(Get1Byte());
    }

    // Dictionary output
    int offset;
    int size;
    if (Get1Bit()) {
      unsigned short v = Get2Byte();
      if (v == 0)
        return _d - _ds;
      offset = (int)(v >> 3) - 0x2000;
      size = v & 7;
      if (size) {
        size += 2;
      } else {
        size = (int)Get1Byte() + 10;
      }
    } else {
      size = Get2Bit() + 2;
      offset = (int)Get1Byte() - 0x100;
    }
    SetMultiByte(_d + offset, size);
  }
}
unsigned char clUncompress::Get1Bit(void) {
  if (_count == 0) {
    _count = 8;
    _bit = Get1Byte();
  }
  unsigned char v = _bit & 1;
  _count--;
  _bit >>= 1;
  return v;
}
unsigned char clUncompress::Get2Bit(void) {
  return (Get1Bit() << 1) | Get1Bit();
}
unsigned char clUncompress::Get1Byte(void) {
  if (_s < _ss || _s >= _se) {
    _s++;
    return 0;
  }
  return *(_s++) ^ 0x95;
}
unsigned short clUncompress::Get2Byte(void) {
  return Get1Byte() | ((unsigned short)Get1Byte() << 8);
}
void clUncompress::Set1Byte(unsigned char v) {
  if (_d >= _ds && _d < _de)
    *_d = v;
  _d++;
}
void clUncompress::SetMultiByte(unsigned char *s, int size) {
  while (size > 0) {
    unsigned char v = 0;
    if (s >= _ds && s < _de)
      v = *s;
    s++;
    Set1Byte(v);
    size--;
  }
}
