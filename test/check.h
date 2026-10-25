#pragma once

#include <iostream>

extern int g_checks;
extern int g_failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    ++g_checks;                                                        \
    if (!(cond)) {                                                     \
      ++g_failures;                                                    \
      std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK(" #cond     \
                << ") failed\n";                                       \
    }                                                                  \
  } while (0)

#define CHECK_EQ(a, b)                                                 \
  do {                                                                 \
    ++g_checks;                                                        \
    auto check_a_ = (a);                                               \
    auto check_b_ = (b);                                               \
    if (!(check_a_ == check_b_)) {                                     \
      ++g_failures;                                                    \
      std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK_EQ(" #a     \
                << ", " #b ") failed: " << check_a_ << " != "          \
                << check_b_ << "\n";                                   \
    }                                                                  \
  } while (0)
