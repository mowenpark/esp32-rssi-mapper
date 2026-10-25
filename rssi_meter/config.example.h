#pragma once

// Copy this file to config.h and edit it. config.h is not committed.

#include "selection.h"

// Your eero network name, exactly as it appears (case-sensitive).
// No password is needed: the meter only scans, it never joins.
#define WIFI_SSID "YourNetwork"

// Optional nicknames for nodes. Walk to each node, read the "Node XX:YY"
// label on screen, and add a line. Keep the {nullptr, nullptr} line last.
static const NodeName NODE_NAMES[] = {
    // {"3F:A1", "Living Room"},
    {nullptr, nullptr},
};
