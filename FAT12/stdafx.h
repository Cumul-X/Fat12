// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define MY_FILE_BEGIN 0
#define MY_FILE_CURRENT 1
#define MY_FILE_END 2

#include "targetver.h"
#include "Fat.h"
#include "DiskLib.h"
#include "BPB.h"
#include "DwHandle.h"
#include "DirTable.h"

#include <stdio.h>
#include <tchar.h>
#include <ctime>
#include <sstream>
#include <bitset>

using std::bitset;
using std::stringstream;

// TODO:  在此处引用程序需要的其他头文件
