/*
 *  [ BSD License: http://opensource.org/licenses/bsd-license.php ]
 *  ===========================================================================
 *  Copyright (c) 2015, Lakutin Ivan
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ipc_named_pipe.h"

VOID
IpcMakePipeName(OUT LPWSTR DestinationName,
                IN SIZE_T DestinationNameSize,
                IN LPCWSTR SourceName)
{
    wcscpy_s(DestinationName,
             DestinationNameSize,
             L"\\\\.\\pipe\\");

    wcscat_s(DestinationName,
             DestinationNameSize,
             SourceName);
}

BOOL
IpcCreateNamedPipe(IN LPCWSTR ObjectName,
                   IN BOOL FirstInstance,
                   IN DWORD OutBufferSize,
                   IN DWORD InBufferSize,
                   IN DWORD DefaultTimeout,
                   OUT PNAMED_PIPE NamedPipe)
{
    DWORD OpenMode;
    DWORD PipeMode;
    PSID sid;
    SECURITY_DESCRIPTOR sd = { 0 };
    SECURITY_ATTRIBUTES sa = { 0 };

    IpcMakePipeName(NamedPipe->ObjectName,
                    0x100,
                    ObjectName);

    NamedPipe->EventHandle = CreateEventW(NULL,
                                          TRUE,
                                          TRUE,
                                          NULL);

    if (!NamedPipe->EventHandle)
    {
        return FALSE;
    }

    OpenMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
    OpenMode |= FirstInstance ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0;

    PipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;
    PipeMode |= PIPE_WAIT;
    PipeMode |= PIPE_REJECT_REMOTE_CLIENTS;

    if (!ScCreateSecurityAttributes(&sa, &sd, &sid))
    {
        NamedPipe->ObjectHandle = 0;
        CloseHandle(NamedPipe->EventHandle);
        NamedPipe->EventHandle = 0;
        return FALSE;
    }

    NamedPipe->ObjectHandle = CreateNamedPipeW(NamedPipe->ObjectName,
                                               OpenMode,
                                               PipeMode,
                                               PIPE_UNLIMITED_INSTANCES,
                                               OutBufferSize,
                                               InBufferSize,
                                               DefaultTimeout,
                                               &sa);

    ScDestroySecurityAttributes(sid);

    if (NamedPipe->ObjectHandle == INVALID_HANDLE_VALUE ||
        GetLastError() == ERROR_ACCESS_DENIED)
    {
        NamedPipe->ObjectHandle = 0;
        CloseHandle(NamedPipe->EventHandle);
        NamedPipe->EventHandle = 0;
        return FALSE;
    }

    return TRUE;
}

#include <stdio.h>

BOOL
IpcDestroyNamedPipe(IN PNAMED_PIPE NamedPipe)
{
    BOOL Result = FALSE;

    if (!NamedPipe->ObjectHandle || !NamedPipe->EventHandle)
    {
        return FALSE;
    }

    TRY(BOOL, TRUE)
    {
        Result = CloseHandle(NamedPipe->EventHandle);
        NamedPipe->EventHandle = 0;
        Result = CloseHandle(NamedPipe->ObjectHandle) && Result;
        NamedPipe->ObjectHandle = 0;
        LEAVE(Result);
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        return TRY_VALUE;
    }

    return Result;
}

UINT32
IpcGetCurrentTimeMilliseconds()
{
    UINT32 Milliseconds = 0;
    SYSTEMTIME Time;
    GetSystemTime(&Time);

    Milliseconds += Time.wMinute * 60 * 1000;
    Milliseconds += Time.wSecond * 1000;
    Milliseconds += Time.wMilliseconds;

    return Milliseconds;
}

BOOL
IpcOpenNamedPipe(IN LPCWSTR ObjectName,
                 IN BOOL WaitIfBusy,
                 OUT PNAMED_PIPE NamedPipe)
{
    DWORD LastError;

    IpcMakePipeName(NamedPipe->ObjectName,
                    0x100,
                    ObjectName);

    NamedPipe->EventHandle = CreateEventW(NULL,
                                          TRUE,
                                          TRUE,
                                          NULL);

    if (!NamedPipe->EventHandle)
    {
        return FALSE;
    }

    while (TRUE)
    {
        NamedPipe->ObjectHandle = CreateFileW(NamedPipe->ObjectName,
                                              GENERIC_READ | GENERIC_WRITE,
                                              0,
                                              NULL,
                                              OPEN_EXISTING,
                                              0,
                                              NULL);

        LastError = GetLastError();

        if (NamedPipe->ObjectHandle == INVALID_HANDLE_VALUE &&
            LastError == ERROR_FILE_NOT_FOUND)
        {
            NamedPipe->ObjectHandle = 0;
            CloseHandle(NamedPipe->EventHandle);
            NamedPipe->EventHandle = 0;
            return FALSE;
        }

        if (WaitIfBusy &&
            NamedPipe->ObjectHandle &&
            LastError == ERROR_PIPE_BUSY)
        {
            WaitNamedPipeW(NamedPipe->ObjectName,
                           NMPWAIT_WAIT_FOREVER);
            continue;
        }

        break;
    }

    if (!SetNamedPipeHandleState(NamedPipe->ObjectHandle,
                                 PIPE_READMODE_BYTE | PIPE_WAIT,
                                 NULL,
                                 NULL))
    {
        CloseHandle(NamedPipe->EventHandle);
        NamedPipe->EventHandle = 0;
        CloseHandle(NamedPipe->ObjectHandle);
        NamedPipe->ObjectHandle = 0;
        return FALSE;
    }

    return TRUE;
}

BOOL
IpcCloseNamedPipe(IN PNAMED_PIPE NamedPipe)
{
    BOOL Result = FALSE;

    if (!NamedPipe->ObjectHandle || !NamedPipe->EventHandle)
    {
        return FALSE;
    }

    TRY(BOOL, TRUE)
    {
        Result = CloseHandle(NamedPipe->EventHandle);
        NamedPipe->EventHandle = 0;
        Result = CloseHandle(NamedPipe->ObjectHandle) && Result;
        NamedPipe->ObjectHandle = 0;
        LEAVE(Result);
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        return TRY_VALUE;
    }

    return Result;
}

BOOL
IpcWaitConnectClient(IN PNAMED_PIPE NamedPipe,
                     IN DWORD Timeout,
                     IN LPVOID Context,
                     IN NAMED_PIPE_LEAVE_PROC LeaveProc)
{
    BOOL Result;
    volatile OVERLAPPED Overlapped = { 0 };
    DWORD UnknownValue;
    DWORD LastError;

    Overlapped.hEvent = NamedPipe->EventHandle;

    Result = ConnectNamedPipe(NamedPipe->ObjectHandle,
                              (LPOVERLAPPED)&Overlapped);

    if (Result)
    {
        return FALSE;
    }

    LastError = GetLastError();

    if (LastError == ERROR_PIPE_CONNECTED)
    {
        return TRUE;
    }

    if (LastError == ERROR_IO_PENDING)
    {
        while (TRUE)
        {
            Result = GetOverlappedResult(NamedPipe->ObjectHandle,
                                         (LPOVERLAPPED)&Overlapped,
                                         &UnknownValue,
                                         FALSE);

            if (!Result && GetLastError() != ERROR_IO_INCOMPLETE)
            {
                break;
            }

            if (Result && GetLastError() == ERROR_BROKEN_PIPE)
            {
                break;
            }

            if (HasOverlappedIoCompleted(&Overlapped))
            {
                break;
            }

            if (LeaveProc && LeaveProc(Context))
            {
                break;
            }
        }
    }

    Result = GetOverlappedResult(NamedPipe->ObjectHandle,
                                 (LPOVERLAPPED)&Overlapped,
                                 &UnknownValue,
                                 FALSE);

    return Result;
}

BOOL
IpcDisconnectClient(IN PNAMED_PIPE NamedPipe)
{
    return DisconnectNamedPipe(NamedPipe->ObjectHandle);
}

BOOL
IpcReadFromNamedPipe(IN PNAMED_PIPE NamedPipe,
                     IN LPVOID Buffer,
                     IN DWORD BufferSize,
                     IN BOOL IsServerSide,
                     IN LPVOID Context,
                     IN NAMED_PIPE_LEAVE_PROC LeaveProc)
{
    BOOL Result;
    DWORD Read;
    volatile OVERLAPPED Overlapped = { 0 };
    DWORD LastError;

    Overlapped.hEvent = NamedPipe->EventHandle;

    Result = ReadFile(NamedPipe->ObjectHandle,
                      Buffer,
                      BufferSize,
                      &Read,
                      IsServerSide ? (LPOVERLAPPED)&Overlapped : NULL);

    if (Result)
    {
        return Read == BufferSize;
    }
    else if (!IsServerSide)
    {
        return FALSE;
    }
    else
    {
        LastError = GetLastError();

        if (LastError == ERROR_BROKEN_PIPE)
        {
            return FALSE;
        }

        if (LastError == ERROR_HANDLE_EOF)
        {
            return FALSE;
        }

        if (LastError == ERROR_IO_PENDING)
        {
            while (TRUE)
            {
                if (HasOverlappedIoCompleted(&Overlapped))
                {
                    break;
                }
            }
        }

        Result = GetOverlappedResult(NamedPipe->ObjectHandle,
                                     (LPOVERLAPPED)&Overlapped,
                                     &Read,
                                     FALSE);
    }

    return Result && Read == BufferSize;
}

BOOL
IpcWriteToNamedPipe(IN PNAMED_PIPE NamedPipe,
                    IN LPVOID Buffer,
                    IN DWORD BufferSize,
                    IN BOOL IsServerSide,
                    IN LPVOID Context,
                    IN NAMED_PIPE_LEAVE_PROC LeaveProc)
{
    BOOL Result;
    DWORD Written;
    volatile OVERLAPPED Overlapped = { 0 };
    DWORD LastError;

    Overlapped.hEvent = NamedPipe->EventHandle;

    Result = WriteFile(NamedPipe->ObjectHandle,
                       Buffer,
                       BufferSize,
                       &Written,
                       IsServerSide ? (LPOVERLAPPED)&Overlapped : NULL);

    if (Result)
    {
        return Written == BufferSize;
    }
    else if (!IsServerSide)
    {
        return FALSE;
    }
    else
    {
        LastError = GetLastError();

        if (LastError == ERROR_BROKEN_PIPE)
        {
            return FALSE;
        }

        if (LastError == ERROR_HANDLE_EOF)
        {
            return FALSE;
        }

        if (LastError == ERROR_IO_PENDING)
        {
            while (TRUE)
            {
                if (HasOverlappedIoCompleted(&Overlapped))
                {
                    break;
                }
            }
        }

        Result = GetOverlappedResult(NamedPipe->ObjectHandle,
                                     (LPOVERLAPPED)&Overlapped,
                                     &Written,
                                     FALSE);
    }

    return Result && Written == BufferSize;
}

BOOL
IpcFlushNamedPipe(IN PNAMED_PIPE NamedPipe)
{
    return FlushFileBuffers(NamedPipe->ObjectHandle);
}
