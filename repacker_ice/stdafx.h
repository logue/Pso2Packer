// stdafx.h : 標準のシステム インクルード ファイルのインクルード
// ファイル、または参照回数が多く、かつあまり変更されない、
// プロジェクト専用のインクルードファイルを記述します。
#pragma once
#define WIN32_LEAN_AND_MEAN

// Windows ヘッダー ファイル:
#include <windows.h>
#include <winternl.h>

// C ランタイム ヘッダー ファイル
#include <direct.h> // _mkdir
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "targetver.h"

// Macro
#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define BitsCount(val) (sizeof(val) * CHAR_BIT)
#define Shift(val, steps) (steps % BitsCount(val))
#define ROL(val, steps)                                                        \
  ((val << Shift(val, steps)) | (val >> (BitsCount(val) - Shift(val, steps))))
#define ROR(val, steps)                                                        \
  ((val >> Shift(val, steps)) | (val << (BitsCount(val) - Shift(val, steps))))

inline unsigned int bswap(unsigned int v) {
  return (v << 0x18) | ((v & 0x0000ff00) << 0x08) | ((v & 0x00ff0000) >> 0x08) |
         (v >> 0x18);
}

#define VERSION_MAJOR 1
#define VERSION_MINOR 3
#define VERSION_BUILD 1
#define VERSION_REVISION 1

#define LIST_FILENAME "FileList.txt"
#define ICE_VERSION 4
#define CRYPT_KEY 0xCD50379E
#define SIGNATURE 0x00454349
