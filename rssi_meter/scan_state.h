#pragma once

// Interprets WiFi.scanNetworks()'s return value. No Arduino headers, so it
// is unit-tested on the host.

// count: result count, or -1 running / -2 failed.
// arduino-esp32 2.0.14 reports a failed scan as done with 0 results, and a
// scan in a home always hears some network, so 0 counts as a failure.
bool scanSucceeded(int count);
