#include "stdafx.h"
#include <stdio.h>

#define IDI_ICON1 101

#define VER_FILEVERSION                                                        \
  VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION
#define VER_FILEVERSION_STR                                                    \
  VERSION_MAJOR "." VERSION_MINOR "." VERSION_BUILD "." VERSION_REVISION "\0"

#define VER_PRODUCTVERSION                                                     \
  VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION
#define VER_PRODUCTVERSION_STR                                                 \
  VERSION_MAJOR "." VERSION_MINOR "." VERSION_BUILD "." VERSION_REVISION "\0"

#ifndef DEBUG
#define VER_DEBUG 0
#else
#define VER_DEBUG VS_FF_DEBUG
#endif

#ifdef _WIN64
#define VER_FILE_DESC                                                          \
  "Phantasy Star Online 2 Ice File Encrypter/Decrypter (x64)"
#else
#define VER_FILE_DESC                                                          \
  "Phantasy Star Online 2 Ice File Encrypter/Decrypter (x86)"
#endif
