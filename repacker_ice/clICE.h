#pragma once

#include "stdafx.h"

//--------------------------------------------------
// ICE class
//--------------------------------------------------
class clICE {
public:
  clICE(unsigned int key = CRYPT_KEY);
  ~clICE();

  // Load / Save / Release
  static bool CheckFile(void *data, unsigned int size);
  bool LoadFile(const char *path);
  bool SaveFile(const char *path, bool mode_compress = true,
                bool mode_crc = false);
  void Release(void);

  // Get version
  unsigned int GetVersion(void);

  // File operation
  unsigned int GetFileCount(void);
  bool GetFile(unsigned int index, const char **filename, unsigned char **data,
               unsigned int *size);
  bool Add(const char *filename, const char *path, int groupIndex = 1);
  bool Delete(const char *filename);
  bool Replace(const char *filename, const char *path);

private:
  struct stHeader {
    unsigned int signature; // 'ICE'
    unsigned int reserve;   // reserve
    unsigned int version;   // version
    unsigned int r1;        // 0x80
  };
  struct stInfo {
    unsigned int r1;       // 0xFF,0x350
    unsigned int crc32;    // CRC32
    unsigned int r2;       // 1(Encryption flag?)
    unsigned int filesize; // File size
  };
  struct stGroup {
    struct {
      unsigned int
          originalSize; // Size size before compression, Size after uncompress
      unsigned int dataSize;  // Size before uncompress, Size after compression
      unsigned int fileCount; // File count
      unsigned int crc32;     // CRC32
    } group1, group2;
    unsigned int group1Size; // Size of Group1
    unsigned int group2Size; // Size of Group2
    unsigned int key;        // key
    unsigned int reserve;    // reserve
  };
  struct stFile {
    stFile *next;
    int groupIndex;
    struct stHeader {
      unsigned int fileExt;      // File extension
      unsigned int fileSize;     // File size (include header)
      unsigned int dataSize;     // Data size
      unsigned int dataOffset;   // Data offset
      unsigned int filenameSize; // Filename size (include NULL character)
      unsigned int r1;           // 1(Encryption flag?)
      unsigned int reserve[10];  // reserve
    } * header;
    unsigned char *releaseData;
  } * _fileFirst, *_fileLast;

  unsigned int _key;
  unsigned char *_group1, *_group2;
  unsigned int _version;
  unsigned int GetKey(stGroup *group);
  unsigned int GetKey1(unsigned int filesize, unsigned char encryptTable[],
                       unsigned int version);
  unsigned int GetKey2(unsigned int key, unsigned char encryptTable[],
                       unsigned int version);
  unsigned int GetKey3(unsigned int key, unsigned char encryptTable[],
                       unsigned int version);
  unsigned char *Compress(unsigned char *data, unsigned int originalSize,
                          unsigned int *dataSize);
  unsigned char *Uncompress(unsigned char *data, unsigned int dataSize,
                            unsigned int originalSize);
  void Encrypt(void *data, unsigned int size, unsigned int version,
               unsigned int blowFishKey1, unsigned int blowFishKey2);
  void Decrypt(void *data, unsigned int size, unsigned int version,
               unsigned int blowFishKey1, unsigned int blowFishKey2);
  unsigned int _crc32[3];
  unsigned char _t[0x100];
};
