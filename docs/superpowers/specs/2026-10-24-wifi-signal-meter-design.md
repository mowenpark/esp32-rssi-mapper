# Wi-Fi Signal Meter — Design

Date: 2026-10-24
Status: Approved in conversation; pending written-spec review

## Purpose

A handheld Wi-Fi signal meter on a LilyGO T-Display-S3 (ESP32-S3, built-in
1.9" 320×170 ST7789 LCD). The user carries it around their apartment and reads
how strong their home Wi-Fi is in each room, to find weak spots and judge
where eero nodes sit well.

### What the user said

- Live meter only — nothing is recorded or exported.
- Home network is an eero mesh (several nodes, one network name).
- Screen shows the single strongest node, big and glanceable.
- Built and uploaded with the Arduino IDE.

### Assumptions

- Only the user's own network matters, not neighbours' networks.
- Powered by USB power bank or a LiPo on the board's battery connector.
- 2.4 GHz only: the ESP32-S3 radio cannot see 5 GHz, so readings reflect each
  node's 2.4 GHz radio.

### Success criteria

- Within ~2 s of walking into a room, the screen shows the strongest eero node
  and its signal in dBm with a colour-coded rating.
- The reading visibly changes when walking toward or away from a node.
- Works untethered on battery.
- No Wi-Fi password is stored on the device.

### Out of scope

Saving readings, room labels, data export, floor-plan maps, multiple-node
list view, button controls, battery indicator, 5 GHz.

## Approach

Scan-only: the board never joins the network. It runs repeated Wi-Fi scans,
keeps results whose SSID matches the configured network name, and shows the
strongest BSSID (node). Scanning needs no password and naturally sees every
mesh node; joining would only report the one node it is attached to.

Toolchain: Arduino IDE with the Espressif ESP32 board package and the
TFT_eSPI display library (the library LilyGO's own T-Display-S3 examples
use). There is a known incompatibility between some TFT_eSPI releases and
some ESP32 Arduino core 3.x releases on the S3; the implementation plan must
check LilyGO's current guidance and pin an exact, compatible board-package
version and TFT_eSPI version (plus the required TFT_eSPI setup selection for
this board) in the README.

## Project layout

```
esp32-rssi-mapper/
  rssi_meter/                 Arduino sketch folder (open in Arduino IDE)
    rssi_meter.ino            setup() + loop(): scan → select → render
    config.example.h          template: network name + optional node nicknames
    config.h                  user's copy; git-ignored
    scanner.h / scanner.cpp   one blocking Wi-Fi scan; returns raw scan results
    selection.h / selection.cpp  pure: filter by SSID, pick strongest, nickname lookup, smoothing
    signal_level.h / signal_level.cpp  pure: dBm → rating, colour, bar fill
    display.h / display.cpp   draws the screen states
  test/                       host-side C++ tests for the pure units
  README.md                   board-package/library versions, IDE settings, test command, hardware checklist
  .gitignore                  ignores rssi_meter/config.h
```

## Components

**config.h** (copied from `config.example.h`)
- `WIFI_SSID` — the eero network name to look for.
- `NODE_NAMES` — optional table of `{bssidSuffix, nickname}` pairs, e.g.
  `{"3F:A1", "Living Room"}`, matched against the last 5 characters of the
  BSSID (last two octets) case-insensitively.

**scanner** — wraps a blocking `WiFi.scanNetworks(async=false, …)` in station
mode with a short per-channel dwell (target full scan ≈1.5–2 s). Exposes one
call that runs a scan and returns its results as a list of
`{ssid, bssid, rssi}`, or reports failure. A result count of 0 counts as a
failure (the 2.0.14 core reports failed scans that way). Knows nothing about
the display.

Scans block on purpose. The async API in arduino-esp32 2.0.14 gives up after
20× the per-channel time while the radio is still scanning; the aborted
scan's "done" event is then credited to the next scan, so back-to-back async
scans never complete (seen on device: ~15,000 failures in 15 s, "Not found"
shown next to a node). Nothing on screen changes during a scan, so blocking
costs nothing visible except that the Stale screen's counter ticks once per
scan rather than once per second.

**selection** (pure, no Arduino headers)
- `pickStrongest(results, ssid)` → the strongest matching `{bssid, rssi}` plus
  the count of matching nodes, or "none found". On equal RSSI the earlier
  result wins.
- `nodeLabel(bssid, names)` → nickname if the suffix matches, else `…XX:YY`
  (last two octets).
- `Smoother` — rolling mean of the last 3 readings for the current node;
  resets when the strongest BSSID changes. Displayed value is rounded to an
  integer dBm.

**signal_level** (pure, no Arduino headers)

| dBm (smoothed) | Rating    | Colour  |
|----------------|-----------|---------|
| ≥ −50          | EXCELLENT | green   |
| −51 … −60      | GOOD      | lime    |
| −61 … −70      | FAIR      | yellow  |
| < −70          | POOR      | red     |

Bar fill = (dBm + 90) / 60, clamped to 4 %…100 % (−90 → minimum sliver,
−30 → full). Colours are defined as RGB565 constants alongside their mockup
hex values (`#22C55E`, `#A3E635`, `#FACC15`, `#F87171`).

**display** — takes a view model and draws one of three states; redraws only
regions whose value changed to avoid flicker. Landscape, black background.

## Screen states

Visual reference: Design canvas "RSSI Meter Screens"
(https://claude.ai/artifact/TjyGxoLBw2aVXQtMi5hs3h), drawn at 2× scale.

1. **Reading** — top: node label; middle: large dBm number with "dBm" and the
   rating word (in rating colour) beside it; full-width bar in rating colour on
   a dark track; bottom status line: "N nodes heard · scanning…".
2. **Not found** — node label area reads "No network"; "Not found" and
   `Looking for "<WIFI_SSID>"` in grey; empty bar; status "0 nodes heard ·
   scanning…".
3. **Stale** — last reading kept but drawn in grey (number, rating, bar);
   status line in amber: "STALE · last scan failed Ns ago".

## Data flow

```
loop():
  scan (blocks ~1.5–2 s):
     ok      → pickStrongest → smoother → signal_level → display(Reading | NotFound)
     failed  → display(Stale) using last reading (or NotFound if none yet)
  update the "Ns ago" counter on the Stale screen (at most once per second)
```

## Error handling

- **Network not found:** Not-found screen; keep scanning; recovers
  automatically when the network reappears. Smoother resets.
- **Scan failure:** keep the previous reading, show Stale, start the next
  scan immediately.
- **Battery power:** drive GPIO 15 (LCD power enable) HIGH at startup and turn
  on the backlight (GPIO 38), so the screen works when not on USB.

## Testing

- **Host tests** (`test/`, compiled with the Mac's `c++`, one documented
  command): `signal_level` thresholds and boundaries (−50, −51, −60, −61, −70,
  −71), bar clamping; `pickStrongest` SSID filtering, tie handling, empty list;
  `nodeLabel` nickname hit, case-insensitivity, fallback format; `Smoother`
  averaging and reset on node change.
- **Hardware checklist** (README): screen lights on battery; number changes
  walking toward/away from a node; strongest node label switches between
  rooms; wrong `WIFI_SSID` shows Not-found; nicknamed node shows its nickname.
