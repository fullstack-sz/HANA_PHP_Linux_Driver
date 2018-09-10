//---------------------------------------------------------------------------------------------------------------------------------
// File: xplat_intsafe.h
//
// Contents: This module defines helper functions to prevent
//			 integer overflow bugs.
//
//---------------------------------------------------------------------------------------------------------------------------------


#ifndef XPLAT_INTSAFE_H
#define XPLAT_INTSAFE_H

#if (_MSC_VER > 1000)
#pragma once
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && (_MSC_VER >= 1300)
#define _W64 __w64
#else
#define _W64
#endif
#endif

#include "sal_def.h"
#include <limits.h>

//
// typedefs
//
typedef char                CHAR;
typedef unsigned char       BYTE;
typedef unsigned short      USHORT;
typedef unsigned short      WORD;
typedef int                 INT;
typedef unsigned int        UINT;
typedef windowsLong_t       LONG;
typedef windowsULong_t      DWORD;
typedef windowsLongLong_t   LONGLONG;
typedef windowsULongLong_t  ULONGLONG;

typedef _W64 windowsLong_t LONG_PTR, *PLONG_PTR;
typedef _W64 windowsULong_t ULONG_PTR, *PULONG_PTR;

typedef LONG_PTR    SSIZE_T;
typedef ULONG_PTR   SIZE_T;

#define DWORD_MAX       0xffffffffUL

#endif // XPLAT_INTSAFE_H
