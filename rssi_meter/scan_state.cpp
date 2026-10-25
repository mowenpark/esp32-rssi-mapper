#include "scan_state.h"

ScanState scanStateFor(bool running, int count) {
  if (!running) return ScanState::Idle;
  if (count == -1) return ScanState::Running;
  if (count <= 0) return ScanState::Failed;
  return ScanState::Done;
}
