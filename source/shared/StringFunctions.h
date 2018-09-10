//---------------------------------------------------------------------------------------------------------------------------------
// File: StringFunctions.h
//
// Contents: Contains functions for handling UTF-16 on non-Windows platforms
//
//---------------------------------------------------------------------------------------------------------------------------------

#if !defined(_STRINGFUNCTIONS_H_)
#define _STRINGFUNCTIONS_H_

#include "xplat_winnls.h"

// ---------------------------------------------------------------------------
// Declare internal versions of string handling functions
// Only the functions implemented are declared here

// Copy
int         mplat_memcpy_s(void *_S1, size_t _N1, const void *_S2, size_t _N);
int         mplat_strcat_s( char *strDestination, size_t numberOfElements, const char *strSource );
int         mplat_strcpy_s(char * _Dst, size_t _SizeInBytes, const char * _Src);

size_t      strnlen_s(const char * _Str, size_t _MaxCount = INT_MAX);

// Copy
#define memcpy_s        mplat_memcpy_s
#define strcat_s        mplat_strcat_s
#define strcpy_s        mplat_strcpy_s

#endif // _STRINGFUNCTIONS_H_
