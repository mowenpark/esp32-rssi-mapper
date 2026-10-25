#pragma once

// Runs Wi-Fi scans in the background so the screen stays live.

#include <vector>

#include "scan_state.h"
#include "selection.h"

void scannerBegin();
void scannerStart();
ScanState scannerPoll();
// Call once after scannerPoll() returns Done; frees the scan's memory.
std::vector<ScanResult> scannerTakeResults();
