#pragma once
#ifndef DWHANDLE_H
#define DWHANDLE_H

#include <windows.h>
#include <string>
#include <map>
using std::map;
using std::string;

class DwHandle {
public:
	DwHandle() {};
	~DwHandle() {};
	long getOffset(DWORD fileHandle);
	void addHandle(DWORD fileHandle);
	void deleteHandle(DWORD fileHandle);
	void alterHandle(DWORD fileHandle, long offset);
	bool searchHandle(DWORD fileHandle);

private:
	map<DWORD, long> dwHandle;//第一个是生成文件返回的目录项偏移，第二个是文件读写的磁头绝对位置
};

#endif // !DWHANDLE_H