#ifndef MU_AREA_I
#define MU_AREA_I

#include "common.h"
#include <stdint.h>

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
#include <stdlib.h>

typedef struct area_t area_t;
struct area_t {
  area_t *next;

  // The size left in the area's data member
  size_t volume;

  char *sentinel;

  _Alignas(max_align_t) char data[];
};

static const size_t AREA_SIZE = 4096 - sizeof(void *[2]);
static const size_t AREA_VOLUME = AREA_SIZE - offsetof(area_t, data);
static const size_t AREA_UNIT = _Alignof(void *);

/**
 * @brief Create an area
 *
 * The volume of the area will be @c AREA_VOLUME, and it will be suitably
 * aligned for any object type with fundamental alignment.
 *
 * On allocation failure, <tt>*area</tt> is unmodified and @c errno is set by
 * the allocator. This function can't fail otherwise.
 *
 * @param area the existing area to link the new area to, or @c NULL
 * @return the created area on success; otherwise @c NULL
 */
area_t *area_create(area_t *area)
  __attribute__((malloc));

/// @internal Create and allocate an object in an area
void *area_create_allocate(area_t **area, size_t size)
  __attribute__((malloc, nonnull));

/// @internal Create an area sized to hold a single object of size @a size
void *area_create_single(area_t **area, size_t size)
  __attribute__((malloc, nonnull));

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
__attribute__((malloc, nonnull))
static inline void *area_allocate(area_t **area, size_t size, size_t unit) {
  assert(*area != NULL);
  assert(size != 0);
  assert(size % unit == 0);
  assert(unit != 0);
  assert(unit <= _Alignof(max_align_t));
  assert(unit & (unit - 1) == 0);

  // Align volume down to unit
  size_t volume = (*area)->volume & ~(unit - 1);
  if (rare(unit > AREA_UNIT))
    volume &= ~(unit - 1);

  void *result = &(*area)->sentinel[-volume];
  if (common(!__builtin_sub_overflow(volume, size, &volume))) {
    (*area)->volume = volume;
#ifdef __SANITIZE_ADDRESS__
    __asan_unpoison_memory_region(result, size);
#endif
    return result;
  }

  if (rare(size > AREA_VOLUME))
    return area_create_single(area, size);

  return area_create_allocate(area, size);
}

/**
 * @brief Deallocate an object in the @a area
 *
 * @param object the object to deallocate
 */
__attribute__((nonnull))
static inline void area_deallocate(void *object) {
  // Find the area from the object by aligning it down to a 4096 byte boundary
  area_t *area = (area_t *) ((uintptr_t) object & ~(4096U - 1));
  assert(area->volume != 0);
  area->volume += (uintptr_t) object - (uintptr_t) area->data;
}

void *test(area_t **area) {
  return area_allocate(area, 48, 8);
}

#endif /* MU_AREA_I */
