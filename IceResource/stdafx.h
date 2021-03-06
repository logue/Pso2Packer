// stdafx.h : 標準のシステム インクルード ファイルのインクルードファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルードファイル
// を記述します。

#pragma once

#include "targetver.h"

// Windows ヘッダーから使用されていない部分を除外します。
#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS
// Windows ヘッダー ファイル:
#include <windows.h>
#include <winnt.h>
#include <ntstatus.h>
#include <winternl.h>

// C ランタイム ヘッダー ファイル
#include <direct.h> // _mkdir
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <algorithm>
#include <map>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define VERSION "1.30"
#define LIST_FILENAME "FileList.txt"
#define ICE_VERSION 4
#define CRYPT_KEY 0xCD50379E
#define SIGNATURE 0x00454349
