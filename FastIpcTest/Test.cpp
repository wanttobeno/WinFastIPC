#include "../FastIpcLib/fast_ipc/ipc.h"
#include <tchar.h>
#include <stdio.h>

#ifdef _DEBUG
#pragma comment(lib,"../Debug/FastIpcLib.lib")
#else
#pragma comment(lib,"../Release/FastIpcLib.lib")
#endif // _DEBUG

#define IpcChannel  _T("MyTestIpcChannel")

CRITICAL_SECTION cs;
static UINT64 Counter = 0;

BOOL WINAPI TestIpcRoutine(
	LPCWSTR ChannelName,
	PVOID MessageBuf,
	DWORD MessageSize,
	PVOID AnswerBuf,
	DWORD AnswerSize
	)
{
	EnterCriticalSection(&cs);
	wprintf_s(L"[%llu] %s\n", ++Counter, MessageBuf);
	*((BOOL*)AnswerBuf) = rand() % 2 == 1;
	LeaveCriticalSection(&cs);
	return TRUE;
}

DWORD WINAPI LoopSend(void* pParm)
{
	LPCWSTR Message = L"Wow. There is random text...";
	DWORD MessageSize = (DWORD)((wcslen(Message) + 1) * sizeof(WCHAR));
	BOOL AnswerBool;

	while (TRUE)
	{
		if (!IpcSendIpcMessage(IpcChannel,
			(LPVOID)Message,
			MessageSize,
			&AnswerBool,
			sizeof(AnswerBool),
			INFINITE,
			FALSE))
		{
			printf("thread %d exit\n", GetCurrentThreadId());
			return 1;
		}
	}

	return 0;
}

int main(int agrc, char** agrv)
{
	InitializeCriticalSection(&cs);
	LPVOID ChannelData;
	bool bRet = IpcCreateIpcChannel(IpcChannel, &TestIpcRoutine, false, &ChannelData);
	if (bRet)
	{
		// 等待内部线程创建完毕
		Sleep(1000);

		const DWORD NumOfThreads = 8;
		HANDLE ThreadHandles[NumOfThreads];

		for (int i = 0; i < NumOfThreads; ++i)
		{
			ThreadHandles[i] = CreateThread(NULL, 0, LoopSend, NULL, 0, NULL);
		}

		getchar();


		for (int i = 0; i < NumOfThreads; ++i)
		{
			CloseHandle(ThreadHandles[i]);
		}

		// all threads will be stopped, SendIpcMessage() returns FALSE

		if (!IpcDestroyIpcChannel(ChannelData))
		{
			printf("DestroyIpcChannel() failed\n");
		}

		printf("all tests done\n");
		getchar();

	}
	else
	{
		printf("CreateIpcChannel() failed\n");
		getchar();
	}

	DeleteCriticalSection(&cs);
	return 0;
}
