#pragma once

// Draws the meter's three screens on the T-Display-S3 (320x170, landscape).
// Only fields whose value changed are redrawn, so updates do not flicker.

#include <string>

#include "signal_level.h"

struct Reading {
  std::string label;
  int dbm;
  Rating rating;
  int nodeCount;
};

void displayBegin();
void showReading(const Reading& r);
void showNotFound(const char* ssid);
void showStale(const Reading& last, unsigned secondsAgo);
