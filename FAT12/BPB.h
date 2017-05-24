#pragma once
#ifndef BPB_H
#define BPB_H

typedef char u8;
typedef char u16[2];
typedef char u32[4];

class BPB {
public:
	BPB() {};
	BPB(char bpbBuffer[25]);
	~BPB() {};
	int getBytePerSec();//获得每扇区字节数
	int getSecPerClus();//获得每簇扇区数
	int getRscdSecCnt();//获得Boot占用扇区数
	int getNumFATs();//获得FAT表个数
	int getRootEntCnt();//获得根目录最大文件数
	int getFATSz16();//获得每FAT扇区数

private:
	u16 BPB_BytsPerSec;
	u8 BPB_SecPerClus;
	u16 BPB_RsvdSecCnt;
	u8 BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_TotSec16;
	u8 BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32 BPB_HiddSec;
	u32 BPB_TotSec32;
};

#endif // !BPB_H