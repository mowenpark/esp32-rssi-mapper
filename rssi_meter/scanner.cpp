#include "scanner.h"

#include <WiFi.h>

namespace {

// 120 ms per channel keeps a full 2.4 GHz scan around 1.5-2 s.
const uint32_t kMsPerChannel = 120;

bool running = false;

}  // namespace

void scannerBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void scannerStart() {
  WiFi.scanNetworks(/*async=*/true, /*show_hidden=*/false, /*passive=*/false,
                    kMsPerChannel);
  running = true;
}

ScanState scannerPoll() {
  ScanState state = scanStateFor(running, WiFi.scanComplete());
  if (state == ScanState::Failed) {
    WiFi.scanDelete();
    running = false;
  }
  return state;
}

std::vector<ScanResult> scannerTakeResults() {
  std::vector<ScanResult> results;
  int16_t n = WiFi.scanComplete();
  for (int16_t i = 0; i < n; ++i) {
    results.push_back({WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(),
                       static_cast<int>(WiFi.RSSI(i))});
  }
  WiFi.scanDelete();
  running = false;
  return results;
}
