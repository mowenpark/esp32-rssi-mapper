#pragma once

// Runs Wi-Fi scans in the background so the screen stays live.

#include <vector>

#include "selection.h"

enum class ScanState { Idle, Running, Done, Failed };

void scannerBegin();
void scannerStart();
ScanState scannerPoll();
// Call once after scannerPoll() returns Done; frees the scan's memory.
std::vector<ScanResult> scannerTakeResults();
