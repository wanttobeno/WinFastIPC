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

#ifndef __IPC_NAMED_PIPE_H
#define __IPC_NAMED_PIPE_H

#include <Windows.h>
#include "../defines.h"
#include "../security/security_utils.h"

#define IPC_NAMED_PIPE_NAME_LENGTH 0x100

typedef struct _NAMED_PIPE
{
    WCHAR ObjectName[IPC_NAMED_PIPE_NAME_LENGTH];
    HANDLE ObjectHandle;
    HANDLE EventHandle;
} NAMED_PIPE, *PNAMED_PIPE;

typedef
BOOL
(*NAMED_PIPE_LEAVE_PROC)(
    LPVOID Context
);

BOOL
IpcCreateNamedPipe(
    IN LPCWSTR ObjectName,
    IN BOOL FirstInstance,
    IN DWORD OutBufferSize,
    IN DWORD InBufferSize,
    IN DWORD DefaultTimeout,
    OUT PNAMED_PIPE NamedPipe
);

BOOL
IpcDestroyNamedPipe(
    IN PNAMED_PIPE NamedPiped
);

BOOL
IpcOpenNamedPipe(
    IN LPCWSTR ObjectName,
    IN BOOL WaitIfNotAvailable,
    OUT PNAMED_PIPE NamedPipe
);

BOOL
IpcCloseNamedPipe(
    IN PNAMED_PIPE NamedPipe
);

BOOL
IpcWaitConnectClient(
    IN PNAMED_PIPE NamedPipe,
    IN DWORD Timeout,
    IN LPVOID Context,
    IN NAMED_PIPE_LEAVE_PROC LeaveProc
);

BOOL
IpcDisconnectClient(
    IN PNAMED_PIPE NamedPipe
);

BOOL
IpcReadFromNamedPipe(
    IN PNAMED_PIPE NamedPipe,
    IN LPVOID Buffer,
    IN DWORD BufferSize,
    IN BOOL IsServerSide,
    IN LPVOID Context,
    IN NAMED_PIPE_LEAVE_PROC LeaveProc
);

BOOL
IpcWriteToNamedPipe(
    IN PNAMED_PIPE NamedPipe,
    IN LPVOID Buffer,
    IN DWORD BufferSize,
    IN BOOL IsServerSide,
    IN LPVOID Context,
    IN NAMED_PIPE_LEAVE_PROC LeaveProc
);

BOOL
IpcFlushNamedPipe(
    IN PNAMED_PIPE NamedPipe
);

#endif
