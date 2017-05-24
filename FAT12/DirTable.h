#pragma once
#ifndef DIRTABLE_H
#define DIRTABLE_H

typedef char u8;
typedef char u16[2];
typedef char u32[4];

class DirTable {
public:
	DirTable() {};
	DirTable(char dirBuffer[32]);
	DirTable(string dirName, int dirAttr, int dirFstClus);
	DirTable(string dirName, int dirAttr, int dirFstClus, long dirFileSize);
	~DirTable() {};
	int getFstClus();//首簇号
	char *getDirName();//文件或者目录名
	int getDirAttr();//文件属性
	long getDirFileSize();

private:
	char DIR_Name[11];
	u8 DIR_Attr;
	char Reserced[10];
	u16 DIR_WrtTime;
	u16 DIR_WrtDate;
	u16 DIR_FstClus;
	long DIR_FileSize;
};

#endif // !DIRTABLE_H