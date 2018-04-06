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

#include "ipc_server.h"

PIPC_CHANNEL_DATA
IpcServerAllocateChannelData()
{
    return (PIPC_CHANNEL_DATA)AllocMem(sizeof(IPC_CHANNEL_DATA));
}

BOOL
IpcServerDeallocateChannelData(PIPC_CHANNEL_DATA ChannelData)
{
    return DeallocMem(ChannelData);
}

BOOL
IpcServerTryCreateChannel(LPCWSTR ChannelName,
                          BOOL MultiSession)
{
    NAMED_PIPE Pipe;

    TRY(BOOL, TRUE)
    {
        if (!IpcCreateNamedPipe(ChannelName,
                                TRUE,
                                0x1000,
                                0x1000,
                                NMPWAIT_WAIT_FOREVER,
                                &Pipe))
        {
            LEAVE(FALSE);
        }
    }
    FINALLY
    {
        IpcDestroyNamedPipe(&Pipe);
    }

    return TRY_VALUE;
}

DWORD
IpcServerThread(PTHREAD Thread,
                LPVOID UserData)
{
    PIPC_SERVER_THREAD_DATA ThreadData;
    NAMED_PIPE Pipe;
    IPC_CHANNEL_HEADER Header;
    LPVOID InternalBuffer = NULL;
    BOOL Accepted;

    ThreadData = (PIPC_SERVER_THREAD_DATA)UserData;

    TRY(DWORD, 0)
    {
        if (!IpcCreateNamedPipe(ThreadData->ChannelName,
                                FALSE,
                                0x1000,
                                0x1000,
                                NMPWAIT_WAIT_FOREVER,
                                &Pipe))
        {
            LEAVE(1);
        }

        while (TRUE)
        {
            if (!IpcWaitConnectClient(&Pipe,
                                      INFINITE,
                                      Thread,
                                      (NAMED_PIPE_LEAVE_PROC)IpcIsSafeStopThread))
            {
                LEAVE(1);
            }

            if (!IpcReadFromNamedPipe(&Pipe,
                                      &Header,
                                      sizeof(IPC_CHANNEL_HEADER),
                                      TRUE,
                                      Thread,
                                      (NAMED_PIPE_LEAVE_PROC)IpcIsSafeStopThread))
            {
                LEAVE(1);
            }

            Accepted = TRUE;

            if (!IpcWriteToNamedPipe(&Pipe,
                                     &Accepted,
                                     sizeof(BOOL),
                                     TRUE,
                                     Thread,
                                     (NAMED_PIPE_LEAVE_PROC)IpcIsSafeStopThread))
            {
                LEAVE(1);
            }

            if (!(InternalBuffer = AllocMem(MAX(Header.MessageSize,
                                                Header.AnswerSize))))
            {
                LEAVE(1);
            }

            if (Header.HasMessage && !IpcReadFromNamedPipe(&Pipe,
                                                           InternalBuffer,
                                                           Header.MessageSize,
                                                           TRUE,
                                                           Thread,
                                                           (NAMED_PIPE_LEAVE_PROC)IpcIsSafeStopThread))
            {
                IpcFlushNamedPipe(&Pipe);
                IpcDisconnectClient(&Pipe);
                DeallocMem(InternalBuffer);
                InternalBuffer = NULL;
                continue;
            }

            ThreadData->Routine(ThreadData->ChannelName,
                                InternalBuffer,
                                Header.MessageSize,
                                InternalBuffer,
                                Header.AnswerSize);

            if (Header.HasAnswer)
            {
                IpcWriteToNamedPipe(&Pipe,
                                    InternalBuffer,
                                    Header.AnswerSize,
                                    TRUE,
                                    Thread,
                                    (NAMED_PIPE_LEAVE_PROC)IpcIsSafeStopThread);
            }

            IpcFlushNamedPipe(&Pipe);
            IpcDisconnectClient(&Pipe);
            DeallocMem(InternalBuffer);
            InternalBuffer = NULL;

        }
    }
    FINALLY
    {
        IpcDisconnectClient(&Pipe);
        IpcDestroyNamedPipe(&Pipe);
        DeallocMem(InternalBuffer);
    }

    return TRY_VALUE;
}

BOOL
IpcServerCreateServerThread(LPCWSTR ChannelName,
                            IPC_ROUTINE Routine,
                            BOOL MultiSession,
                            PTHREAD *Thread)
{
    PIPC_SERVER_THREAD_DATA UserData;

    UserData = (PIPC_SERVER_THREAD_DATA)
        AllocMem(sizeof(IPC_SERVER_THREAD_DATA));

    if (!UserData)
    {
        return FALSE;
    }

    wcscpy_s(UserData->ChannelName,
             ChannelName);

    UserData->Routine = Routine;
    UserData->MultiSession = MultiSession;

    if (!IpcCreateThread(IpcServerThread,
                         TRUE,
                         UserData,
                         Thread))
    {
        DeallocMem(UserData);
        return FALSE;
    }

    return TRUE;
}

BOOL
IpcDestroyServerThread(PTHREAD *Thread)
{
    BOOL Result;
    LPVOID UserData;

    UserData = IpcGetThreadUserData(*Thread);
    Result = DeallocMem(UserData);
    Result = IpcTerminateThread(*Thread, 0) && Result;
    Result = IpcDestroyThread(Thread) && Result;

    return Result;
}

