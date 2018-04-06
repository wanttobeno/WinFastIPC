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

#ifndef __IPC_TYPES_H
#define __IPC_TYPES_H

#include "ipc_thread.h"
#include "ipc_named_pipe.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef
BOOL
(WINAPI *IPC_ROUTINE)(
    LPCWSTR ChannelName,
    LPVOID MessageBuffer,
    DWORD MessageSize,
    LPVOID AnswerBuffer,
    DWORD AnswerSize
);

typedef struct _IPC_CHANNEL_DATA
{
    PTHREAD ServerThread;
    NAMED_PIPE ServerNamedPipe;
} IPC_CHANNEL_DATA, *PIPC_CHANNEL_DATA;

typedef struct _IPC_SERVER_THREAD_DATA
{
    WCHAR ChannelName[0x100];
    IPC_ROUTINE Routine;
    BOOL MultiSession;
} IPC_SERVER_THREAD_DATA, *PIPC_SERVER_THREAD_DATA;

typedef struct _IPC_CHANNEL_HEADER
{
    BOOL HasMessage;
    BOOL HasAnswer;
    DWORD MessageSize;
    DWORD AnswerSize;
} IPC_CHANNEL_HEADER, *PIPC_CHANNEL_HEADER;

#endif
