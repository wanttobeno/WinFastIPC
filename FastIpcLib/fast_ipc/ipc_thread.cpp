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

#include "ipc_thread.h"

DWORD
WINAPI
IpcSystemThreadRoutine(IN LPVOID Parameter)
{
    PTHREAD CurrentThread;

    if (!Parameter)
    {
        return 1;
    }

    CurrentThread = (PTHREAD)Parameter;

    return CurrentThread->ThreadRoutine(CurrentThread,
                                        CurrentThread->UserData);
}

BOOL
IpcCreateThread(IN IPC_THREAD_ROUTINE ThreadRoutine,
                IN BOOL CreateSuspended,
                IN LPVOID UserData,
                OUT PTHREAD *Thread)
{
    PTHREAD NewThread = NULL;

    TRY(BOOL, TRUE)
    {
        if (!(NewThread = (PTHREAD)AllocMem(sizeof(THREAD))))
        {
            LEAVE(FALSE);
        }

        InitializeCriticalSection(&NewThread->CriticalSection);

        NewThread->DoSafeStop = FALSE;
        NewThread->IsSuspended = CreateSuspended;
        NewThread->ThreadRoutine = ThreadRoutine;
        NewThread->UserData = UserData;
        NewThread->ObjectHandle = CreateThread(NULL,
                                               0,
                                               IpcSystemThreadRoutine,
                                               NewThread,
                                               CREATE_SUSPENDED,
                                               &NewThread->ObjectId);

        if (!NewThread->ObjectHandle)
        {
            LEAVE(FALSE);
        }

        if (!CreateSuspended)
        {
            ResumeThread(NewThread->ObjectHandle);
        }
    }
    FINALLY
    {
        if (!TRY_VALUE && NewThread)
        {
            DeleteCriticalSection(&NewThread->CriticalSection);
            TerminateThread(NewThread->ObjectHandle, 0);
            CloseHandle(NewThread->ObjectHandle);
            DeallocMem(NewThread);
            NewThread = NULL;
        }
    }

    *Thread = NewThread;

    return TRY_VALUE;
}

BOOL
IpcDestroyThread(IN PTHREAD *Thread)
{
    BOOL Result;

    DeleteCriticalSection(&(*Thread)->CriticalSection);
    Result = CloseHandle((*Thread)->ObjectHandle);
    Result = DeallocMem(*Thread) && Result;

    *Thread = NULL;

    return Result;
}

VOID
IpcSafeStopThread(IN PTHREAD Thread,
                  IN BOOL WaitThread,
                  IN DWORD Timeout)
{
    EnterCriticalSection(&Thread->CriticalSection);
    Thread->DoSafeStop = TRUE;
    LeaveCriticalSection(&Thread->CriticalSection);

    if (WaitThread)
    {
        WaitForSingleObject(Thread->ObjectHandle, Timeout);
    }
}

BOOL
IpcIsSafeStopThread(IN PTHREAD Thread)
{
    BOOL Result;
    EnterCriticalSection(&Thread->CriticalSection);
    Result = Thread->DoSafeStop;
    LeaveCriticalSection(&Thread->CriticalSection);
    return Result;
}

BOOL
IpcTerminateThread(IN PTHREAD Thread,
                   IN DWORD ExitCode)
{
    return TerminateThread(Thread->ObjectHandle, ExitCode);
}

BOOL
IpcSuspendThread(IN PTHREAD Thread)
{
    if (Thread->IsSuspended)
    {
        return FALSE;
    }

    SuspendThread(Thread->ObjectHandle);
    Thread->IsSuspended = TRUE;

    return TRUE;
}

BOOL
IpcResumeThread(IN PTHREAD Thread)
{
    if (!Thread->IsSuspended)
    {
        return FALSE;
    }

    ResumeThread(Thread->ObjectHandle);
    Thread->IsSuspended = FALSE;

    return TRUE;
}

LPVOID
IpcGetThreadUserData(IN PTHREAD Thread)
{
    return Thread->UserData;
}
