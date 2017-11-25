// stdafx.h : 標準のシステム インクルード ファイルのインクルード
// ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード
// ファイル
// を記述します。
//

#pragma once
#define WIN32_LEAN_AND_MEAN

#include "targetver.h"

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

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define VERSION "1.30"
#define LIST_FILENAME "FileList.txt"
#define ICE_VERSION 4
#define CRYPT_KEY 0xCD50379E
#define SIGNATURE 0x00454349

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
