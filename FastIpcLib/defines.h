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

#ifndef __DEFINES_H
#define __DEFINES_H

#include <Windows.h>

// POINTER
#define PTR_ADD_OFFSET(base, offset) ((LPVOID)((ULONG_PTR)base + (offset)))
#define PTR_SUB_OFFSET(base, offset) ((LPVOID)((ULONG_PTR)base - (offset)))

// TRY FINALLY EXCEPT
#define TRY(return_type, return_value) return_type __try_var_value = (return_value); __try
#define FINALLY __finally
#define EXCEPT __except
#define LEAVE(return_value) __try_var_value = (return_value); __leave;
#define TRY_VALUE (__try_var_value)

// IF
#define DO_IF(expr, do_expr) if (expr) { do_expr; }
#define DO_ELSE(expr, do_expr) DO_IF(!(expr), do_expr)

// LOOP
#define CONTINUE continue;
#define CONTINUE_IF(expr) DO_IF(expr, CONTINUE)

#define BREAK break
#define BREAK_IF(expr) DO_IF(expr, BREAK)

#define RETURN return
#define RETURN_IF(expr) DO_IF((expr), RETURN)
#define RETURN_VALUE_IF(expr, value) DO_IF((expr), RETURN(value))

#endif
