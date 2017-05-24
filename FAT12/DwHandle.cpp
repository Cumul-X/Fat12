#include "stdafx.h"
#include "DwHandle.h"

long DwHandle::getOffset(DWORD fileHandle) {
	return dwHandle.at(fileHandle);
}

void DwHandle::addHandle(DWORD fileHandle) {
	dwHandle.insert({ fileHandle,0 });
}

void DwHandle::deleteHandle(DWORD fileHandle) {
	dwHandle.erase(fileHandle);
}

void DwHandle::alterHandle(DWORD fileHandle, long offset) {
	//dwHandle[fileHandle] = offset;
	auto it = dwHandle.find(fileHandle);
	if (it == dwHandle.end()) {
		dwHandle.insert({ fileHandle, offset });
	}
	else {
		it->second = offset;
	}
}

bool DwHandle::searchHandle(DWORD fileHandle) {
	auto it = dwHandle.find(fileHandle);
	if (it == dwHandle.end()) {
		return false;
	}
	else {
		return true;
	}
}
