#include "display.h"

#include <TFT_eSPI.h>

namespace {

const int kPowerPin = 15;      // LCD power enable; must be HIGH on battery
const int kBacklightPin = 38;

const int kMargin = 12;
const int kWidth = 320 - 2 * kMargin;
const int kLabelY = 8;
const int kNumberY = 40;
const int kUnitY = 48;
const int kRatingY = 66;
const int kBarY = 112;
const int kBarHeight = 16;
const int kStatusY = 146;

const int kFontSmall = 2;   // 16 px
const int kFontMedium = 4;  // 26 px
const int kFontNumber = 7;  // 48 px seven-segment, digits and '-' only

enum class Layout { None, Meter, NotFound };

// What is on screen now, so each field is redrawn only when it changes.
struct Drawn {
  std::string label, number, rating, status, body, detail;
  uint16_t labelColor = 0, numberColor = 0, ratingColor = 0, statusColor = 0,
           bodyColor = 0, detailColor = 0;
  int barWidth = -1;
  uint16_t barColor = 0;
  bool unitDrawn = false;
};

TFT_eSPI tft;
Layout layout = Layout::None;
Drawn drawn;
int ratingX = 0;  // column right of the widest number

void useLayout(Layout next) {
  if (layout == next) return;
  tft.fillScreen(TFT_BLACK);
  layout = next;
  drawn = Drawn();
}

void drawText(std::string& cached, uint16_t& cachedColor,
              const std::string& text, uint16_t color, int x, int y, int font,
              int padWidth) {
  if (cached == text && cachedColor == color) return;
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextPadding(padWidth);
  tft.drawString(text.c_str(), x, y, font);
  cached = text;
  cachedColor = color;
}

void drawBar(int percent, uint16_t color) {
  int width = kWidth * percent / 100;
  if (drawn.barWidth == width && drawn.barColor == color) return;
  tft.fillRoundRect(kMargin, kBarY, kWidth, kBarHeight, 3, Colors::Track);
  if (width > 0) tft.fillRoundRect(kMargin, kBarY, width, kBarHeight, 3, color);
  drawn.barWidth = width;
  drawn.barColor = color;
}

void drawMeter(const Reading& r, uint16_t labelColor, uint16_t numberColor,
               uint16_t ratingColor, const std::string& status,
               uint16_t statusColor) {
  useLayout(Layout::Meter);
  drawText(drawn.label, drawn.labelColor, r.label, labelColor, kMargin,
           kLabelY, kFontMedium, kWidth);
  drawText(drawn.number, drawn.numberColor, std::to_string(r.dbm), numberColor,
           kMargin, kNumberY, kFontNumber, ratingX - kMargin);
  if (!drawn.unitDrawn) {
    tft.setTextColor(Colors::TextDim, TFT_BLACK);
    tft.setTextPadding(0);
    tft.drawString("dBm", ratingX, kUnitY, kFontSmall);
    drawn.unitDrawn = true;
  }
  drawText(drawn.rating, drawn.ratingColor, ratingName(r.rating), ratingColor,
           ratingX, kRatingY, kFontMedium, kMargin + kWidth - ratingX);
  drawBar(barPercent(r.dbm), ratingColor);
  drawText(drawn.status, drawn.statusColor, status, statusColor, kMargin,
           kStatusY, kFontSmall, kWidth);
}

}  // namespace

void displayBegin() {
  pinMode(kPowerPin, OUTPUT);
  digitalWrite(kPowerPin, HIGH);
  pinMode(kBacklightPin, OUTPUT);
  digitalWrite(kBacklightPin, HIGH);
  tft.init();
  tft.setRotation(1);
  tft.setTextDatum(TL_DATUM);
  tft.fillScreen(TFT_BLACK);
  ratingX = kMargin + tft.textWidth("-100", kFontNumber) + 12;
}

void showReading(const Reading& r) {
  uint16_t color = ratingColor(r.rating);
  drawMeter(r, TFT_WHITE, TFT_WHITE, color,
            nodeCountText(r.nodeCount) + " - scanning...", Colors::TextDim);
}

void showStale(const Reading& last, unsigned secondsAgo) {
  drawMeter(last, Colors::TextDim, Colors::Stale, Colors::Stale,
            "STALE - last scan failed " + std::to_string(secondsAgo) + "s ago",
            Colors::Amber);
}

void showNotFound(const char* ssid) {
  useLayout(Layout::NotFound);
  drawText(drawn.label, drawn.labelColor, "No network", Colors::TextDim,
           kMargin, kLabelY, kFontMedium, kWidth);
  drawText(drawn.body, drawn.bodyColor, "Not found", TFT_WHITE, kMargin, 50,
           kFontMedium, kWidth);
  drawText(drawn.detail, drawn.detailColor,
           std::string("Looking for \"") + ssid + "\"", Colors::TextDim,
           kMargin, 84, kFontSmall, kWidth);
  drawBar(0, Colors::Track);
  drawText(drawn.status, drawn.statusColor,
           nodeCountText(0) + " - scanning...", Colors::TextDim, kMargin,
           kStatusY, kFontSmall, kWidth);
}
