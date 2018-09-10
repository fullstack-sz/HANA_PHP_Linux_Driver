//---------------------------------------------------------------------------------------------------------------------------------
// File: xplat_winnls.h
//
// Contents: Contains the minimal definitions to build on non-Windows platforms
//
//---------------------------------------------------------------------------------------------------------------------------------

#ifndef XPLAT_WINNLS_H
#define XPLAT_WINNLS_H

#include <stdlib.h>
#include "typedefs_for_linux.h"

struct threadlocaleinfostruct;
struct threadmbcinfostruct;
typedef struct threadlocaleinfostruct * pthreadlocinfo;
typedef struct threadmbcinfostruct * pthreadmbcinfo;

typedef struct localeinfo_struct
{
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
} _locale_tstruct, *_locale_t;

typedef DWORD LCTYPE;

#define MB_ERR_INVALID_CHARS      0x00000008  // error for invalid chars

typedef WORD LANGID;

#define MAKELANGID(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))
#define LANG_NEUTRAL                     0x00
#define SUBLANG_DEFAULT                             0x01    // user default

BOOL
WINAPI
IsDBCSLeadByte(
    __inn BYTE  TestChar);


#endif // XPLAT_WINNLS_H
