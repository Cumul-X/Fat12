#include "stdafx.h"

BPB::BPB(char bpbBuffer[25]) {
	BPB_BytsPerSec[0] = bpbBuffer[0];
	BPB_BytsPerSec[1] = bpbBuffer[1];
	BPB_SecPerClus = bpbBuffer[2];
	BPB_RsvdSecCnt[0] = bpbBuffer[3];
	BPB_RsvdSecCnt[1] = bpbBuffer[4];
	BPB_NumFATs = bpbBuffer[5];
	BPB_RootEntCnt[1] = bpbBuffer[6];
	BPB_RootEntCnt[2] = bpbBuffer[7];
	BPB_FATSz16[0] = bpbBuffer[11];
	BPB_FATSz16[1] = bpbBuffer[12];
}

int BPB::getBytePerSec() {
	int i = BPB_BytsPerSec[0]+BPB_BytsPerSec[1]*256;
	return i;
}

int BPB::getSecPerClus() {
	return BPB_SecPerClus;
}

int BPB::getRscdSecCnt() {
	int i = BPB_RsvdSecCnt[0] + BPB_RsvdSecCnt[1] * 256;
	return i;
}

int BPB::getNumFATs() {
	return BPB_NumFATs;
}

int BPB::getRootEntCnt() {
	int i = BPB_RootEntCnt[0];
	i = i&0x000000ff;
	i += BPB_RootEntCnt[1] * 256;
	return i;
}

int BPB::getFATSz16() {
	int i = BPB_FATSz16[0] + BPB_FATSz16[1] * 256;
	return i;
}
