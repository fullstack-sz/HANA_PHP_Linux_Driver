/*********************************************************************
\if EMIT_LICENCE
    ========== licence begin  Microsoft
** SQLTYPES.H - This file defines the types used in ODBC
**
** (C) Copyright 1995-1999 By Microsoft Corp.
**
**		Created 04/10/95 for 2.50 specification
**		Updated 12/11/95 for 3.00 specification
    ========== licence end
\endif
*********************************************************************/

/* Adopted for SAP DB */

#ifndef __SQLTYPES2_SAPDB
#define __SQLTYPES2_SAPDB

#ifdef __cplusplus
extern "C" { 			/* Assume C declarations for C++   */
#endif  /* __cplusplus */

typedef char            TCHAR;
#define BOOL            int 
typedef TCHAR*          LPTSTR;
/* environment specific definitions */
#ifdef __cplusplus
}                                    /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* #ifndef __SQLTYPES */

