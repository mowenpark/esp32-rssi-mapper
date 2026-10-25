#pragma once

// Runs one Wi-Fi scan at a time, to completion.
//
// Scans block (about 1.5 s) on purpose: arduino-esp32 2.0.14's async scan
// gives up after 20x the per-channel time while the radio is still
// scanning, and the aborted scan's "done" event is then credited to the
// next scan, so back-to-back async scans never finish.

#include <vector>

#include "scan_state.h"
#include "selection.h"

void scannerBegin();
// Fills results and returns true, or returns false if the scan failed.
bool scannerScan(std::vector<ScanResult>& results);
