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

#include "security_utils.h"

BOOL
ScCreateSecurityAttributes(PSECURITY_ATTRIBUTES SecurityAttributes,
                           PSECURITY_DESCRIPTOR SecurityDescriptor,
                           PSID *Sid)
{
    BOOL result;
    DWORD sidSize = 0;
    WCHAR domainName[0x100];
    DWORD domainNameLength = 0x100;
    WCHAR userName[0x400];
    DWORD userNameSize = sizeof(WCHAR) * 0x100;
    SID_NAME_USE sidNameUse;

    if (!Sid)
    {
        return FALSE;
    }

    result = GetUserNameW(userName, &userNameSize);

    if (!result)
    {
        return FALSE;
    }

    result = LookupAccountNameW(NULL,
                                userName,
                                NULL,
                                &sidSize,
                                domainName,
                                &domainNameLength,
                                &sidNameUse);

    if (result || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        return FALSE;
    }

    *Sid = (PSID)AllocMem(sidSize);

    if (!(*Sid))
    {
        return FALSE;
    }

    result = LookupAccountNameW(NULL,
                                userName,
                                *Sid,
                                &sidSize,
                                domainName,
                                &domainNameLength,
                                &sidNameUse);

    if (!result)
    {
        DeallocMem(*Sid);
        return FALSE;
    }

    result = InitializeSecurityDescriptor(SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION);

    if (!result)
    {
        DeallocMem(*Sid);
        return FALSE;
    }

    result = SetSecurityDescriptorOwner(SecurityDescriptor,
                                        *Sid,
                                        TRUE);

    if (!result)
    {
        DeallocMem(*Sid);
        return FALSE;
    }

    result = SetSecurityDescriptorDacl(SecurityDescriptor,
                                       TRUE,
                                       NULL,
                                       TRUE);

    if (!result)
    {
        DeallocMem(*Sid);
        return FALSE;
    }

    SecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes->lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes->bInheritHandle = FALSE;

    return TRUE;
}

BOOL
ScDestroySecurityAttributes(PSID Sid)
{
    DeallocMem(Sid);
    return TRUE;
}

