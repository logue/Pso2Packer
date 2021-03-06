#pragma once

//--------------------------------------------------
// Compression class
//--------------------------------------------------
class clCompress {
public:
  clCompress();

  // Compression
  //   Add 0 for 1 byte at the end of src. <-- may not be necessary?
  //   The size of dst must be (srcLen+srcLen/8+3) or more.
  int Compress(unsigned char *dst, unsigned char *src, int dstLen, int srcLen);

private:
  struct stDictionary {
    stDictionary *prev;
    stDictionary *next;
    unsigned char *s;
    void Init(void);
    void Disconnect(void);
    stDictionary *InsertNext(stDictionary *newDic);
    void GetLongest(int *rSize, int *rOffset, unsigned char *src);
  };
  int _count;
  int _bit;
  unsigned char *_df;
  unsigned char *_dd;
  unsigned char *_s;
  unsigned char *_se;
  int _dicCount;
  int GetIndex(unsigned char *s);
  void SetBit(int bitLen, int bit);
  void SetByte(unsigned char v);
};

//--------------------------------------------------
// Uncompress class
//--------------------------------------------------
class clUncompress {
public:
  clUncompress();

  // Uncompress
  int Uncompress(unsigned char *dst, unsigned char *src, int dstLen,
                 int srcLen);

private:
  unsigned char _count;
  unsigned char _bit;
  unsigned char *_d;
  unsigned char *_s;
  unsigned char *_ds;
  unsigned char *_de;
  unsigned char *_ss;
  unsigned char *_se;
  unsigned char Get1Bit(void);
  unsigned char Get2Bit(void);
  unsigned char Get1Byte(void);
  unsigned short Get2Byte(void);
  void Set1Byte(unsigned char v);
  void SetMultiByte(unsigned char *s, int size);
};
