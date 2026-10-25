#include "check.h"
#include "scan_state.h"

void test_scan_state() {
  CHECK(scanStateFor(false, 5) == ScanState::Idle);
  CHECK(scanStateFor(true, -1) == ScanState::Running);  // WIFI_SCAN_RUNNING
  CHECK(scanStateFor(true, -2) == ScanState::Failed);   // WIFI_SCAN_FAILED
  // The 2.0.14 core reports a failed scan as done with 0 results; in a home
  // a real scan always hears something, so 0 is treated as a failure.
  CHECK(scanStateFor(true, 0) == ScanState::Failed);
  CHECK(scanStateFor(true, 1) == ScanState::Done);
  CHECK(scanStateFor(true, 12) == ScanState::Done);
}
