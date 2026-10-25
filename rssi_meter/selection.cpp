#include "selection.h"

#include <cctype>
#include <cmath>

namespace {

const size_t kSuffixLength = 5;  // "3F:A1"

std::string upperSuffix(const std::string& bssid) {
  std::string tail = bssid.size() > kSuffixLength
                         ? bssid.substr(bssid.size() - kSuffixLength)
                         : bssid;
  for (char& c : tail) c = static_cast<char>(std::toupper(c));
  return tail;
}

}  // namespace

Pick pickStrongest(const std::vector<ScanResult>& results,
                   const std::string& ssid) {
  Pick pick{false, "", 0, 0};
  for (const ScanResult& r : results) {
    if (r.ssid != ssid) continue;
    ++pick.nodeCount;
    if (!pick.found || r.rssi > pick.rssi) {
      pick.found = true;
      pick.bssid = r.bssid;
      pick.rssi = r.rssi;
    }
  }
  return pick;
}

std::string nodeLabel(const std::string& bssid, const NodeName* names,
                      size_t count) {
  std::string suffix = upperSuffix(bssid);
  for (size_t i = 0; i < count; ++i) {
    if (names[i].suffix && upperSuffix(names[i].suffix) == suffix) {
      return names[i].name;
    }
  }
  return "Node " + suffix;
}

int Smoother::add(const std::string& bssid, int rssi) {
  if (bssid != bssid_) {
    reset();
    bssid_ = bssid;
  }
  readings_[next_] = rssi;
  next_ = (next_ + 1) % 3;
  if (count_ < 3) ++count_;
  int sum = 0;
  for (int i = 0; i < count_; ++i) sum += readings_[i];
  return static_cast<int>(std::lround(static_cast<double>(sum) / count_));
}

void Smoother::reset() {
  bssid_.clear();
  count_ = 0;
  next_ = 0;
}
