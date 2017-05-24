// FAT12.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


int main()
{
	char* file = ".\\fat.img";
	int num = MultiByteToWideChar(0, 0, file, -1, NULL, 0);
	wchar_t *wFileName = new wchar_t[num];
	MultiByteToWideChar(0, 0, file, -1, wFileName, num);
	StartupDisk(wFileName);
	delete wFileName;
	/*MyCreateDirectory("C:", "abc");
	MyCreateDirectory("C:\\abc", "aaa");
	MyCreateDirectory("C:\\abc\\aaa", "www");
	MyCreateDirectory("C:\\abc\\aaa", "delete");
	MyCreateFile("C:", "xiong.xin");
	MyCreateFile("C:\\abc", "zhang.xu");
	MyCreateFile("C:\\abc\\aaa", "shui.bi");*/
	DWORD fileHanldeWrite;
	fileHanldeWrite = MyOpenFile("C:", "xiong.xin");
	MySetFilePointer(fileHanldeWrite, 100, MY_FILE_BEGIN);
	MySetFilePointer(fileHanldeWrite, 200, MY_FILE_CURRENT);
	DWORD fileHanldeRead;
	fileHanldeRead = MyOpenFile("C:", "FLOWER.TXT");
	MySetFilePointer(fileHanldeRead, 0, MY_FILE_BEGIN);
	char buffer[600];
	MyReadFile(fileHanldeRead, buffer, 600);
	MyWriteFile(fileHanldeWrite, buffer, 600);
	MyCloseFile(fileHanldeRead);
	MyCloseFile(fileHanldeWrite);
	//MyDeleteDirectory("C:\\abc\\aaa", "delete");
	//MyDeleteDirectory("C:", "abc");
	//MyCreateDirectory("C:", "a");
	//MyDeleteFile("C:", "xiong.xin");
	//MyDeleteFile("C:\\abc", "zhang.xu");
	//MyDeleteFile("C:\\abc\\aaa", "shui.bi");
	ShutdownDisk();
    return 0;
}

