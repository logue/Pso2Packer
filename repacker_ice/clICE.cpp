
//--------------------------------------------------
// Include
//--------------------------------------------------
#include "clICE.h"
#include "clBlowFish.h"
#include "clCRC.h"
#include "clCompress.h"
#include "stdafx.h"

//--------------------------------------------------
// Inline function
//--------------------------------------------------

inline unsigned int Aligned(unsigned int v, unsigned int align) {
  return (v + (align - 1)) & (-1 ^ (align - 1));
}

//--------------------------------------------------
// Constructor / Destructor
//--------------------------------------------------
clICE::clICE(unsigned int key)
    : _key(key), _group1(nullptr), _group2(nullptr), _fileFirst(nullptr),
      _fileLast(nullptr), _version(0) {
  memset(_crc32, 0, sizeof(_crc32));
  memset(_t, 0, sizeof(_t));
}
clICE::~clICE() { Release(); }

//--------------------------------------------------
// ICE check
//--------------------------------------------------
bool clICE::CheckFile(void *data, unsigned int size) {
  return (data && size >= 4 && *(int *)data == SIGNATURE);
}

//--------------------------------------------------
// Load file
//--------------------------------------------------
bool clICE::LoadFile(const char *path) {
  printf("\n***** clICE::LoadFile\n");
  if (!path)
    return false;

  // release
  Release();

  // Open file
  FILE *fp;
  if (fopen_s(&fp, path, "rb"))
    return false;

  // Header check
  printf("Checking header...\n");
  stHeader h;
  fread(&h, sizeof(h), 1, fp);
  if (!CheckFile(&h, sizeof(h))) {
    printf("Error: seems not ICE format.\n");
    fclose(fp);
    return false;
  }

  // Process by version
  stGroup g;
  _version = h.version;
  printf("Version : %d \n", _version);
  switch (h.version) {
  case 3: {

    // Get information
    stInfo i;
    fread(&g, sizeof(g), 1, fp);
    fread(&i, sizeof(i), 1, fp);
    fseek(fp, 0x30, SEEK_CUR);
    _crc32[0] = i.crc32;
    _crc32[1] = g.group1.crc32;
    _crc32[2] = g.group2.crc32;
    memset(_t, 0, sizeof(_t));

    // generate key
    unsigned int key = g.group1Size;
    if (key)
      key = bswap(key);
    else if (i.r2)
      key = GetKey(&g);
    printf("\n--Decryption Infomation------------\n");
    // printf("crc32                 = %10d [%08x]\n", crc32, crc32);
    printf("key                   = %10d [%08x]\n", key, key);

    // Group1
    printf("\n--Group1---------------------------\n");
    printf("g.group1.fileCount    = %10d [%08x]\n", g.group1.fileCount,
           g.group1.fileCount);
    if (g.group1.dataSize) {
      unsigned char *data = new unsigned char[g.group1.dataSize];
      if (!data) {
        fclose(fp);
        return false;
      }

      printf("g.group1.originalSize = %10d [%08x]\n", g.group1.originalSize,
             g.group1.originalSize);
      printf("g.group1.dataSize     = %10d [%08x]\n", g.group1.dataSize,
             g.group1.dataSize);
      printf("g.group1.crc32        = %10d [%08x]\n", g.group1.crc32,
             g.group1.crc32);
      printf("g.group1Size          = %10d [%08x]\n", g.group1Size,
             g.group1Size);

      fread(data, g.group1.dataSize, 1, fp);
      Decrypt(data, g.group1.dataSize, h.version, key, 0);
      _group1 = Uncompress(data, g.group1.dataSize, g.group1.originalSize);
      delete[] data;
    } else if (g.group1.originalSize) {
      unsigned char *data = new unsigned char[g.group1.originalSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      fread(data, g.group1.originalSize, 1, fp);
      Decrypt(data, g.group1.originalSize, h.version, key, 0);
      _group1 = data;
    }

    // Group2
    printf("\n--Group2---------------------------\n");
    printf("g.group2.fileCount    = %10d [%08x]\n", g.group2.fileCount,
           g.group2.fileCount);
    if (g.group2.dataSize) {
      unsigned char *data = new unsigned char[g.group2.dataSize];
      if (!data) {
        fclose(fp);
        return false;
      }

      printf("g.group2.originalSize = %10d [%08x]\n", g.group2.originalSize,
             g.group2.originalSize);
      printf("g.group2.dataSize     = %10d [%08x]\n", g.group2.dataSize,
             g.group2.dataSize);
      printf("g.group2.crc32        = %10d [%08x]\n", g.group2.crc32,
             g.group2.crc32);
      printf("g.group2Size          = %10d [%08x]\n", g.group2Size,
             g.group2Size);

      fread(data, g.group2.dataSize, 1, fp);
      Decrypt(data, g.group2.dataSize, h.version, key, 0);
      _group2 = Uncompress(data, g.group2.dataSize, g.group2.originalSize);
      delete[] data;
    } else if (g.group2.originalSize) {
      unsigned char *data = new unsigned char[g.group2.originalSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      fread(data, g.group2.originalSize, 1, fp);
      Decrypt(data, g.group2.originalSize, h.version, key, 0);
      _group2 = data;
    }

  } break;
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9: {

    // Get information
    stInfo i;
    unsigned char t[0x100];
    fread(&i, sizeof(i), 1, fp);
    if (h.version > 4)
      fseek(fp, 0x10, SEEK_CUR);
    fread(t, sizeof(t), 1, fp);
    fread(&g, sizeof(g), 1, fp);

    // generate key for group1, group2
    static int list13[] = {13, 17, 4, 7, 5, 14};
    static int list17[] = {17, 25, 15, 10, 28, 8};
    unsigned int key = GetKey3(
        GetKey2(GetKey1(i.filesize, t, h.version), t, h.version), t, h.version);
    unsigned int groupHeadersKey =
        rol(key, h.version <= 9 ? list13[h.version - 4] : 0);
    unsigned int group1BlowFishKey1 = key;
    unsigned int group1BlowFishKey2 = GetKey2(key, t, h.version);

    unsigned int group2BlowFishKey1 =
        rol(group1BlowFishKey1, h.version <= 9 ? list17[h.version - 4] : 0);
    unsigned int group2BlowFishKey2 =
        rol(group1BlowFishKey2, h.version <= 9 ? list17[h.version - 4] : 0);

    // Decrypt group information
    clBlowFish bf(&groupHeadersKey, sizeof(groupHeadersKey), true);
    bf.Decrypt(&g, sizeof(g));
    printf("\n--Decryption Infomation------------\n");
    /// printf("crc32                 = %12d [%08x]\n", crc32, crc32);
    printf("key                   = %12d [%08x]\n", key, key);
    printf("groupHeadersKey       = %12d [%08x]\n", groupHeadersKey,
           groupHeadersKey);
    printf("g.reserve             = %12d [%08x]\n", g.reserve, g.reserve);

    // check
    if (g.reserve) {
      printf("\nWarning: It may have failed to generate the key.\n");
      printf("         If you continue, please enter any key.\n");
      char str[4];
      fgets(str, sizeof(str), stdin);
    }

    // Maintain CRC (temporary)
    _crc32[0] = i.crc32;
    _crc32[1] = g.group1.crc32;
    _crc32[2] = g.group2.crc32;
    memcpy(_t, t, sizeof(t));

    printf("---------------------------------\n");
    printf("sizeof(h)             = %d\n", sizeof(h));
    printf("sizeof(i)             = %d\n", sizeof(i));
    printf("sizeof(t)             = %d\n", sizeof(t));
    printf("sizeof(g)             = %d\n", sizeof(g));
    printf("h+i+t+g               = %d\n",
           sizeof(h) + sizeof(i) + sizeof(t) + sizeof(g));
    printf("\n");

    printf("i.r1                  = %12d [%08x]\n", i.r1, i.r1);
    printf("i.crc32               = %12d [%08x]\n", i.crc32, i.crc32);
    printf("i.r2                  = %12d [%08x]\n", i.r2, i.r2);
    printf("i.filesize            = %12d [%08x]\n", i.filesize, i.filesize);

    printf("g.key                 = %12d [%08x]\n", g.key, g.key);
    printf("g.reserve             = %12d [%08x]\n", g.reserve, g.reserve);

    // Group1
    printf("\n--Group1---------------------------\n");
    printf("group1BlowFishKey1    = %12d [%08x]\n", group1BlowFishKey1,
           group1BlowFishKey1);
    printf("group1BlowFishKey2    = %12d [%08x]\n", group1BlowFishKey2,
           group1BlowFishKey2);
    printf("g.group1.fileCount    = %12d [%08x]\n", g.group1.fileCount,
           g.group1.fileCount);

    if (g.group1.dataSize) {
      unsigned char *data = new unsigned char[g.group1.dataSize];
      if (!data) {
        fclose(fp);
        return false;
      }

      printf("g.group1.originalSize = %12d [%08x]\n", g.group1.originalSize,
             g.group1.originalSize);
      printf("g.group1.dataSize     = %12d [%08x]\n", g.group1.dataSize,
             g.group1.dataSize);
      printf("g.group1.crc32        = %12d [%08x]\n", g.group1.crc32,
             g.group1.crc32);
      printf("g.group1Size          = %12d [%08x]\n", g.group1Size,
             g.group1Size);

      fread(data, g.group1.dataSize, 1, fp);
      Decrypt(data, g.group1.dataSize, h.version, group1BlowFishKey1,
              group1BlowFishKey2);
      _group1 = Uncompress(data, g.group1.dataSize, g.group1.originalSize);
      delete[] data;
    } else if (g.group1.originalSize) {
      unsigned char *data = new unsigned char[g.group1.originalSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      fread(data, g.group1.originalSize, 1, fp);
      Decrypt(data, g.group1.originalSize, h.version, group1BlowFishKey1,
              group1BlowFishKey2);
      _group1 = data;
    }

    // Group2
    printf("\n--Group2---------------------------\n");
    printf("group2BlowFishKey1    = %12d [%08x]\n", group2BlowFishKey1,
           group2BlowFishKey1);
    printf("group2BlowFishKey2    = %12d [%08x]\n", group2BlowFishKey2,
           group2BlowFishKey2);
    printf("g.group2.fileCount    = %12d [%08x]\n", g.group2.fileCount,
           g.group2.fileCount);
    if (g.group2.dataSize) {
      unsigned char *data = new unsigned char[g.group2.dataSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      printf("g.group2.originalSize = %12d [%08x]\n", g.group2.originalSize,
             g.group2.originalSize);
      printf("g.group2.dataSize     = %12d [%08x]\n", g.group2.dataSize,
             g.group2.dataSize);
      printf("g.group2.crc32        = %12d [%08x]\n", g.group2.crc32,
             g.group2.crc32);
      printf("g.group2Size          = %12d [%08x]\n", g.group2Size,
             g.group2Size);
      fread(data, g.group2.dataSize, 1, fp);
      Decrypt(data, g.group2.dataSize, h.version, group2BlowFishKey1,
              group2BlowFishKey2);
      _group2 = Uncompress(data, g.group2.dataSize, g.group2.originalSize);
      delete[] data;
    } else if (g.group2.originalSize) {
      unsigned char *data = new unsigned char[g.group2.originalSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      fread(data, g.group2.originalSize, 1, fp);
      Decrypt(data, g.group2.originalSize, h.version, group2BlowFishKey1,
              group2BlowFishKey2);
      _group2 = data;
    }

  } break;
  default:
    printf("Error: Unsupported format.\n");
    fclose(fp);
    return false;
  }

  // Close file
  fclose(fp);

  // Add list
  stFile::stHeader *file = (stFile::stHeader *)_group1;
  for (unsigned int i = 0; i < g.group1.fileCount && file;
       i++, file = (stFile::stHeader *)((char *)file + file->fileSize)) {
    stFile *f = new stFile;
    if (!f)
      return false;
    memset(f, 0, sizeof(*f));
    f->groupIndex = 0;
    f->header = file;
    if (_fileLast)
      _fileLast->next = f;
    else
      _fileFirst = f;
    _fileLast = f;
  }
  file = (stFile::stHeader *)_group2;
  for (unsigned int i = 0; i < g.group2.fileCount && file;
       i++, file = (stFile::stHeader *)((char *)file + file->fileSize)) {
    stFile *f = new stFile;
    if (!f)
      return false;
    memset(f, 0, sizeof(*f));
    f->groupIndex = 1;
    f->header = file;
    if (_fileLast)
      _fileLast->next = f;
    else
      _fileFirst = f;
    _fileLast = f;
  }

  printf("\ndone.\nPlease enter any key to exit.\n");
  char str[4];
  fgets(str, sizeof(str), stdin);
  return true;
}

//--------------------------------------------------
// Save file
//--------------------------------------------------
bool clICE::SaveFile(const char *path, bool mode_compress, bool mode_crc) {
  printf("\n***** clICE::SaveFile\n");

  clCRC crc;
  if (!path)
    return false;

  // Open file
  FILE *fp;
  if (fopen_s(&fp, path, "wb"))
    return false;

  // Generate group & compression
  stGroup g;
  memset(&g, 0, sizeof(g));
  unsigned char *group1 = nullptr, *group2 = nullptr;

  printf("Prosessing Gruop1...\n");
  {
    for (stFile *file = _fileFirst; file; file = file->next)
      if (file->groupIndex == 0 && file->header)
        g.group1.originalSize += file->header->fileSize;
    unsigned char *buf = new unsigned char[g.group1.originalSize], *p = buf;
    if (!buf) {
      fclose(fp);
      return false;
    }
    for (stFile *file = _fileFirst; file; file = file->next)
      if (file->groupIndex == 0 && file->header) {
        memcpy(p, file->header, file->header->fileSize);
        p += file->header->fileSize;
        g.group1.fileCount++;
      }
    if (mode_compress) {
      printf("Compressing Gruop1...\n");
      group1 = Compress(buf, g.group1.originalSize, &g.group1.dataSize);
    }
    if (group1 && g.group1.originalSize > g.group1.dataSize) {
      delete[] buf;
    } else {
      if (group1) {
        delete[] group1;
      }
      group1 = buf;
      g.group1.dataSize = 0;
    }
    g.group1Size =
        g.group1.dataSize ? g.group1.dataSize : g.group1.originalSize;
  }

  printf("Prosessing Gruop2...\n");
  {
    for (stFile *file = _fileFirst; file; file = file->next)
      if (file->groupIndex == 1 && file->header)
        g.group2.originalSize += file->header->fileSize;
    unsigned char *buf = new unsigned char[g.group2.originalSize], *p = buf;
    if (!buf) {
      if (group1)
        delete[] group1;
      fclose(fp);
      return false;
    }
    for (stFile *file = _fileFirst; file; file = file->next)
      if (file->groupIndex == 1 && file->header) {
        memcpy(p, file->header, file->header->fileSize);
        p += file->header->fileSize;
        g.group2.fileCount++;
      }
    if (mode_compress) {
      printf("Compressing Gruop2...\n");
      group2 = Compress(buf, g.group2.originalSize, &g.group2.dataSize);
    }
    if (group2 && g.group2.originalSize > g.group2.dataSize) {
      delete[] buf;
    } else {
      if (group2) {
        delete[] group2;
      }
      group2 = buf;
      g.group2.dataSize = 0;
    }
    g.group2Size =
        g.group2.dataSize ? g.group2.dataSize : g.group2.originalSize;
  }

  // Generate header info
  printf("Generating header...\n");
  stHeader h;
  memset(&h, 0, sizeof(h));
  h.signature = SIGNATURE;
  h.version = ICE_VERSION;
  h.r1 = 0x80;
  stInfo i;
  memset(&i, 0, sizeof(i));
  i.r1 = 0xFF;
  i.r2 = 1;
  unsigned char t[0x100];
  memset(t, 0, sizeof(t));
  if (mode_crc)
    memcpy(t, _t, sizeof(_t)); // Maintain random table
  i.crc32 =
      crc.GetCRC32(group2, g.group2Size, crc.GetCRC32(group1, g.group1Size));
  i.filesize = sizeof(h) + sizeof(i) + sizeof(t) + sizeof(g) + g.group1Size +
               g.group2Size;

  // Generate key group1, group2
  static int list13[] = {13, 17, 4, 7, 5, 14};
  static int list17[] = {17, 25, 15, 10, 28, 8};
  unsigned int key = GetKey3(
      GetKey2(GetKey1(i.filesize, t, h.version), t, h.version), t, h.version);
  unsigned int groupHeadersKey =
      rol(key, h.version <= 9 ? list13[h.version - 4] : 0);
  unsigned int group1BlowFishKey1 = key;
  unsigned int group1BlowFishKey2 = GetKey2(key, t, h.version);
  unsigned int group2BlowFishKey1 =
      rol(group1BlowFishKey1, h.version <= 9 ? list17[h.version - 4] : 0);
  unsigned int group2BlowFishKey2 =
      rol(group1BlowFishKey2, h.version <= 9 ? list17[h.version - 4] : 0);

  // Encryption
  Encrypt(group1, g.group1Size, h.version, group1BlowFishKey1,
          group1BlowFishKey2);
  g.group1.crc32 = crc.GetCRC32(group1, g.group1Size);
  Encrypt(group2, g.group2Size, h.version, group2BlowFishKey1,
          group2BlowFishKey2);
  g.group2.crc32 = crc.GetCRC32(group2, g.group2Size);

  // Maintain CRC (temporary)
  if (mode_crc) {
    i.crc32 = _crc32[0];
    g.group1.crc32 = _crc32[1];
    g.group2.crc32 = _crc32[2];
  }

  // Write
  printf("Writing..\n");
  fwrite(&h, sizeof(h), 1, fp);
  fwrite(&i, sizeof(i), 1, fp);
  fwrite(t, sizeof(t), 1, fp);
  stGroup group;
  memcpy(&group, &g, sizeof(group));
  clBlowFish bf(&groupHeadersKey, sizeof(groupHeadersKey), true);
  bf.Encrypt(&group, sizeof(group));
  fwrite(&group, sizeof(group), 1, fp);
  if (group1) {
    fwrite(group1, g.group1Size, 1, fp);
    delete[] group1;
  }
  if (group2) {
    fwrite(group2, g.group2Size, 1, fp);
    delete[] group2;
  }

  printf("---------------------------------\n");
  printf("i.r1                  = %d [%08x]\n", i.r1, i.r1);
  printf("i.crc32               = %d [%08x]\n", i.crc32, i.crc32);
  printf("i.r2                  = %d [%08x]\n", i.r2, i.r2);
  printf("i.filesize            = %d [%08x]\n", i.filesize, i.filesize);

  printf("g.group1.originalSize = %d [%08x]\n", g.group1.originalSize,
         g.group1.originalSize);
  printf("g.group1.dataSize     = %d [%08x]\n", g.group1.dataSize,
         g.group1.dataSize);
  printf("g.group1.fileCount    = %d [%08x]\n", g.group1.fileCount,
         g.group1.fileCount);
  printf("g.group1.crc32        = %d [%08x]\n", g.group1.crc32, g.group1.crc32);
  printf("g.group1Size          = %d [%08x]\n", g.group1Size, g.group1Size);

  printf("g.group2.originalSize = %d [%08x]\n", g.group2.originalSize,
         g.group2.originalSize);
  printf("g.group2.dataSize     = %d [%08x]\n", g.group2.dataSize,
         g.group2.dataSize);
  printf("g.group2.fileCount    = %d [%08x]\n", g.group2.fileCount,
         g.group2.fileCount);
  printf("g.group2.crc32        = %d [%08x]\n", g.group2.crc32, g.group2.crc32);
  printf("g.group2Size          = %d [%08x]\n", g.group2Size, g.group2Size);
  printf("g.key                 = %d [%08x]\n", g.key, g.key);
  printf("g.reserve             = %d [%08x]\n", g.reserve, g.reserve);

  // Close file
  fclose(fp);

  return true;
}

//--------------------------------------------------
// Release
//--------------------------------------------------
void clICE::Release(void) {

  // Release
  if (_group1)
    delete[] _group1;
  if (_group2)
    delete[] _group2;
  for (stFile *f = _fileFirst, *n; f; f = n) {
    n = f->next;
    if (f->releaseData)
      delete[] f->releaseData;
    delete[] f;
  }

  // Reset
  _group1 = nullptr;
  _group2 = nullptr;
  _fileFirst = nullptr;
  _fileLast = nullptr;
}

//--------------------------------------------------
// Get version
//--------------------------------------------------
unsigned int clICE::GetVersion(void) { return _version; }

//--------------------------------------------------
// Get file count
//--------------------------------------------------
unsigned int clICE::GetFileCount(void) {
  unsigned int count = 0;
  for (stFile *f = _fileFirst; f; f = f->next)
    count++;
  return count;
}

//--------------------------------------------------
// Get file info
//--------------------------------------------------
bool clICE::GetFile(unsigned int index, const char **filename,
                    unsigned char **data, unsigned int *size) {
  stFile *f = _fileFirst;
  for (; f && index; f = f->next)
    index--;
  if (f && f->header) {
    if (filename)
      *filename = (char *)f->header + sizeof(*f->header);
    if (data)
      *data = (unsigned char *)f->header + f->header->dataOffset;
    if (size)
      *size = f->header->dataSize;
  }
  return f != nullptr;
}

//--------------------------------------------------
// Add file
//--------------------------------------------------
bool clICE::Add(const char *filename, const char *path, int groupIndex) {
  if (!(filename && path))
    return false;
  FILE *fp;
  if (fopen_s(&fp, path, "rb"))
    return false;
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  int dataOffset =
      Aligned(sizeof(stFile::stHeader) + strlen(filename) + 1, 0x10);
  int fileSize = dataOffset + size;
  if (strstr(path, ".dds")) {
    fileSize += (size % 0x10);
  }
  unsigned char *data = new unsigned char[fileSize];
  if (!data) {
    fclose(fp);
    return false;
  }
  stFile::stHeader *h = (stFile::stHeader *)data;
  memset(h, 0, fileSize);
  const char *e = strrchr(path, '.');
  if (e && strlen(e + 1) <= 4)
    h->fileExt = *(int *)(e + 1);
  h->fileSize = fileSize;
  h->dataSize = size;
  h->dataOffset = dataOffset;
  h->filenameSize = strlen(filename) + 1;
  h->r1 = 1;
  strcpy_s((char *)&data[sizeof(*h)], dataOffset - sizeof(*h), filename);
  fread(&data[dataOffset], size, 1, fp);
  fclose(fp);
  stFile *f = new stFile;
  if (!f) {
    delete[] data;
    return false;
  }
  memset(f, 0, sizeof(*f));
  f->groupIndex = groupIndex;
  f->header = h;
  f->releaseData = data;
  if (_fileLast)
    _fileLast->next = f;
  else
    _fileFirst = f;
  _fileLast = f;
  return true;
}

//--------------------------------------------------
// Delete file
//--------------------------------------------------
bool clICE::Delete(const char *filename) {
  if (!(filename))
    return false;
  for (stFile *f = _fileFirst, *prev = NULL; f; prev = f, f = f->next) {
    if (f->header &&
        strcmp((char *)f->header + sizeof(*f->header), filename) == 0) {
      if (prev)
        prev->next = f->next;
      else
        _fileFirst = f->next;
      if (!f->next)
        _fileLast = prev;
      if (f->releaseData)
        delete[] f->releaseData;
      delete f;
      return true;
    }
  }
  return false;
}

//--------------------------------------------------
// Replace file
//--------------------------------------------------
bool clICE::Replace(const char *filename, const char *path) {
  if (!(filename && path))
    return false;
  for (stFile *f = _fileFirst; f; f = f->next) {
    if (f->header &&
        strcmp((char *)f->header + sizeof(*f->header), filename) == 0) {
      FILE *fp;
      if (fopen_s(&fp, path, "rb"))
        return false;
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      f->header->dataSize = size;
      f->header->fileSize = f->header->dataOffset + size;
      unsigned char *data = new unsigned char[f->header->fileSize];
      if (!data) {
        fclose(fp);
        return false;
      }
      memcpy(data, f->header, f->header->dataOffset);
      fread(&data[f->header->dataOffset], size, 1, fp);
      memset(&data[f->header->dataOffset + size], 0,
             f->header->fileSize - f->header->dataOffset - size);
      fclose(fp);
      if (f->releaseData)
        delete[] f->releaseData;
      f->header = (stFile::stHeader *)(f->releaseData = data);
      return true;
    }
  }
  return false;
}

//--------------------------------------------------
// Key generation
//--------------------------------------------------
unsigned int clICE::GetKey(stGroup *group) {
  return group->group1.originalSize ^ group->group2.originalSize ^
         group->group2Size ^ group->key ^ 0xC8D7469A;
}
unsigned int clICE::GetKey1(unsigned int filesize, unsigned char encryptTable[],
                            unsigned int version) {
  if (!(version >= 4 && version <= 9))
    return 0;
  static const int list[] = {
      0x6C, 0x7C, 0xDC, 0x83, 0x0A, 0xD2, 0xB3, 0x50, 0x61,
      0xD7, 0x17, 0x47, 0x16, 0x54, 0x61, 0xDC, 0xBD, 0xDB,
  };
  int a = list[(version - 4) * 3 + 0];
  int b = list[(version - 4) * 3 + 1];
  int c = list[(version - 4) * 3 + 2];
  clCRC crc;
  return *(unsigned int *)&encryptTable[a] ^ filesize ^
         crc.GetCRC32(&encryptTable[b], c - b) ^ 0x4352F5C2;
}
unsigned int clICE::GetKey2(unsigned int key, unsigned char encryptTable[],
                            unsigned int version) {
  if (!(version >= 4 && version <= 9))
    return 0;
  int a1, a2, a3, a4, b1, b2, b3, b4;
  if (version == 4) {
    b1 = 8;
    b2 = 24;
    b3 = 0;
    b4 = 16;
  } else {
    b1 = 16;
    b2 = 8;
    b3 = 24;
    b4 = 0;
  }
  static const int list[] = {
      0x5D, 0x3F, 0x45, -0x3A, 0xE2, 0xC6, 0xA1, 0xF3, 0xE8, 0xAE, 0xB7, 0x64,
      0x08, 0xF9, 0x5D, 0xFD,  0xC8, 0xAA, 0x5E, 0x7A, 0x0D, 0x9C, 0xF5, 0x93,
  };
  a1 = list[(version - 4) * 4 + 0];
  a2 = list[(version - 4) * 4 + 1];
  a3 = list[(version - 4) * 4 + 2];
  a4 = list[(version - 4) * 4 + 3];
  return ((unsigned int)rol(encryptTable[((key >> 0) + a1) & 0xFF], a1 & 7)
          << b1) |
         ((unsigned int)rol(encryptTable[((key >> 8) + a2) & 0xFF], a2 & 7)
          << b2) |
         ((unsigned int)rol(encryptTable[((key >> 16) + a3) & 0xFF], a3 & 7)
          << b3) |
         ((unsigned int)rol(encryptTable[((key >> 24) + a4) & 0xFF], a4 & 7)
          << b4);
}
unsigned int clICE::GetKey3(unsigned int key, unsigned char encryptTable[],
                            unsigned int version) {
  const unsigned int key1 = 0x4352F5C2;
  if (version < 5) {
    key ^= key1 ^ _key;
    int count = (int)(((unsigned long long int)0x24924925 * key) >> 32);
    count = key - ((((key - count) >> 1) + count) >> 2) * 7 + 2;
    for (; count > 0; count--)
      key = GetKey2(key, encryptTable, version);
  } else if (version <= 9) {
    key ^= key1 ^ _key;
    int count = (int)(((unsigned long long int)0x4EC4EC4F * key) >> 32);
    count = key - (count >> 2) * 13 + 3;
    for (; count > 0; count--)
      key = GetKey2(key, encryptTable, version);
  } else {
    key = 0;
  }
  return key1 ^ _key ^ key;
}

//--------------------------------------------------
// Compression / Decompression
//--------------------------------------------------
unsigned char *clICE::Compress(unsigned char *data, unsigned int originalSize,
                               unsigned int *dataSize) {

  // Check
  if (!(data && originalSize && dataSize))
    return nullptr;

  // Compression
  unsigned int size = originalSize + originalSize / 8 + 3;
  unsigned char *d = new unsigned char[size];
  if (!d)
    return nullptr;
  clCompress cmp;
  *dataSize = cmp.Compress(d, data, size, originalSize);

  return d;
}
unsigned char *clICE::Uncompress(unsigned char *data, unsigned int dataSize,
                                 unsigned int originalSize) {

  // Check
  if (!(data && dataSize && originalSize))
    return nullptr;

  // Decompression
  unsigned char *d = new unsigned char[originalSize];
  if (!d)
    return nullptr;
  clUncompress cmp;
  if (!cmp.Uncompress(d, data, originalSize, dataSize)) {
    delete[] d;
    return nullptr;
  }

  return d;
}

//--------------------------------------------------
// Encryption / Decryption
//--------------------------------------------------
void clICE::Encrypt(void *data, unsigned int size, unsigned int version,
                    unsigned int blowFishKey1, unsigned int blowFishKey2) {
  if (data && size) {
    if (version < 4) {
      clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
      bf.Encrypt(data, size);
    } else {
      if ((version < 5 && size <= 0x19000) ||
          (version >= 5 && version <= 9 && size <= 0x25800)) {
        clBlowFish bf(&blowFishKey2, sizeof(blowFishKey2), true);
        bf.Encrypt(data, size);
      }
      {
        clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
        bf.Encrypt(data, size);
      }
      unsigned char x =
          (unsigned char)(blowFishKey1 ^
                          (blowFishKey1 >> (version < 5    ? 16
                                            : version <= 9 ? version + 5
                                                           : 0)));
      for (unsigned char *s = (unsigned char *)data, *e = s + size; s < e; s++)
        if (*s && *s != x)
          *s ^= x;
    }
  }
}
void clICE::Decrypt(void *data, unsigned int size, unsigned int version,
                    unsigned int blowFishKey1, unsigned int blowFishKey2) {
  if (data && size) {
    if (version < 4) {
      clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
      bf.Decrypt(data, size);
    } else {
      unsigned char x =
          (unsigned char)(blowFishKey1 ^
                          (blowFishKey1 >> (version < 5    ? 16
                                            : version <= 9 ? version + 5
                                                           : 0)));
      for (unsigned char *s = (unsigned char *)data, *e = s + size; s < e; s++)
        if (*s && *s != x)
          *s ^= x;
      {
        clBlowFish bf(&blowFishKey1, sizeof(blowFishKey1), true);
        bf.Decrypt(data, size);
      }
      if ((version < 5 && size <= 0x19000) ||
          (version >= 5 && version <= 9 && size <= 0x25800)) {
        clBlowFish bf(&blowFishKey2, sizeof(blowFishKey2), true);
        bf.Decrypt(data, size);
      }
    }
  }
}
