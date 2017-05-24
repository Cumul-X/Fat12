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
	int getBytePerSec();//���ÿ�����ֽ���
	int getSecPerClus();//���ÿ��������
	int getRscdSecCnt();//���Bootռ��������
	int getNumFATs();//���FAT�����
	int getRootEntCnt();//��ø�Ŀ¼����ļ���
	int getFATSz16();//���ÿFAT������

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