#pragma once

// Decides what the meter shows from a list of scan results: which node of
// the home network is strongest, what to call it, and a smoothed reading.
// No Arduino headers, so it is unit-tested on the host.

#include <cstddef>
#include <string>
#include <vector>

struct ScanResult {
  std::string ssid;
  std::string bssid;  // "AA:BB:CC:DD:EE:FF"
  int rssi;
};

struct Pick {
  bool found;
  std::string bssid;
  int rssi;
  int nodeCount;  // results whose SSID matched
};

// Strongest result whose SSID matches exactly (case-sensitive). On equal
// RSSI the earlier result wins.
Pick pickStrongest(const std::vector<ScanResult>& results,
                   const std::string& ssid);

struct NodeName {
  const char* suffix;  // last two octets, e.g. "3F:A1"
  const char* name;
};

// Nickname whose suffix matches the end of bssid (case-insensitive), else
// "Node XX:YY" from the last two octets in uppercase.
std::string nodeLabel(const std::string& bssid, const NodeName* names,
                      size_t count);

// Rolling mean of the last 3 readings of one node. A reading from a
// different node starts the history over.
class Smoother {
 public:
  int add(const std::string& bssid, int rssi);
  void reset();

 private:
  std::string bssid_;
  int readings_[3] = {0, 0, 0};
  int count_ = 0;
  int next_ = 0;
};
