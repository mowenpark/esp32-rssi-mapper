#include "check.h"
#include "scan_state.h"

void test_scan_state() {
  CHECK(!scanSucceeded(-2));  // WIFI_SCAN_FAILED
  CHECK(!scanSucceeded(-1));  // WIFI_SCAN_RUNNING: not a finished scan
  // The 2.0.14 core reports a failed scan as done with 0 results; in a home
  // a real scan always hears something, so 0 is treated as a failure.
  CHECK(!scanSucceeded(0));
  CHECK(scanSucceeded(1));
  CHECK(scanSucceeded(26));
}
