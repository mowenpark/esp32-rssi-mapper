# Wi-Fi Signal Meter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Arduino sketch for the LilyGO T-Display-S3 that scans for the user's eero network and shows the strongest node's signal as a large dBm number, rating word and colour bar.

**Architecture:** Two pure C++ units (`signal_level`, `selection`) hold all decision logic and are unit-tested on the Mac with plain `c++`. Two hardware units (`scanner`, `display`) wrap the ESP32 Wi-Fi scan and TFT_eSPI. The `.ino` wires them in a non-blocking loop. Correctness of firmware is checked by compiling with `arduino-cli` (same core and library the user's Arduino IDE uses) and by the user's on-device checklist.

**Tech Stack:** C++17, Arduino IDE 2.x, Espressif `esp32` board package **2.0.14**, TFT_eSPI **2.5.43** with `Setup206_LilyGo_T_Display_S3.h`, `arduino-cli` (verification only), `make` + Apple clang for host tests.

**Spec:** `docs/superpowers/specs/2026-10-24-wifi-signal-meter-design.md`

## Global Constraints

- Board package `esp32` by Espressif pinned to **2.0.14** — LilyGO tested this version, and TFT_eSPI crashes in `tft.init()` on 3.x cores (TFT_eSPI issue #3329). Never upgrade it in this project.
- TFT_eSPI pinned to **2.5.43**; its `User_Setup_Select.h` must include `User_Setups/Setup206_LilyGo_T_Display_S3.h` instead of `User_Setup.h`.
- Board settings: ESP32S3 Dev Module, USB CDC On Boot Enabled, Flash Size 16MB, Partition Scheme "16M Flash (3MB APP/9.9MB FATFS)", PSRAM OPI PSRAM, USB Mode Hardware CDC and JTAG, Upload Mode UART0/Hardware CDC.
- `signal_level.*` and `selection.*` include no Arduino headers (only `<cstdint>`, `<string>`, `<vector>`, `<cmath>`, `<cstddef>`), so they compile both on-device and on the Mac.
- All on-screen text is plain ASCII: TFT_eSPI's built-in fonts have no `…` or `·` glyphs. Use `...` and `-` where the mockups show `…` and `·`.
- Screen: landscape (rotation 1), 320×170, black background.
- No Wi-Fi password anywhere; the device never calls `WiFi.begin`.
- `rssi_meter/config.h` is git-ignored; only `config.example.h` is committed.

## Review Focus

1. **`config.h` missing on first build** — a fresh clone has no `config.h`; the user should get a clear `#error` naming the copy step, not a wall of undefined-symbol errors. (Task 3, compile check.)
2. **BSSID case and format from the ESP32** — `WiFi.BSSIDstr()` returns uppercase `AA:BB:CC:DD:EE:FF`, while users may type nicknames in lowercase; matching must be case-insensitive and labels uppercase. (Task 2 tests.)
3. **Exactly one node heard** — the status line must read "1 node heard", not "1 nodes heard". (Task 1 test of `nodeCountText`.)
4. **Stronger-than-expected signal next to a node** (e.g. −25 dBm) and very weak (−95) — the bar must clamp to 100 % and 4 % and the rating stay EXCELLENT / POOR. (Task 1 tests.)
5. **Strongest node flips back and forth between two nodes of similar strength** — the smoother resets on every switch, so the number shown must be that node's raw reading, never an average across two nodes. (Task 2 test.)

---

### Task 1: Project scaffold, host test harness, signal_level

**Files:**
- Create: `.gitignore`, `Makefile`, `test/check.h`, `test/test_signal_level.cpp`, `test/test_main.cpp`
- Create: `rssi_meter/signal_level.h`, `rssi_meter/signal_level.cpp`

**Interfaces:**
- Produces:
  ```cpp
  enum class Rating { Excellent, Good, Fair, Poor };
  Rating ratingFor(int dbm);              // ≥-50 Excellent, ≥-60 Good, ≥-70 Fair, else Poor
  const char* ratingName(Rating r);        // "EXCELLENT" "GOOD" "FAIR" "POOR"
  uint16_t ratingColor(Rating r);          // RGB565
  int barPercent(int dbm);                 // (dbm+90)*100/60, int truncation, clamped 4..100
  std::string nodeCountText(int n);        // "0 nodes heard", "1 node heard", "3 nodes heard"
  namespace Colors {                       // RGB565 of the mockup hexes
    constexpr uint16_t Excellent = 0x262B; // #22C55E
    constexpr uint16_t Good      = 0xA726; // #A3E635
    constexpr uint16_t Fair      = 0xFE62; // #FACC15
    constexpr uint16_t Poor      = 0xFB8E; // #F87171
    constexpr uint16_t Track     = 0x1946; // #1F2937
    constexpr uint16_t TextDim   = 0x9D15; // #9CA3AF
    constexpr uint16_t Stale     = 0x6B90; // #6B7280
    constexpr uint16_t Amber     = 0xFDE4; // #FBBF24
  }
  ```
- Test harness: `test/check.h` defines `CHECK(cond)` and `CHECK_EQ(a, b)` that print file:line and the values on failure and bump a global failure counter; `test/test_main.cpp` calls each `test_*()` function, prints `N checks, M failures`, and returns non-zero on any failure. No third-party framework.

- [ ] **Step 1: Scaffold.** `.gitignore` contains `rssi_meter/config.h` and `build/`. `Makefile` target `test` builds `build/tests` from `test/*.cpp rssi_meter/signal_level.cpp rssi_meter/selection.cpp` with `c++ -std=c++17 -Wall -Wextra -Irssi_meter` and runs it. Until Task 2, list only `signal_level.cpp` in the sources (Task 2 adds `selection.cpp`).

- [ ] **Step 2: Write failing tests** in `test/test_signal_level.cpp`:

```cpp
void test_signal_level() {
  CHECK(ratingFor(-30) == Rating::Excellent);
  CHECK(ratingFor(-50) == Rating::Excellent);
  CHECK(ratingFor(-51) == Rating::Good);
  CHECK(ratingFor(-60) == Rating::Good);
  CHECK(ratingFor(-61) == Rating::Fair);
  CHECK(ratingFor(-70) == Rating::Fair);
  CHECK(ratingFor(-71) == Rating::Poor);
  CHECK(ratingFor(-95) == Rating::Poor);
  CHECK_EQ(std::string(ratingName(Rating::Good)), std::string("GOOD"));
  CHECK_EQ(ratingColor(Rating::Fair), Colors::Fair);
  CHECK_EQ(barPercent(-30), 100);
  CHECK_EQ(barPercent(-25), 100);   // clamp high
  CHECK_EQ(barPercent(-52), 63);
  CHECK_EQ(barPercent(-78), 20);
  CHECK_EQ(barPercent(-90), 4);     // clamp low
  CHECK_EQ(barPercent(-95), 4);
  CHECK_EQ(nodeCountText(0), std::string("0 nodes heard"));
  CHECK_EQ(nodeCountText(1), std::string("1 node heard"));
  CHECK_EQ(nodeCountText(3), std::string("3 nodes heard"));
}
```

- [ ] **Step 3: Run** `make test`. Expected: compile failure (`signal_level.h` not found).
- [ ] **Step 4: Implement** `rssi_meter/signal_level.h/.cpp` per the Interfaces block.
- [ ] **Step 5: Run** `make test`. Expected: `19 checks, 0 failures`, exit 0.
- [ ] **Step 6: Commit** — `git add .gitignore Makefile test rssi_meter/signal_level.* && git commit -m "Add signal_level with host tests"`

---

### Task 2: selection — strongest node, labels, smoothing

**Files:**
- Create: `rssi_meter/selection.h`, `rssi_meter/selection.cpp`, `test/test_selection.cpp`
- Modify: `Makefile` (add `rssi_meter/selection.cpp`), `test/test_main.cpp` (call `test_selection()`)

**Interfaces:**
- Consumes: `test/check.h` from Task 1.
- Produces:
  ```cpp
  struct ScanResult { std::string ssid; std::string bssid; int rssi; }; // bssid "AA:BB:CC:DD:EE:FF"
  struct Pick { bool found; std::string bssid; int rssi; int nodeCount; };
  Pick pickStrongest(const std::vector<ScanResult>& results, const std::string& ssid);
  struct NodeName { const char* suffix; const char* name; };   // suffix like "3F:A1"
  std::string nodeLabel(const std::string& bssid, const NodeName* names, size_t count);
  class Smoother {
   public:
    int add(const std::string& bssid, int rssi);  // returns smoothed integer dBm
    void reset();
  };
  ```
- Rules: SSID match is exact and case-sensitive. `nodeCount` = number of matching results. On equal RSSI the earlier result wins. `nodeLabel` compares the last 5 characters of `bssid` to each `suffix` case-insensitively; no match → `"Node "` + last 5 chars uppercased. `Smoother` keeps the last 3 readings of the current BSSID; a different BSSID clears history first; return `std::lround` of the mean.

- [ ] **Step 1: Write failing tests** in `test/test_selection.cpp`, function `test_selection()`:

```cpp
std::vector<ScanResult> r = {
  {"Neighbor", "00:00:00:00:00:01", -40},
  {"Home",     "AA:BB:CC:DD:3F:A1", -61},
  {"Home",     "AA:BB:CC:DD:12:34", -55},
  {"Home",     "AA:BB:CC:DD:56:78", -55},
};
Pick p = pickStrongest(r, "Home");
CHECK(p.found); CHECK_EQ(p.bssid, std::string("AA:BB:CC:DD:12:34")); // tie → earlier
CHECK_EQ(p.rssi, -55); CHECK_EQ(p.nodeCount, 3);
CHECK(!pickStrongest(r, "home").found);            // case-sensitive SSID
CHECK(!pickStrongest({}, "Home").found);
CHECK_EQ(pickStrongest({}, "Home").nodeCount, 0);

NodeName names[] = {{"3f:a1", "Living Room"}};
CHECK_EQ(nodeLabel("AA:BB:CC:DD:3F:A1", names, 1), std::string("Living Room"));
CHECK_EQ(nodeLabel("aa:bb:cc:dd:12:34", names, 1), std::string("Node 12:34"));
CHECK_EQ(nodeLabel("AA:BB:CC:DD:12:34", nullptr, 0), std::string("Node 12:34"));

Smoother s;
CHECK_EQ(s.add("A", -50), -50);
CHECK_EQ(s.add("A", -54), -52);
CHECK_EQ(s.add("A", -58), -54);
CHECK_EQ(s.add("A", -62), -58);   // window of 3: -54,-58,-62
CHECK_EQ(s.add("B", -70), -70);   // node change resets
CHECK_EQ(s.add("A", -40), -40);   // flip back: no mixing with old A or B
CHECK_EQ(s.add("A", -41), -41);   // mean -40.5 → lround → -41
s.reset();
CHECK_EQ(s.add("A", -60), -60);
```

- [ ] **Step 2: Run** `make test`. Expected: compile failure (`selection.h` not found).
- [ ] **Step 3: Implement** `rssi_meter/selection.h/.cpp` per the Interfaces block (the Smoother may use a fixed `int[3]` plus count, no heap needed beyond the `std::string` BSSID).
- [ ] **Step 4: Run** `make test`. Expected: `0 failures`, exit 0.
- [ ] **Step 5: Commit** — `git add rssi_meter/selection.* test Makefile && git commit -m "Add node selection, labels and smoothing"`

---

### Task 3: Firmware — config, scanner, display, sketch

**Files:**
- Create: `rssi_meter/config.example.h`, `rssi_meter/scanner.h`, `rssi_meter/scanner.cpp`, `rssi_meter/display.h`, `rssi_meter/display.cpp`, `rssi_meter/rssi_meter.ino`
- Create (local only, ignored): `rssi_meter/config.h` copied from the example, for compiling

**Interfaces:**
- Consumes: everything in Tasks 1–2.
- Produces:
  ```cpp
  // config.example.h  (starts with #include "selection.h" for NodeName)
  #define WIFI_SSID "YourNetwork"
  static const NodeName NODE_NAMES[] = { /* {"3F:A1", "Living Room"}, */ {nullptr, nullptr} };
  // NODE_NAME_COUNT = entries before the {nullptr,nullptr} sentinel; nodeLabel gets that count.

  // scanner.h
  enum class ScanState { Idle, Running, Done, Failed };
  void scannerBegin();                       // WiFi.mode(WIFI_STA); WiFi.disconnect();
  void scannerStart();                       // WiFi.scanNetworks(true, false, false, 120)
  ScanState scannerPoll();                   // maps WiFi.scanComplete(): -1 Running, -2 Failed, >=0 Done
  std::vector<ScanResult> scannerTakeResults(); // reads SSID/BSSIDstr/RSSI, then WiFi.scanDelete()

  // display.h
  struct Reading { std::string label; int dbm; Rating rating; int nodeCount; };
  void displayBegin();                       // GPIO15 HIGH, GPIO38 HIGH, tft.init(), setRotation(1), fill black
  void showReading(const Reading& r);
  void showNotFound(const char* ssid);
  void showStale(const Reading& last, unsigned secondsAgo);
  ```
- Layout (pixels, landscape 320×170, 12 px margin): label Font 4 at (12,8); number Font 7 at (12,40); "dBm" Font 2 and rating word Font 4 (rating colour) to the right of the number at y 48 and 70; bar x 12–308, y 112, height 16, `Colors::Track` track with rating-colour fill of `barPercent` width; status line Font 2 at (12,146), `Colors::TextDim`.
- Copy: Reading status `nodeCountText(n) + " - scanning..."`; Not-found label `No network`, body `Not found` and `Looking for "<ssid>"`, status `0 nodes heard - scanning...`; Stale status `STALE - last scan failed <N>s ago` in `Colors::Amber`, number/rating/bar in `Colors::Stale`.
- Flicker: `display.cpp` remembers which screen and values are drawn. A screen change clears to black; within the same screen redraw only fields whose value changed, using `setTextColor(fg, TFT_BLACK)` with `setTextPadding` sized to the field so old text is overwritten, and redraw the bar only when width or colour changes.
- Sketch loop (no `delay` in `loop`): start a scan when idle; on `Done` → `pickStrongest` → found: `Smoother::add`, build `Reading`, `showReading`, remember as last good + time; not found: `Smoother::reset`, `showNotFound(WIFI_SSID)`. On `Failed` → if a last good reading exists `showStale(last, secondsSinceLast)`, else `showNotFound`; then start the next scan. While stale, refresh `showStale` once per second.

- [ ] **Step 1: Install verification toolchain.** `brew install arduino-cli`; `arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`; `arduino-cli core install esp32:esp32@2.0.14 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`; `arduino-cli lib install "TFT_eSPI@2.5.43"`; edit `~/Documents/Arduino/libraries/TFT_eSPI/User_Setup_Select.h` to comment out `#include <User_Setup.h>` and uncomment `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`. Run `arduino-cli core list`. Expected: `esp32:esp32 2.0.14`.
- [ ] **Step 2: Config guard.** `rssi_meter.ino` begins with `#if __has_include("config.h")` / `#include "config.h"` / `#else` / `#error "Copy rssi_meter/config.example.h to rssi_meter/config.h and set WIFI_SSID"` / `#endif`. With no `config.h`, run the compile command from Step 4. Expected: fails with exactly that `#error` message.
- [ ] **Step 3: Implement** `config.example.h`, `scanner.*`, `display.*`, `rssi_meter.ino` per the Interfaces block; copy `config.example.h` to `config.h`.
- [ ] **Step 4: Compile.** `arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,USBMode=hwcdc rssi_meter`. Expected: `Sketch uses … bytes` with no errors or warnings from project files. Also run `make test`: `0 failures`.
- [ ] **Step 5: Commit** — `git add rssi_meter && git commit -m "Add scanner, display and sketch loop"`; confirm with `git status --short` that `rssi_meter/config.h` is not staged.

---

### Task 4: README and on-device check

**Files:**
- Create: `README.md`

**Interfaces:**
- Consumes: settings in Global Constraints; `make test`; compile command from Task 3.

- [ ] **Step 1: Write README** with sections: What it does (one paragraph, 2.4 GHz-only note); Arduino IDE setup (board manager URL, install `esp32` **2.0.14** exactly, TFT_eSPI **2.5.43** via Library Manager, the `User_Setup_Select.h` edit with its file location on macOS, and a warning that updating TFT_eSPI overwrites that edit); Board settings (the list from Global Constraints); Configure (copy `config.example.h` → `config.h`, set `WIFI_SSID`, optional nicknames: power on, read the `Node XX:YY` label in each room, add `{"XX:YY", "Room"}`); Upload (open `rssi_meter/rssi_meter.ino`, select port, Upload; if upload fails hold BOOT, tap RST, release BOOT); Running tests (`make test`); Hardware checklist (the five checks from the spec's Testing section).
- [ ] **Step 2: Verify** every command in the README by running `make test` and the Task 3 compile command. Expected: both pass.
- [ ] **Step 3: Commit** — `git add README.md && git commit -m "Add README with setup and hardware checklist"`
- [ ] **Step 4: On-device check (user).** User uploads from Arduino IDE and runs the hardware checklist. Expected: all five checks pass; any failure goes to superpowers:systematic-debugging.
