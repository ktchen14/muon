#ifndef MU_COMMON_H
#define MU_COMMON_H

#include <stdio.h>
#include <stdint.h>

/// Stream to emit debugging output to (defaults to @c stderr)
extern _Thread_local FILE *muon_debug_stream;

/// Whether to colorize the debug output
extern _Thread_local _Bool mu_debug_colorize;

typedef uint64_t MuonHash;

#ifdef __has_attribute
  #define MUON_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
  #define MUON_HAS_ATTRIBUTE(x) 0
#endif

#if MUON_HAS_ATTRIBUTE(const)
  #define MUON_CONST __attribute__((const))
#else
  #define MUON_CONST
#endif

#if MUON_HAS_ATTRIBUTE(counted_by)
  #define MUON_COUNTED_BY(...) __attribute__((counted_by(__VA_ARGS__)))
#else
  #define MUON_COUNTED_BY(...)
#endif

#if MUON_HAS_ATTRIBUTE(format)
  #define MUON_FORMAT(...) __attribute__((format(__VA_ARGS__)))
#else
  #define MUON_FORMAT(...)
#endif

#if MUON_HAS_ATTRIBUTE(malloc)
  #define MUON_MALLOC __attribute__((malloc))
#else
  #define MUON_MALLOC
#endif

#if MUON_HAS_ATTRIBUTE(nonnull)
  #define MUON_NONNULL __attribute__((nonnull))
  #define MUON_NONNULL_ARGS(...) __attribute__((nonnull(__VA_ARGS__)))
#else
  #define MUON_NONNULL
  #define MUON_NONNULL_ARGS(...)
#endif

#if MUON_HAS_ATTRIBUTE(pure)
  #define MUON_PURE __attribute__((pure))
#else
  #define MUON_PURE
#endif

#if MUON_HAS_ATTRIBUTE(returns_nonnull)
  #define MUON_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
  #define MUON_RETURNS_NONNULL
#endif

#undef MUON_HAS_ATTRIBUTE

#endif /* MU_COMMON_H */
