#ifndef MU_AREA_I
#define MU_AREA_I

#include "common.h"

#ifdef __has_feature
#if __has_feature(address_sanitizer)
#ifndef __SANITIZE_ADDRESS__
#define __SANITIZE_ADDRESS__
#endif
#endif
#endif

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif

#include <assert.h>
#include <stddef.h>

typedef struct area_t area_t;
struct area_t {
  area_t *next;

  // The size left in the area's data member
  size_t volume;

  char *sentinel;

  _Alignas(max_align_t) char data[];
};

static const size_t AREA_SIZE = 4096 - sizeof(void *[2]);
static const size_t AREA_UNIT = _Alignof(void *);

void *area_allocate_internal(area_t **area, size_t area_size);

/**
 * @brief Allocate an object (of the @a size and alignment @a m) in the @a area
 *
 * If this must create a new area to allocate the object, then <tt>*area</tt>
 * will be set to the new area.
 *
 * The @a size must be a nonzero multiple of the alignment requirement @a unit,
 * and @a unit must be a fundamental alignment, i.e. it must be a nonzero power
 * of two less than or equal to <tt>_Alignof(max_align_t)</tt>.
 *
 * On allocation failure, <tt>*area</tt> is unmodified and @c errno is set by
 * the allocator. This function can't fail otherwise. The behavior is undefined
 * if:
 *
 * - @a area or <tt>*area</tt> is @c NULL
 * - @a size is zero
 * - @a size isn't a multiple of @a unit
 * - @a unit is zero
 * - @a unit isn't a fundamental alignment
 *
 * @param area reference to the area to allocate the object in
 * @param size size of the object to allocate
 * @param unit alignment requirement of the object to allocate
 * @return the allocation on success; otherwise @c NULL
 */
__attribute__((nonnull))
static inline void *area_allocate(area_t **area, size_t size, size_t unit) {
  assert(*area != NULL);
  assert(size != 0);
  assert(size % unit == 0);
  assert(unit != 0);
  assert(unit <= _Alignof(max_align_t));
  assert(unit & (unit - 1) == 0);

  size_t volume = (*area)->volume;
  if (rare(unit > AREA_UNIT))
    volume &= ~(unit - 1);

  void *result = (*area)->sentinel - volume;
  if (common(!__builtin_sub_overflow(volume, size, &volume))) {
    (*area)->volume = volume;
#ifdef __SANITIZE_ADDRESS__
    __asan_unpoison_memory_region(result, size);
#endif
    return result;
  }

  size_t area_size;
  if (common(size < AREA_SIZE - offsetof(area_t, data)))
    area_size = AREA_SIZE;
  else if (rare((area_size = struct_size(area_t, data, size)) == 0))
    return NULL;

  return area_allocate_internal(area, area_size);
}

void *test(area_t **area) {
  return area_allocate(area, 48, 8);
}

#endif /* MU_AREA_I */
