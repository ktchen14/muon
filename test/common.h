#ifndef MUON_TEST_COMMON_H
#define MUON_TEST_COMMON_H

#include <stddef.h>

typedef struct {
  const char *name;
  void (*test)();
} Test;

extern Test test_roster[4096];
extern size_t test_length;

/// Used to define a test
#define TEST(name) \
  static void test_##name(); \
  [[gnu::constructor]] void register_##name() { \
    test_roster[test_length++] = (Test) {#name, test_##name}; \
  } \
  static void test_##name() \

#endif /* MUON_TEST_COMMON_H */
