//---------------------------------------------------------------------------------------------------------------------------------
// File: InterlockedAtomic.h
//
// Contents: Contains a portable abstraction for interlocked, atomic
// 			 operations on int32_t and pointer types.
//
//---------------------------------------------------------------------------------------------------------------------------------

#ifndef __INTERLOCKEDATOMIC_H__
#define __INTERLOCKEDATOMIC_H__

// Forward references and contract specifications
//

// Always returns old value
// Sets to new value if old value equals compareTo
LONG InterlockedCompareExchange( LONG volatile * atomic, LONG newValue, LONG compareTo );


// Use conditional compilation to load the implementation
//
#if defined(_MSC_VER)
#include "InterlockedAtomic_WwoWH.h"
#elif defined(__GNUC__)
#include "interlockedatomic_gcc.h"
#else
#error "Unsupported compiler"
#endif

#endif // __INTERLOCKEDATOMIC_H__
