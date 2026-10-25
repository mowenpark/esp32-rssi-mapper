#include <iostream>

#include "check.h"

int g_checks = 0;
int g_failures = 0;

void test_signal_level();

int main() {
  test_signal_level();
  std::cout << g_checks << " checks, " << g_failures << " failures\n";
  return g_failures == 0 ? 0 : 1;
}
