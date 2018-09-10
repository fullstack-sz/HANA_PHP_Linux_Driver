//---------------------------------------------------------------------------------------------------------------------------------
// File: InterlockedAtomic_gcc.h
//
// Contents: Contains a portable abstraction for interlocked, atomic
//			 operations on int32_t and pointer types.
//
//---------------------------------------------------------------------------------------------------------------------------------

#ifndef __INTERLOCKEDATOMIC_GCC_H__
#define __INTERLOCKEDATOMIC_GCC_H__

#if !defined(__GNUC__)
#error "Incorrect compiler configuration in InterlockedAtomic.h.  Was expecting GCC."
#endif

inline LONG InterlockedCompareExchange( LONG volatile * atomic, LONG newValue, LONG compareTo )
{
    return __sync_val_compare_and_swap( atomic, compareTo, newValue );
}

#endif // __INTERLOCKEDATOMIC_GCC_H__
