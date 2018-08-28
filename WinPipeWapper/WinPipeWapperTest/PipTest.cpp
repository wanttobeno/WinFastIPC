#include <Windows.h>
#include <thread>
#include "Pipe.h"

bool g_bExit = false;

void th_pipeWrite()
{
	Pipe pipe;
	pipe.CreatePipe("test_pipe");
	int nCount = 1;
	char szNum[10] = { 0 };
	do 
	{
		itoa(nCount, szNum, 10);
		nCount++;
		std::string strMsg = "WinPip中文123_测试";
		strMsg.append(szNum);
		pipe.PipeSendMessage(strMsg);
		// 写的时间间要大于读的时间间隔
		Sleep(500);
	} while (!g_bExit);
}

void th_pipeRead()
{
	Pipe pipe;
	pipe.ConnectToPipe("\\\\.\\pipe\\test_pipe");
	do
	{
		std::string  str;
		pipe.ReadMessage(str);
		if (str.length()>0)
		{
			printf("Get: %s \n", str.c_str());
		}
		// 写的时间间要大于读的时间间隔
		Sleep(150);
	} while (!g_bExit);
}

int main(int agrc,char* agrv[])
{
	std::thread t1(th_pipeWrite);
	t1.detach();
	Sleep(100);
	std::thread t2(th_pipeRead);;
	t2.join();
	return 0;
}