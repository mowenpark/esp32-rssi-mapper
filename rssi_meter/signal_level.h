#pragma once

// Turns a signal strength in dBm into what the screen shows: a rating,
// its colour and how full the bar is. No Arduino headers, so it is
// unit-tested on the host.

#include <cstdint>
#include <string>

enum class Rating { Excellent, Good, Fair, Poor };

// RGB565 values of the mockup colours.
namespace Colors {
constexpr uint16_t Excellent = 0x262B;  // #22C55E
constexpr uint16_t Good = 0xA726;       // #A3E635
constexpr uint16_t Fair = 0xFE62;       // #FACC15
constexpr uint16_t Poor = 0xFB8E;       // #F87171
constexpr uint16_t Track = 0x1946;      // #1F2937
constexpr uint16_t TextDim = 0x9D15;    // #9CA3AF
constexpr uint16_t Stale = 0x6B90;      // #6B7280
constexpr uint16_t Amber = 0xFDE4;      // #FBBF24
}  // namespace Colors

// >= -50 Excellent, >= -60 Good, >= -70 Fair, otherwise Poor.
Rating ratingFor(int dbm);

const char* ratingName(Rating r);
uint16_t ratingColor(Rating r);

// Bar fill in percent: -90 dBm is empty, -30 dBm is full, clamped to 4..100
// so even a very weak signal shows a sliver.
int barPercent(int dbm);

// "0 nodes heard", "1 node heard", "3 nodes heard".
std::string nodeCountText(int n);
