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
	map<DWORD, long> dwHandle;//��һ���������ļ����ص�Ŀ¼��ƫ�ƣ��ڶ������ļ���д�Ĵ�ͷ����λ��
};

#endif // !DWHANDLE_H