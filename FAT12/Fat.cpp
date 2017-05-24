#include "stdafx.h"

int bytePerSec = 0;
int secPerClus = 0;
int rscdSecCnt = 0;
int numFATs = 0;
int rootEntCnt = 0;
int FATSz16 = 0;
DwHandle handles = DwHandle();
bitset<12> fat[4096];

void init() {
	unsigned char fatBuffer[6144];
	//char bpbBuffer[25];
	BPB bpb;

	SetHeaderOffset(11, NULL, FILE_BEGIN);
	ReadFromDisk(&bpb, 25, NULL);
	bytePerSec = bpb.getBytePerSec();
	secPerClus = bpb.getSecPerClus();
	rscdSecCnt = bpb.getRscdSecCnt();
	numFATs = bpb.getNumFATs();
	rootEntCnt = bpb.getRootEntCnt();
	FATSz16 = bpb.getFATSz16();
	//bpb = BPB(bpbBuffer);
	SetHeaderOffset(bytePerSec*rscdSecCnt, NULL, FILE_BEGIN);
	ReadFromDisk(fatBuffer, FATSz16*bytePerSec, NULL);
	for (int i = 0; i < 4096; i++) {
		int j = i + i / 2;
		char tempBuffer[2];
		if (j % 3 == 0) {
			tempBuffer[0] = fatBuffer[j];
			tempBuffer[1] = fatBuffer[j + 1];
			unsigned short tempClus = tempBuffer[0];
			tempClus &= 0x00ff;
		    tempClus += tempBuffer[1] * 256;
			tempClus = tempClus & 0x0fff;
			fat[i] = bitset<12>(tempClus);
		}
		else if(j%3==1){
			tempBuffer[0] = fatBuffer[j];
			tempBuffer[1] = fatBuffer[j+1];
			unsigned short tempClus = tempBuffer[0];
			tempClus &= 0x00ff;
			tempClus += tempBuffer[1] * 256;
			tempClus = tempClus >> 4;
			fat[i] = bitset<12>(tempClus);
		}
	}
}

//分配一个簇，并接上传入的簇
int findNextClus(int firstClus) {
	long offset;
	int clusId;
	for (int i = 2; i < 4096; i++) {
		offset = bytePerSec*rscdSecCnt;
		bitset<12> tempClus = fat[i];
		if (tempClus.to_ulong() == 0) {
			clusId = i;
			bitset<12> endClus(0xfff);
			string twoClus;
			if (i % 2 == 1) {
				twoClus = endClus.to_string() + fat[i - 1].to_string();
			}
			else {
				twoClus = fat[i + 1].to_string() + endClus.to_string();
			}
			bitset<24> doubleClus(twoClus);
			bitset<24> *clusPtr = &doubleClus;
			fat[i] = bitset<12>(0xfff);
			offset += 3 * (i / 2);
			for (int j = 0; j < numFATs; j++) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				WriteToDisk(clusPtr, 3, NULL);
				offset += FATSz16*bytePerSec;
			}
			break;
		}
	}
	if (firstClus != 0) {
		offset = bytePerSec*rscdSecCnt;
		bitset<12> endClus(clusId);
		string twoClus;
		if (firstClus % 2 == 1) {
			twoClus = endClus.to_string() + fat[firstClus - 1].to_string();
		}
		else {
			twoClus = fat[firstClus + 1].to_string() + endClus.to_string();
		}
		bitset<24> doubleClus(twoClus);
		bitset<24> *clusPtr = &doubleClus;
		offset += 3 * (firstClus / 2);
		for (int j = 0; j < numFATs; j++) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			WriteToDisk(clusPtr, 3, NULL);
			offset += FATSz16*bytePerSec;
		}
	}
	return clusId;
}

//清零传入的簇链
void deleteClus(int firstClus) {
	long offset = bytePerSec*rscdSecCnt;

	int next = fat[firstClus].to_ulong();
	bitset<12> thisClus(0x0);
	string twoClus;
	if (firstClus % 2 == 1) {
		twoClus = thisClus.to_string() + fat[firstClus - 1].to_string();
	}
	else {
		twoClus = fat[firstClus + 1].to_string() + thisClus.to_string();
	}
	bitset<24> doubleClus(twoClus);
	bitset<24> *clusPtr = &doubleClus;
	fat[firstClus] = bitset<12>(0x0);
	offset += 3 * (firstClus / 2);
	for (int j = 0; j < numFATs; j++) {
		SetHeaderOffset(offset, NULL, FILE_BEGIN);
		WriteToDisk(clusPtr, 3, NULL);
		offset += FATSz16*bytePerSec;
	}
	if (next != 0xfff) {
		deleteClus(next);
	}
}

//遍历当前目录的所有子目录并找到传入目录名，目录存在返回目录的目录项偏移地址，否则返回0
DWORD searchFolder(const char* pszFolderPath, int dirFstClus) {
	stringstream folderStream;
	stringstream sstream;
	folderStream << pszFolderPath;
	string tempDirName;
	std::getline(folderStream, tempDirName, '\\');
	string nextFolderPath;
	std::getline(folderStream, nextFolderPath,'\\');

	bitset<12> tempClus(dirFstClus);
	
	int count;//每个簇的目录项个数
	long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
	long offset;

	while (tempClus.to_ulong() != 0xfff) {
		if (tempClus.to_ulong() != 0x0) {
			count = secPerClus*bytePerSec / 32;
			offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			tempClus = fat[tempClus.to_ulong()];
		}
		else {
			offset = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec;
			count = rootEntCnt;
			tempClus = bitset<12>(0xfff);
		}
		while (count > 0 ) {
			char dirBuffer[32];
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			DirTable dirTable(dirBuffer);
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			sstream >> aString;
			if (dirTable.getDirAttr() == 0x10 && tempDirName == aString) {
				if (!nextFolderPath.empty()) {
					const char *c_nextFolderPath = nextFolderPath.c_str();
					return searchFolder(c_nextFolderPath, dirTable.getFstClus());
				}
				else {
					return offset;
				}
			}
			sstream.str("");
			offset += 32;
			count--;
		}
	}
	return 0;
}

//给定目录的目录项的偏移地址，查找该目录下是否存在相应文件或目录，存在返回文件目录项偏移地址，否则返回0
DWORD searchFile(long dirOffset, char * pszFileName, int dirAttr) {
	stringstream fileStream;
	stringstream sstream;
	fileStream << pszFileName;
	string dirName;
	if (dirAttr == 0x20) {
		string fileName, fileExension;
		std::getline(fileStream, fileName, '.');
		std::getline(fileStream, fileExension, '.');
		for (int i = fileName.size(); i < 8; i++) {
			fileName += ' ';
		}
		for (int i = fileExension.size(); i < 3; i++) {
			fileExension += ' ';
		}
		dirName = string(fileName.begin(),fileName.begin()+8) + string(fileExension.begin(),fileExension.begin()+3);
	}
	else if (dirAttr == 0x10) {
		dirName = fileStream.str();
	}

	if (dirOffset != (rscdSecCnt + FATSz16 * numFATs)*bytePerSec) {
		char dirBuffer[32];
		SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		DirTable dirTable(dirBuffer);
		bitset<12> tempClus(dirTable.getFstClus());
		int count;//每个簇的目录项个数
		long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
		long offset;

		while (tempClus.to_ulong() != 0xfff) {
			count = secPerClus*bytePerSec / 32;
			offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			tempClus = fat[tempClus.to_ulong()];
			while (count > 0) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(dirBuffer, 32, NULL);
				dirTable = DirTable(dirBuffer);
				for (int i = 0; i < 11; i++) {
					sstream << *(dirTable.getDirName()+i);
				}
				string aString;
				//sstream >> aString;
				if (dirTable.getDirAttr() == 0x20) {
					aString = sstream.str();
				}
				else if (dirTable.getDirAttr() == 0x10) {
					sstream >> aString;
				}
				if (dirTable.getDirAttr() == dirAttr && dirName == aString) {
					return offset;
				}
				sstream.str("");
				offset += 32;
				count--;
			}
		}
	}
	else {
		int count = rootEntCnt;
		long offset = dirOffset;
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			if (dirAttr == 0x20) {
				aString = sstream.str();
			}
			else if (dirAttr == 0x10) {
				sstream >> aString;
			}
			if (dirTable.getDirAttr() == dirAttr && dirName == aString) {
				return offset;
			}
			sstream.str("");
			offset += 32;
			count--;
		}
	}
	return 0;
}

//给定目录的目录项的偏移地址，在该目录下生成相应文件，返回文件目录项偏移地址
DWORD createFile(long dirOffset, char * pszFileName) {
	stringstream fileStream;
	fileStream << pszFileName;
	string fileName, fileExension;
	std::getline(fileStream, fileName, '.');
	std::getline(fileStream, fileExension, '.');
	for (int i = fileName.size(); i < 8; i++) {
		fileName += ' ';
	}
	for (int i = fileExension.size(); i < 3; i++) {
		fileExension += ' ';
	}
	string dirName = string(fileName.begin(), fileName.begin() + 8) + string(fileExension.begin(), fileExension.begin() + 3);

	DirTable dirTable;
	if (dirOffset != (rscdSecCnt + FATSz16 * numFATs)*bytePerSec) {
		char dirBuffer[32];
		SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		dirTable = DirTable(dirBuffer);
		bitset<12> tempClus(dirTable.getFstClus());
		int count;//每个簇的目录项个数
		long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
		long offset;
		bitset<12> lastClus;
		while (tempClus.to_ulong() != 0xfff) {
			count = secPerClus*bytePerSec / 32;
			offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			while (count > 0) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(dirBuffer, 32, NULL);
				dirTable = DirTable(dirBuffer);
				char firstByte =  *(dirTable.getDirName());
				if (firstByte == 0x0 || firstByte == 0x5e) {
					DirTable newDirTable(dirName, 0x20, findNextClus(0));
					//DirTable *ditTablePtr = &newDirTable;
					SetHeaderOffset(offset, NULL, FILE_BEGIN);
					WriteToDisk(&newDirTable, 32, NULL);
					return offset;
				}
				offset += 32;
				count--;
			}
			lastClus = tempClus;
			tempClus = fat[tempClus.to_ulong()];
		}
		offset = bytePerSec*secPerClus*(findNextClus(lastClus.to_ulong()) - 2) + offsetOfFirstData;
		DirTable newDirTable(dirName, 0x20, findNextClus(0));
		//DirTable *dirTablePtr = &newDirTable;
		SetHeaderOffset(offset, NULL, FILE_BEGIN);
		WriteToDisk(&newDirTable, 32, NULL);
		return offset;
	}
	else {
		char dirBuffer[32];
		int count = rootEntCnt;
		long offset = dirOffset;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			char firstByte = *(dirTable.getDirName());
			if (firstByte == 0x0 || firstByte == 0x5e) {
				DirTable newDirTable(dirName, 0x20, findNextClus(0));
				//DirTable *ditTablePtr = &newDirTable;
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				WriteToDisk(&newDirTable, 32, NULL);
				return offset;
			}
			offset += 32;
			count--;
		}
	}
	return 0;
}

//给定目录的目录项的偏移地址，在该目录下生成相应目录，返回目录目录项偏移地址
DWORD createDirectory(long dirOffset, char *pszFileName) {
	stringstream fileStream;
	fileStream << pszFileName;
	string dirName = fileStream.str();
	long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;

	if (dirOffset != (rscdSecCnt + FATSz16 * numFATs)*bytePerSec) {
		char dirBuffer[32];
		SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		DirTable dirTable(dirBuffer);
		bitset<12> tempClus(dirTable.getFstClus());
		int count;//每个簇的目录项个数
		long offset;
		bitset<12> lastClus;
		int firstClus = tempClus.to_ulong();
		while (tempClus.to_ulong() != 0xfff) {
			count = secPerClus*bytePerSec / 32;
			offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			while (count > 0) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(dirBuffer, 32, NULL);
				dirTable = DirTable(dirBuffer);
				char firstByte = *(dirTable.getDirName());
				if (firstByte == 0x0 || firstByte == 0x5e) {
					int dirClus = findNextClus(0);
					DirTable newDirTable(dirName, 0x10, dirClus);
					//DirTable *ditTablePtr = &newDirTable;
					SetHeaderOffset(offset, NULL, FILE_BEGIN);
					WriteToDisk(&newDirTable, 32, NULL);
					long diroffset = bytePerSec*secPerClus*(dirClus - 2) + offsetOfFirstData;
					newDirTable =  DirTable(string("."), 0x10, dirClus);
					SetHeaderOffset(diroffset, NULL, FILE_BEGIN);
					WriteToDisk(&newDirTable, 32, NULL);
					newDirTable = DirTable(string(".."), 0x10, firstClus);
					SetHeaderOffset(diroffset+32, NULL, FILE_BEGIN);
					WriteToDisk(&newDirTable, 32, NULL);
					return offset;
				}
				offset += 32;
				count--;
			}
			lastClus = tempClus;
			tempClus = fat[tempClus.to_ulong()];
		}
		offset = bytePerSec*secPerClus*(findNextClus(lastClus.to_ulong()) - 2) + offsetOfFirstData;
		int dirClus = findNextClus(0);
		DirTable newDirTable(dirName, 0x10, dirClus);
		//DirTable *ditTablePtr = &newDirTable;
		SetHeaderOffset(offset, NULL, FILE_BEGIN);
		WriteToDisk(&newDirTable, 32, NULL);
		long diroffset = bytePerSec*secPerClus*(dirClus - 2) + offsetOfFirstData;
		newDirTable = DirTable(string("."), 0x10, dirClus);
		SetHeaderOffset(diroffset, NULL, FILE_BEGIN);
		WriteToDisk(&newDirTable, 32, NULL);
		newDirTable = DirTable(string(".."), 0x10, firstClus);
		SetHeaderOffset(diroffset + 32, NULL, FILE_BEGIN);
		WriteToDisk(&newDirTable, 32, NULL);
		return offset;
	}
	else {
		int count = rootEntCnt;
		long offset = dirOffset;
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			char firstByte = *(dirTable.getDirName());
			if (firstByte == 0x0 || firstByte == 0x5e) {
				int dirClus = findNextClus(0);
				DirTable newDirTable(dirName, 0x10, dirClus);
				//DirTable *ditTablePtr = &newDirTable;
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				WriteToDisk(&newDirTable, 32, NULL);
				long diroffset = bytePerSec*secPerClus*(dirClus - 2) + offsetOfFirstData;
				newDirTable = DirTable(string("."), 0x10, dirClus);
				SetHeaderOffset(diroffset, NULL, FILE_BEGIN);
				WriteToDisk(&newDirTable, 32, NULL);
				newDirTable = DirTable(string(".."), 0x10, 0x0);
				SetHeaderOffset(diroffset + 32, NULL, FILE_BEGIN);
				WriteToDisk(&newDirTable, 32, NULL);
				return offset;
			}
			offset += 32;
			count--;
		}
	}
	return 0;
}

//给定文件的目录项的偏移地址，删除该文件，失败返回0
BOOL deleteFile(long dirOffset) {
	char dirBuffer[32];
	SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
	ReadFromDisk(dirBuffer, 32, NULL);
	DirTable dirTable(dirBuffer);
	char firstByte = 0x5e;
	stringstream sstream;
	sstream << firstByte;
	for (int i = 1; i < 11; i++) {
		sstream << *(dirTable.getDirName() + i);
	}
	string dirName = sstream.str();
	DirTable newDirTable(dirName, 0x10, dirTable.getFstClus());
	//DirTable *ditTablePtr = &newDirTable;
	SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
	WriteToDisk(&newDirTable, 32, NULL);
	deleteClus(dirTable.getFstClus());
	return TRUE;
}

//给定目录的目录项的偏移地址，删除该目录，失败返回0
BOOL deleteDirectory(long dirOffset) {
	char dirBuffer[32];
	SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
	ReadFromDisk(dirBuffer, 32, NULL);
	DirTable dirTable(dirBuffer);
	char firstByte = 0x5e;
	stringstream sstream;
	sstream << firstByte;
	for (int i = 1; i < 11; i++) {
		sstream << *(dirTable.getDirName() + i);
	}
	string dirName = sstream.str();
	DirTable newDirTable(dirName, 0x10, dirTable.getFstClus());
	//DirTable *ditTablePtr = &newDirTable;
	SetHeaderOffset(dirOffset, NULL, FILE_BEGIN);
	WriteToDisk(&newDirTable, 32, NULL);
	bitset<12> tempClus(dirTable.getFstClus());
	int count;
	long offset;
	long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
	stringstream s;
	for (int i = 0; i < 11; i++) {
		s << *(dirTable.getDirName() + i);
	}
	string special;
	s >> special;
	if (special != "."&&special != "..") {
		while (tempClus.to_ulong() != 0xfff) {
			count = secPerClus*bytePerSec / 32;
			offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			tempClus = fat[tempClus.to_ulong()];
			while (count > 0) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(dirBuffer, 32, NULL);
				DirTable adirTable(dirBuffer);
				char afirstByte = *(adirTable.getDirName());
				if (afirstByte != 0x0 && afirstByte != 0x5e) {
					if (adirTable.getDirAttr() == 0x20)
						deleteFile(offset);
					else if (adirTable.getDirAttr() == 0x10) {
						deleteDirectory(offset);
					}
				}
				sstream.str("");
				offset += 32;
				count--;
			}
		}
		deleteClus(dirTable.getFstClus());
	}
	return TRUE;
}

DWORD MyCreateFile(char * pszFolderPath, char * pszFileName) {
	init();
	long offset = (rscdSecCnt + FATSz16*numFATs)*bytePerSec;
	int count = rootEntCnt;
	long offsetOfFolder = 0;

	stringstream folderStream;
	folderStream << pszFolderPath;
	if (folderStream.str() != string("C:")) {
		string tempDirName;
		string folderPath = folderStream.str();
		std::getline(folderStream, tempDirName, '\\');
		folderPath = string(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
		std::getline(folderStream, tempDirName, '\\');
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			stringstream sstream;
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			if (dirTable.getDirAttr() == 0x20) {
				aString = sstream.str();
			}
			else if (dirTable.getDirAttr() == 0x10) {
				sstream >> aString;
			}
			if (dirTable.getDirAttr() == 0x10 && tempDirName == aString) {
				if (folderPath.begin() + tempDirName.size() != folderPath.end()) {
					string nextFolderPath(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
					const char *c_nextFolderPath = nextFolderPath.c_str();
					offsetOfFolder = searchFolder(c_nextFolderPath, dirTable.getFstClus());
					break;
				}
				else {
					offsetOfFolder = offset;
					break;
				}
			}
			sstream.str("");
			offset += 32;
			count--;
		}
		if (offsetOfFolder != 0) {
			if (searchFile(offsetOfFolder, pszFileName,0x20) == 0) {
				long handle = createFile(offsetOfFolder, pszFileName);
				return handle;
			}
		}
		else {
			return 0;
		}
	}
	else {
		offsetOfFolder = offset;
		if (searchFile(offsetOfFolder, pszFileName,0x20) == 0) {
			long handle = createFile(offsetOfFolder, pszFileName);
			return handle;
		}
		else {
			return 0;
		}
	}
	return 0;
}

DWORD MyOpenFile(char * pszFolderPath, char * pszFileName) {
	init();
	long offset = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec;
	int count = rootEntCnt;

	long offsetOfFolder = 0;
	stringstream folderStream;
	folderStream << pszFolderPath;
	if (folderStream.str()!=string("C:")) {
		stringstream sstream;
		string tempDirName;
		string folderPath = folderStream.str();
		std::getline(folderStream, tempDirName, '\\');
		folderPath = string(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
		std::getline(folderStream, tempDirName, '\\');
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			sstream >> aString;
			if (dirTable.getDirAttr() == 0x10 && tempDirName == aString) {
				if (folderPath.begin() + tempDirName.size() != folderPath.end()) {
					string nextFolderPath(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
					const char *c_nextFolderPath = nextFolderPath.c_str();
					offsetOfFolder = searchFolder(c_nextFolderPath, dirTable.getFstClus());
					break;
				}
				else {
					offsetOfFolder = offset;
					break;
				}
			}
			sstream.str("");
			offset += 32;
			count--;
		}
		if (offsetOfFolder != 0) {
			DWORD fileHandle = searchFile(offsetOfFolder, pszFileName, 0x20);
			if (fileHandle != 0) {
				handles.addHandle(fileHandle);
			}
			return fileHandle;
		}
		else {
			return 0;
		}
	}
	else {
		offsetOfFolder = offset;
		DWORD fileHandle = searchFile(offsetOfFolder, pszFileName, 0x20);
		if (fileHandle != 0) {
			handles.addHandle(fileHandle);
		}
		return fileHandle;
	}
}

void MyCloseFile(DWORD dwHandle) {
	handles.deleteHandle(dwHandle);
}

BOOL MyDeleteFile(char * pszFolderPath, char * pszFileName) {
	long offset = MyOpenFile(pszFolderPath, pszFileName);
	if (offset != 0) {
		BOOL isDelete = deleteFile(offset);
		handles.deleteHandle(offset);
		return isDelete;
	}
	else {
		return 0;
	}
}

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	if (handles.searchHandle(dwHandle)) {
		int bytePerClus = bytePerSec*secPerClus;
		long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
		long filePionter = handles.getOffset(dwHandle);
		char *tempBuffer = new char[dwBytesToWrite + filePionter];
		MySetFilePointer(dwHandle, 0, MY_FILE_BEGIN);
		MyReadFile(dwHandle, tempBuffer, filePionter);
		char dirBuffer[32];
		SetHeaderOffset(dwHandle, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		DirTable dirTable(dirBuffer);
		stringstream sstream;
		for (int i = 0; i < 11; i++) {
			sstream << *(dirTable.getDirName() + i);
		}
		memcpy(tempBuffer + filePionter, pBuffer, dwBytesToWrite);
		char * buffer = tempBuffer;
		long fileSize = dwBytesToWrite + filePionter;
		long lastByte = fileSize;
		int firstClus = findNextClus(0);
		int nextClus = firstClus;
		long offset;
		while (lastByte > bytePerClus) {
			offset = bytePerSec*secPerClus*(nextClus - 2) + offsetOfFirstData;
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			WriteToDisk(buffer, bytePerClus, NULL);
			lastByte -= bytePerClus;
			nextClus = findNextClus(nextClus);
			buffer += bytePerClus;
		}
		offset = bytePerSec*secPerClus*(nextClus - 2) + offsetOfFirstData;
		SetHeaderOffset(offset, NULL, FILE_BEGIN);
		WriteToDisk(buffer, lastByte, NULL);
		deleteFile(dwHandle);
		DirTable newDirTable = DirTable(sstream.str(), dirTable.getDirAttr(), firstClus, fileSize);
		SetHeaderOffset(dwHandle, NULL, FILE_BEGIN);
		WriteToDisk(&newDirTable, 32, NULL);
		delete[] tempBuffer;
		return dwBytesToWrite;
	}
	else {
		return -1;
	}
}

DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead) {
	int bytePerClus = bytePerSec*secPerClus;
	long offsetOfFirstData = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec + 32 * rootEntCnt;
	char * tempBuffer = new char[dwBytesToRead];
	if (handles.searchHandle(dwHandle)) {
		long filePionter = handles.getOffset(dwHandle);
		int clusNum = filePionter / bytePerClus;
		long offset = filePionter % bytePerClus;

		char dirBuffer[32];
		SetHeaderOffset(dwHandle, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		DirTable dirTable(dirBuffer);
		long fileSize = dirTable.getDirFileSize();
		bitset<12> tempClus(dirTable.getFstClus());
		for (int i = 0; i < clusNum; i++) {
			tempClus = fat[tempClus.to_ulong()];
		}
		if (dwBytesToRead > fileSize - filePionter) {
			offset += bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(tempBuffer, bytePerClus - filePionter%bytePerClus, NULL);
			char *buffer = tempBuffer + (bytePerClus - filePionter%bytePerClus);
			tempClus = fat[tempClus.to_ulong()];
			while (tempClus.to_ulong() != 0xfff) {
				offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(buffer, bytePerClus, NULL);
				tempClus = fat[tempClus.to_ulong()];
				buffer += bytePerClus;
			}
			memcpy(pBuffer, tempBuffer, fileSize - filePionter);
			delete[] tempBuffer;
			handles.alterHandle(dwHandle, fileSize);
			return fileSize - filePionter;
		}
		else {
			offset += bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
			if (dwBytesToRead <= (bytePerClus - filePionter % bytePerClus)) {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(tempBuffer, dwBytesToRead, NULL);
			}
			else {
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(tempBuffer, bytePerClus - filePionter%bytePerClus, NULL);
				char* buffer = tempBuffer + (bytePerClus - filePionter%bytePerClus);
				tempClus = fat[tempClus.to_ulong()];
				long lastByteToRead = dwBytesToRead - (bytePerClus - filePionter%bytePerClus);
				while (lastByteToRead > bytePerClus) {
					offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
					SetHeaderOffset(offset, NULL, FILE_BEGIN);
					ReadFromDisk(buffer, bytePerClus, NULL);
					tempClus = fat[tempClus.to_ulong()];
					buffer += bytePerClus;
					lastByteToRead -= bytePerClus;
				}
				offset = bytePerSec*secPerClus*(tempClus.to_ulong() - 2) + offsetOfFirstData;
				SetHeaderOffset(offset, NULL, FILE_BEGIN);
				ReadFromDisk(buffer, lastByteToRead, NULL);
			}
			memcpy(pBuffer, tempBuffer, dwBytesToRead);
			delete[] tempBuffer;
			handles.alterHandle(dwHandle, filePionter + dwBytesToRead);
			return dwBytesToRead;
		}
	}
	else {
		return -1;
	}
}

BOOL MyCreateDirectory(char * pszFolderPath, char * pszFolderName) {
	init();
	long offset = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec;
	int count = rootEntCnt;

	long offsetOfFolder = 0;
	stringstream folderStream;
	folderStream << pszFolderPath;
	if (folderStream.str() != string("C:")) {
		string tempDirName;
		string folderPath = folderStream.str();
		std::getline(folderStream, tempDirName, '\\');
		folderPath = string(folderPath.begin() + tempDirName.size()+1, folderPath.end());
		std::getline(folderStream, tempDirName, '\\');
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			stringstream sstream;
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			if (dirTable.getDirAttr() == 0x20) {
				aString = sstream.str();
			}
			else if (dirTable.getDirAttr() == 0x10) {
				sstream >> aString;
			}
			if (dirTable.getDirAttr() == 0x10 && tempDirName == aString) {
				if (folderPath.begin() + tempDirName.size() != folderPath.end()) {
					string nextFolderPath(folderPath.begin() + tempDirName.size()+1, folderPath.end());
					const char *c_nextFolderPath = nextFolderPath.c_str();
					offsetOfFolder = searchFolder(c_nextFolderPath, dirTable.getFstClus());
					break;
				}
				else {
					offsetOfFolder = offset;
					break;
				}
			}
			sstream.str("");
			offset += 32;
			count--;
		}
		if (offsetOfFolder != 0) {
			if (searchFile(offsetOfFolder, pszFolderName, 0x10) == 0) {
				createDirectory(offsetOfFolder, pszFolderName);
				return TRUE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		offsetOfFolder = offset;
		if (searchFile(offsetOfFolder, pszFolderName, 0x10) == 0) {
			createDirectory(offsetOfFolder, pszFolderName);
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	return FALSE;
}

BOOL MyDeleteDirectory(char * pszFolderPath, char * pszFolderName) {
	init();
	long offset = (rscdSecCnt + FATSz16 * numFATs)*bytePerSec;
	int count = rootEntCnt;

	long offsetOfFolder = 0;
	stringstream folderStream;
	folderStream << pszFolderPath;
	if (folderStream.str() != string("C:")) {
		stringstream sstream;
		string tempDirName;
		string folderPath = folderStream.str();
		std::getline(folderStream, tempDirName, '\\');
		folderPath = string(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
		std::getline(folderStream, tempDirName, '\\');
		char dirBuffer[32];
		DirTable dirTable;
		while (count > 0) {
			SetHeaderOffset(offset, NULL, FILE_BEGIN);
			ReadFromDisk(dirBuffer, 32, NULL);
			dirTable = DirTable(dirBuffer);
			for (int i = 0; i < 11; i++) {
				sstream << *(dirTable.getDirName() + i);
			}
			string aString;
			sstream >> aString;
			if (dirTable.getDirAttr() == 0x10 && tempDirName == aString) {
				if (folderPath.begin() + tempDirName.size() != folderPath.end()) {
					string nextFolderPath(folderPath.begin() + tempDirName.size() + 1, folderPath.end());
					const char *c_nextFolderPath = nextFolderPath.c_str();
					offsetOfFolder = searchFolder(c_nextFolderPath, dirTable.getFstClus());
					break;
				}
				else {
					offsetOfFolder = offset;
					break;
				}
			}
			sstream.str("");
			offset += 32;
			count--;
		}
		if (offsetOfFolder != 0) {
			long dirOffset = searchFile(offsetOfFolder, pszFolderName, 0x10);
			if ( dirOffset != 0) {
				deleteDirectory(dirOffset);
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		offsetOfFolder = offset;
		long dirOffset = searchFile(offsetOfFolder, pszFolderName, 0x10);
		if (dirOffset != 0) {
			deleteDirectory(dirOffset);
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
}

BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	if (handles.searchHandle(dwFileHandle)) {
		char dirBuffer[32];
		SetHeaderOffset(dwFileHandle, NULL, FILE_BEGIN);
		ReadFromDisk(dirBuffer, 32, NULL);
		DirTable dirTable(dirBuffer);
		bitset<12> tempClus(dirTable.getFstClus());
		long fileSize = dirTable.getDirFileSize();
		long offset = handles.getOffset(dwFileHandle);
		switch (dwMoveMethod) {
		case 0://begin
			if (nOffset <= 0) {
				handles.alterHandle(dwFileHandle, 0);
			}
			else if (nOffset > fileSize) {
				handles.alterHandle(dwFileHandle, fileSize);
			}
			else {
				handles.alterHandle(dwFileHandle, nOffset);
			}
			break;
		case 1://current
			if (nOffset + offset < 0) {
				handles.alterHandle(dwFileHandle, 0);
			}
			else if (nOffset + offset > fileSize) {
				handles.alterHandle(dwFileHandle, fileSize);
			}
			else {
				handles.alterHandle(dwFileHandle, nOffset + offset);
			}
			break;
		case 2://end
			if (nOffset + fileSize < 0) {
				handles.alterHandle(dwFileHandle, 0);
			}
			else if (nOffset + fileSize > fileSize) {
				handles.alterHandle(dwFileHandle, fileSize);
			}
			else {
				handles.alterHandle(dwFileHandle, nOffset + fileSize);
			}
			break;
		default:
			break;
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}
