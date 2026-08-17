/* wint_t via the __need_wint_t partial-include protocol (glibc/newlib
 * convention): <cwctype>/<wctype.h> #define __need_wint_t then #include
 * <stddef.h> expecting ONLY this typedef, bypassing the __STDDEF_H__ guard
 * below. This shim shadows the real <stddef.h> on the include path (see the
 * ptrdiff_t note below for the same class of issue), so without this, any
 * iostream-family header pulled in by a port TU breaks on VitaSDK's g++
 * 15.2.0 with 'wint_t' was not declared. Mirrors the real stddef.h's own
 * guard exactly. */
#if defined(__need_wint_t)
#ifndef _WINT_T
#define _WINT_T
#ifndef __WINT_TYPE__
#define __WINT_TYPE__ unsigned int
#endif
typedef __WINT_TYPE__ wint_t;
#endif
#undef __need_wint_t
#endif

#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <PR/ultratypes.h>

#ifdef _MSC_VER
/* MSVC: offsetof is a compiler intrinsic */
#ifndef offsetof
#define offsetof(type, member) ((size_t)&(((type*)0)->member))
#endif
#elif !defined(__sgi)
/* GCC/Clang: use built-in offsetof macro */
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
/* IDO: use macro from Indy headers */
#define offsetof(s, m) (size_t)(&(((s*)0)->m))
#endif

/* ptrdiff_t. Decomp source never uses it, but this shim shadows the real
 * <stddef.h> on the include path, so any system header pulled in by a port
 * TU that transitively references ptrdiff_t breaks if we don't supply it.
 * NDK r29 / bionic <unistd.h> surfaces this; macOS/glibc happen to declare
 * it elsewhere on the include chain, which masked the issue on desktop.
 * Provided via the compiler's built-in __PTRDIFF_TYPE__ macro (GCC/Clang). */
#if (defined(__GNUC__) || defined(__clang__)) && !defined(_PTRDIFF_T_DEFINED_)
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#define _PTRDIFF_T_DEFINED_
#endif

/* max_align_t. <cstddef> (pulled in by libultraship's use of <vector>/
 * <memory_resource> etc.) does `using ::max_align_t;` unconditionally in
 * C++11+, expecting a normal (non-__need_*) <stddef.h> include to have
 * provided it - same shadowing issue as wint_t/ptrdiff_t above. Struct
 * layout copied from the real stddef.h; only used for alignment, not
 * cross-TU layout, so this doesn't need to match byte-for-byte, just be
 * at least as aligned as any standard type. */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) \
	|| (defined(__cplusplus) && __cplusplus >= 201103L)
#ifndef _GCC_MAX_ALIGN_T
#define _GCC_MAX_ALIGN_T
typedef struct {
	long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
	long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#endif
#endif

#endif /* __STDDEF_H__ */
