#include "scanner.h"

#include <WiFi.h>

namespace {

// 120 ms per channel keeps a full 2.4 GHz scan around 1.5-2 s.
const uint32_t kMsPerChannel = 120;

}  // namespace

void scannerBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

bool scannerScan(std::vector<ScanResult>& results) {
  results.clear();
  int16_t n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/false,
                                /*passive=*/false, kMsPerChannel);
  bool ok = scanSucceeded(n);
  for (int16_t i = 0; ok && i < n; ++i) {
    results.push_back({WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(),
                       static_cast<int>(WiFi.RSSI(i))});
  }
  WiFi.scanDelete();
  return ok;
}
