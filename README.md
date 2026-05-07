# MacroLad

MacroLad is an ESP32-S3 USB macropad firmware with a built-in Wi-Fi configuration page, per-key shortcuts, RGB effects, OLED status screens, profile support, and animated RoboEyes.

It is designed around a hand-wired 10-key build, but the firmware can be altered for different key counts, GPIO pins, LED layouts, and display wiring.

![MacroLad device photo](images/macrolad-device.gif)

The photographed build uses the [shortcut_deck 3D-printable model by the_pragma_way on Thingiverse](https://www.thingiverse.com/thing:7151316). If you build your own MacroLad with different printed parts, check the switch spacing, OLED opening, USB access, LED clearance, and GPIO wiring before flashing.

> **Demo WebUI:** try the browser-only mockup before flashing:  
> <https://casketpizza.github.io/MacroLad/>

---

## Features

- **10 configurable keys**
  - Normal press action per key
  - Long-press action per key
  - Key 10 long-hold reserved for the on-device OLED menu
- **USB HID keyboard output**
  - Letters, numbers, function keys, arrows, navigation keys, and common shortcuts
  - Media controls such as play/pause, next, previous, volume, and mute
- **Custom key combinations**
  - Build shortcuts from parts such as Ctrl, Shift, Alt, GUI/Win, letters, numbers, arrows, function keys, and navigation keys
  - Saved combinations appear in the normal key action dropdowns
- **Profiles**
  - Up to 5 profiles
  - Add, copy, rename, delete, switch, export, and import profiles
- **Wi-Fi setup**
  - Starts a `MacroLad` setup hotspot
  - Can connect to a LAN so the WebUI is reachable from your network
  - Shows IP address on the OLED
- **WebUI configuration**
  - Configure keys, long-press actions, LED behaviour, display options, clock options, profiles, Wi-Fi, and shortcuts
  - Served directly from the ESP32 using `WebServer`
- **RGB lighting**
  - Global lighting modes: solid, breathing, rainbow, chase, sparkle
  - Per-key press effects: ambient pulse, white pulse, rainbow burst, rainbow wipe, trails, comet, scanner, pop, sparkle pop, strobe, and none
- **OLED display**
  - 128×64 SSD1306 display
  - Idle pages for IP address, clock, and animated RoboEyes
  - Configurable display timings and brightness
  - On-device OLED menu
- **Clock display**
  - NTP clock sync when connected to Wi-Fi
  - Multiple time zones
  - 12/24-hour options
  - Date format options
  - Multiple Adafruit GFX fonts
- **Animated RoboEyes**
  - Idle eye personalities such as playful, calm, dramatic, sleepy, nervous, cyclops, happy, angry, curious, glitchy, judgy, alert, and more
  - Per-key eye animations and eye positions
- **Settings storage**
  - Uses ESP32 `Preferences` / NVS so settings survive reboot

---

## Hardware used in this build

The current hardware section is **entered by hand in the sketch**. If you use different parts, you must manually change the constants in the firmware. This is not auto-detected.

Current wiring:

| Part | Firmware setting |
|---|---|
| Board | ESP32-S3 board with native USB / USB-OTG support |
| Keys | 10 mechanical switches, each wired from GPIO to GND |
| Key GPIOs | `1, 2, 3, 4, 5, 6, 10, 11, 12, 13` |
| RGB data pin | GPIO `7` |
| RGB LED count | `10` |
| OLED | SSD1306 128×64 I2C OLED |
| OLED SDA | GPIO `8` |
| OLED SCL | GPIO `9` |
| Wi-Fi AP name | `MacroLad` |

### Printed enclosure / case

This build uses the [shortcut_deck model by the_pragma_way on Thingiverse](https://www.thingiverse.com/thing:7151316). The firmware does not depend on this exact shell, but the default key layout, OLED position, and photographed build are based around that printed design.

Relevant firmware constants:

```cpp
static const uint8_t KEY_COUNT = 10;
static const uint8_t KEY_PINS[KEY_COUNT] = {1, 2, 3, 4, 5, 6, 10, 11, 12, 13};

static const uint8_t STRIP_LED_COUNT = 10;
static const uint8_t RGB_PIN = 7;

static const uint8_t OLED_SDA = 8;
static const uint8_t OLED_SCL = 9;

static const uint8_t OLED_WIDTH = 128;
static const uint8_t OLED_HEIGHT = 64;
static const char *AP_SSID = "MacroLad";
```

### Potentially compatible hardware

MacroLad should be adaptable to:

- ESP32-S3 boards with native USB / TinyUSB HID support
- Other switch counts, if `KEY_COUNT`, `KEY_PINS`, and related UI assumptions are updated
- Momentary buttons or keyboard switches wired from GPIO to GND
- WS2812 / NeoPixel-compatible RGB LEDs
- SSD1306 I2C OLEDs, especially 128×64 modules
- Other I2C pins, if `OLED_SDA` and `OLED_SCL` are changed
- Other RGB data pins, if `RGB_PIN` is changed

### Important board setting for USB keyboard output

USB keyboard output requires an ESP32-S3 build configuration with USB HID enabled. The WebUI can still run with a USB serial-only build, but the device will **not** act as a keyboard unless TinyUSB HID is enabled.

In Arduino IDE, check the **Tools** menu for your selected ESP32-S3 board and use settings equivalent to:

```text
USB Mode: USB-OTG (TinyUSB)
USB CDC On Boot: Enabled
```

The exact wording can vary between ESP32 board packages and board definitions, but the important part is that the build must use **TinyUSB / USB-OTG HID**, not USB serial-only / hardware CDC mode.

---

## Libraries used

Install these through Arduino Library Manager where possible:

| Library / component | Used for | Link | License |
|---|---|---|---|
| Adafruit GFX | Display drawing, fonts, graphics primitives | <https://github.com/adafruit/Adafruit-GFX-Library> | BSD |
| Adafruit SSD1306 | SSD1306 OLED driver | <https://github.com/adafruit/Adafruit_SSD1306> | BSD |
| Adafruit NeoPixel | WS2812 / NeoPixel RGB LEDs | <https://github.com/adafruit/Adafruit_NeoPixel> | LGPL-3.0 |
| FluxGarage RoboEyes | Animated OLED eyes | <https://github.com/FluxGarage/RoboEyes> | GPL-3.0 |
| ESP32 Arduino core | Wi-Fi, WebServer, Preferences, Wire, USB HID support | <https://github.com/espressif/arduino-esp32> | LGPL-2.1 |

Built-in / core headers used by the firmware include:

- `WiFi.h`
- `WebServer.h`
- `Preferences.h`
- `Wire.h`
- `USB.h`
- `USBHIDKeyboard.h`
- `USBHIDConsumerControl.h`
- `time.h`

---

## GPL-3.0 licensing notes

MacroLad is released under the **GNU General Public License v3.0**.

Required GPL-3.0 related links:

- MacroLad source: <https://github.com/CasketPizza/MacroLad>
- GPL-3.0 license text: <https://www.gnu.org/licenses/gpl-3.0.en.html>
- SPDX GPL-3.0-only reference: <https://spdx.org/licenses/GPL-3.0-only.html>
- FluxGarage RoboEyes, the GPL-3.0 library used for the animated eyes: <https://github.com/FluxGarage/RoboEyes>

Because MacroLad uses GPL-3.0 code, keep the project source available when sharing firmware builds, modified versions, or redistributions. Include a `LICENSE` file containing the GPL-3.0 license text in the repository.

---

## WebUI demo

The online demo is a static browser mockup of the MacroLad WebUI. It does **not** connect to hardware and it does **not** save settings to a real ESP32.

Use it to preview the layout and explore the kind of settings available before flashing:

<https://casketpizza.github.io/MacroLad/>

---

## Flashing / setup overview

1. Install the ESP32 board package in Arduino IDE.
2. Install the required libraries listed above.
3. Open the MacroLad sketch.
4. Select your ESP32-S3 board.
5. Select USB settings that enable TinyUSB / USB-OTG HID.
6. Check the hardware constants match your wiring.
7. Flash the board.
8. Connect to the `MacroLad` Wi-Fi hotspot, or connect it to your LAN through the WebUI.
9. Open the displayed IP address in a browser.
10. Configure keys, profiles, LEDs, display options, clock, and Wi-Fi.

---

## Wiring notes

Each button should connect its GPIO pin to GND. The firmware uses internal pull-ups, so the key reads as pressed when the GPIO is pulled low.

Basic key wiring:

```text
GPIO ---- switch ---- GND
```

RGB LED strip:

```text
ESP32-S3 GPIO 7 ---- DIN on LED strip
5V / 3.3V as required by your LEDs
GND shared with ESP32
```

OLED:

```text
OLED SDA ---- GPIO 8
OLED SCL ---- GPIO 9
OLED VCC ---- 3.3V
OLED GND ---- GND
```

Use level shifting or appropriate power wiring if your LED strip requires 5 V data or higher current than the board can safely supply.

---

## Customising hardware

For a different build, edit the hardware constants manually.

Examples:

### Change the key pins

```cpp
static const uint8_t KEY_COUNT = 6;
static const uint8_t KEY_PINS[KEY_COUNT] = {1, 2, 3, 4, 5, 6};
```

### Change the RGB pin and LED count

```cpp
static const uint8_t STRIP_LED_COUNT = 12;
static const uint8_t RGB_PIN = 14;
```

### Change OLED I2C pins

```cpp
static const uint8_t OLED_SDA = 8;
static const uint8_t OLED_SCL = 9;
```

If you change the key count, check the WebUI, profile storage, OLED menu behaviour, and any key-specific logic. Key 10 currently has special menu behaviour, so smaller layouts may need that logic changed.

---


## Credits

- Firmware by Casket Pizza
- 3D-printable enclosure/model: [shortcut_deck by the_pragma_way on Thingiverse](https://www.thingiverse.com/thing:7151316)
- Animated eyes powered by FluxGarage RoboEyes
- Display and LED libraries by Adafruit
- ESP32 platform support by Espressif
