#include "signal_level.h"

Rating ratingFor(int dbm) {
  if (dbm >= -50) return Rating::Excellent;
  if (dbm >= -60) return Rating::Good;
  if (dbm >= -70) return Rating::Fair;
  return Rating::Poor;
}

const char* ratingName(Rating r) {
  switch (r) {
    case Rating::Excellent: return "EXCELLENT";
    case Rating::Good: return "GOOD";
    case Rating::Fair: return "FAIR";
    case Rating::Poor: return "POOR";
  }
  return "POOR";
}

uint16_t ratingColor(Rating r) {
  switch (r) {
    case Rating::Excellent: return Colors::Excellent;
    case Rating::Good: return Colors::Good;
    case Rating::Fair: return Colors::Fair;
    case Rating::Poor: return Colors::Poor;
  }
  return Colors::Poor;
}

int barPercent(int dbm) {
  int pct = (dbm + 90) * 100 / 60;
  if (pct < 4) return 4;
  if (pct > 100) return 100;
  return pct;
}

std::string nodeCountText(int n) {
  return std::to_string(n) + (n == 1 ? " node heard" : " nodes heard");
}
