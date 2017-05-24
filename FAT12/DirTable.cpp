#include "stdafx.h"
#include "DirTable.h"

DirTable::DirTable(char dirBuffer[32]) {
	for (int i = 0; i < 11; i++) {
		DIR_Name[i] = dirBuffer[i];
	}
	DIR_Attr = dirBuffer[11];
	DIR_FstClus[0] = dirBuffer[26];
	DIR_FstClus[1] = dirBuffer[27];
	long size = 0;
	size += dirBuffer[28];
	size += dirBuffer[29]*256;
	size += dirBuffer[30]*256*256;
	size += dirBuffer[31]*256*256*256;
	DIR_FileSize = size;
}

DirTable::DirTable(string dirName, int dirAttr, int dirFstClus) {
	for (int i = 0; i < dirName.size(); i++) {
		DIR_Name[i] = dirName[i];
	}
	for (int i = dirName.size(); i < 11; i++) {
		DIR_Name[i] = ' ';
	}
	DIR_Attr = dirAttr;
	time_t tt = time(NULL);//这句返回的只是一个时间戳
	unsigned long time, date;
	unsigned short hour, min, sec;
	unsigned short year, mon, day;
	tm* t = localtime(&tt);
	hour = t->tm_hour;
	min = t->tm_min;
	sec = t->tm_sec/2;
	bitset<16> bitTime = bitset<16>(bitset<5>(hour).to_string() + bitset<6>(min).to_string() + bitset<5>(sec).to_string());
	time = bitTime.to_ulong();
	DIR_WrtTime[0] = time % 256;
	DIR_WrtTime[1] = time / 256;
	year = t->tm_year - 80;
	mon = t->tm_mon + 1;
	day = t->tm_mday;
	bitset<16> bitDate = bitset<16>(bitset<7>(year).to_string() + bitset<4>(mon).to_string() + bitset<5>(day).to_string());
	date = bitDate.to_ulong();
	DIR_WrtDate[0] = date % 256;
	DIR_WrtDate[1] = date / 256;
	
	DIR_FstClus[0] = dirFstClus % 256;
	DIR_FstClus[1] = dirFstClus / 256;
	DIR_FileSize = 0x0;
	for (int i = 0; i < 10; i++) {
		Reserced[i] = 0x0;
	}
}

DirTable::DirTable(string dirName, int dirAttr, int dirFstClus, long dirFileSize) {
	for (int i = 0; i < dirName.size(); i++) {
		DIR_Name[i] = dirName[i];
	}
	for (int i = dirName.size(); i < 11; i++) {
		DIR_Name[i] = ' ';
	}
	DIR_Attr = dirAttr;
	time_t tt = time(NULL);//这句返回的只是一个时间戳
	unsigned long time, date;
	unsigned short hour, min, sec;
	unsigned short year, mon, day;
	tm* t = localtime(&tt);
	hour = t->tm_hour;
	min = t->tm_min;
	sec = t->tm_sec / 2;
	bitset<16> bitTime = bitset<16>(bitset<5>(hour).to_string() + bitset<6>(min).to_string() + bitset<5>(sec).to_string());
	time = bitTime.to_ulong();
	DIR_WrtTime[0] = time % 256;
	DIR_WrtTime[1] = time / 256;
	year = t->tm_year - 80;
	mon = t->tm_mon + 1;
	day = t->tm_mday;
	bitset<16> bitDate = bitset<16>(bitset<7>(year).to_string() + bitset<4>(mon).to_string() + bitset<5>(day).to_string());
	date = bitDate.to_ulong();
	DIR_WrtDate[0] = date % 256;
	DIR_WrtDate[1] = date / 256;
	DIR_FstClus[0] = dirFstClus % 256;
	DIR_FstClus[1] = dirFstClus / 256;
	DIR_FileSize = dirFileSize;
	for (int i = 0; i < 10; i++) {
		Reserced[i] = 0x0;
	}
}

int DirTable::getFstClus() {
	int i = DIR_FstClus[0] + DIR_FstClus[1] * 256;
	return i;
}

char * DirTable::getDirName() {
	return DIR_Name;
}

int DirTable::getDirAttr() {
	return DIR_Attr;
}

long DirTable::getDirFileSize() {
	return DIR_FileSize;
}
