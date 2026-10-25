#pragma once

// Interprets WiFi.scanComplete()'s return value. No Arduino headers, so it
// is unit-tested on the host.

enum class ScanState { Idle, Running, Done, Failed };

// running: a scan was started and not yet collected.
// count: WiFi.scanComplete(); -1 running, -2 failed, otherwise result count.
// arduino-esp32 2.0.14 reports a failed scan as done with 0 results, and a
// scan in a home always hears some network, so 0 counts as a failure.
ScanState scanStateFor(bool running, int count);
