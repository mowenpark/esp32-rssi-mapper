# Wi-Fi Signal Meter

A handheld Wi-Fi signal meter for the LilyGO T-Display-S3. Carry it around
and it shows which node of your eero mesh is strongest where you are, how
strong it is in dBm, a rating (EXCELLENT / GOOD / FAIR / POOR) and a colour
bar. It only scans, so it never needs your Wi-Fi password.

The ESP32-S3 radio is 2.4 GHz only, so readings reflect each node's 2.4 GHz
signal; 5 GHz is not visible to it.

| Smoothed dBm | Rating    |
|--------------|-----------|
| -50 or better | EXCELLENT |
| -51 to -60   | GOOD      |
| -61 to -70   | FAIR      |
| below -70    | POOR      |

## Arduino IDE setup (once)

1. **Board package.** Arduino IDE → Settings → *Additional boards manager
   URLs*, add
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
   Then Boards Manager → **esp32 by Espressif Systems** → install version
   **2.0.14** exactly. Newer versions (3.x) crash TFT_eSPI on this board, so
   don't accept the IDE's update prompts for this package.
2. **Display library.** Library Manager → **TFT_eSPI** by Bodmer → install
   version **2.5.43**.
3. **Select this board's display setup.** Open
   `~/Documents/Arduino/libraries/TFT_eSPI/User_Setup_Select.h` and:
   - comment out `#include <User_Setup.h>`
   - uncomment `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`

   Updating TFT_eSPI replaces this file, so redo this step after any update.

## Board settings (Tools menu)

| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| USB Mode | Hardware CDC and JTAG |
| Upload Mode | UART0 / Hardware CDC |

## Configure

1. Copy `rssi_meter/config.example.h` to `rssi_meter/config.h`
   (it's git-ignored, so your settings stay local).
2. Set `WIFI_SSID` to your eero network name, exactly as it appears
   (upper/lower case matters).
3. Optional nicknames: power the meter on next to each eero node, note the
   `Node XX:YY` label it shows, and add a line to `NODE_NAMES`, e.g.
   `{"3F:A1", "Living Room"},`. Keep the `{nullptr, nullptr}` line last.

If you forget step 1, the build stops with a message telling you to copy the
file.

## Upload

Open `rssi_meter/rssi_meter.ino` in the Arduino IDE, connect the board by
USB-C, pick its port under Tools → Port, and click Upload. If the upload
can't connect: hold **BOOT**, tap **RST**, release **BOOT**, then upload
again, and press **RST** once it finishes.

## Screens

- **Reading** — node name, smoothed dBm (average of the last 3 scans of that
  node), rating, bar, and how many of your nodes it can hear. Updates about
  every 2 seconds.
- **Not found** — your network isn't visible (or `WIFI_SSID` is wrong). It
  keeps scanning and recovers on its own.
- **Stale** — a scan failed; the last reading is shown in grey with how long
  ago it was taken.

The screen is blank for about 2 seconds after power-on while the first scan
runs.

## Running the tests

The rating, node-picking, nickname and smoothing logic is tested on your Mac,
no board needed:

```bash
make test
```

To compile the firmware from the command line (needs `arduino-cli` with the
same board package and library as above):

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,USBMode=hwcdc rssi_meter
```

## Hardware checklist

After uploading, check:

- [ ] Unplug USB and run from battery: the screen still lights up.
- [ ] Walk toward and away from a node: the dBm number and bar change.
- [ ] Move between rooms: the node label switches to the nearest node.
- [ ] Set a wrong `WIFI_SSID`, upload: the Not found screen appears.
- [ ] Add a nickname for a node: it shows the nickname instead of `Node XX:YY`.
