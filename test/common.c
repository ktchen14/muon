#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

Test test_roster[4096];
size_t test_length;

TEST(should_fail) {
  assert(0);
}

int main() {
  for (size_t i = 0; i < test_length; i++) {
    Test test = test_roster[i];
    fprintf(stderr, "Test: %s\n", test.name);
    test.test();
  }
}
