// Wi-Fi signal meter for the LilyGO T-Display-S3.
// Scans for your eero network and shows the strongest node's signal.

// The ESP32 SDK has its own config.h on the include path, so a missing
// config.h here can't be detected with __has_include alone; check for
// WIFI_SSID instead.
#if __has_include("config.h")
#include "config.h"
#endif
#ifndef WIFI_SSID
#error "Copy rssi_meter/config.example.h to rssi_meter/config.h and set WIFI_SSID"
#endif

#include "display.h"
#include "scanner.h"
#include "selection.h"
#include "signal_level.h"

// NODE_NAMES ends with a {nullptr, nullptr} sentinel.
static const size_t NODE_NAME_COUNT =
    sizeof(NODE_NAMES) / sizeof(NODE_NAMES[0]) - 1;

static Smoother smoother;
static Reading lastGood;
static bool haveLastGood = false;
static unsigned long lastGoodMs = 0;
static bool stale = false;
static unsigned long lastStaleDrawMs = 0;

static void onScanDone() {
  Pick pick = pickStrongest(scannerTakeResults(), WIFI_SSID);
  stale = false;
  if (!pick.found) {
    smoother.reset();
    haveLastGood = false;
    showNotFound(WIFI_SSID);
    return;
  }
  int dbm = smoother.add(pick.bssid, pick.rssi);
  lastGood = {nodeLabel(pick.bssid, NODE_NAMES, NODE_NAME_COUNT), dbm,
              ratingFor(dbm), pick.nodeCount};
  haveLastGood = true;
  lastGoodMs = millis();
  showReading(lastGood);
}

static void onScanFailed() {
  if (!haveLastGood) {
    showNotFound(WIFI_SSID);
    return;
  }
  if (!stale) {
    stale = true;
    lastStaleDrawMs = millis() - 1000;  // draw the stale screen right away
  }
}

void setup() {
  displayBegin();
  scannerBegin();
  scannerStart();
}

void loop() {
  switch (scannerPoll()) {
    case ScanState::Idle:
      scannerStart();
      break;
    case ScanState::Running:
      break;
    case ScanState::Done:
      onScanDone();
      scannerStart();
      break;
    case ScanState::Failed:
      onScanFailed();
      scannerStart();
      break;
  }

  if (stale && millis() - lastStaleDrawMs >= 1000) {
    lastStaleDrawMs = millis();
    showStale(lastGood, (millis() - lastGoodMs) / 1000);
  }
}
