#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <string.h>
#include <time.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoOblique12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/TomThumb.h>
#include "esp_system.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc_caps.h"
#include <FluxGarage_RoboEyes.h>

#ifndef SOC_USB_OTG_SUPPORTED
#define SOC_USB_OTG_SUPPORTED 0
#endif

#ifndef CONFIG_TINYUSB_HID_ENABLED
#define CONFIG_TINYUSB_HID_ENABLED 0
#endif

#if SOC_USB_OTG_SUPPORTED && CONFIG_TINYUSB_HID_ENABLED
#define MACROPAD_USB_HID 1
#include <USB.h>
#include <USBHIDConsumerControl.h>
#include <USBHIDKeyboard.h>
#else
#define MACROPAD_USB_HID 0

#define KEY_LEFT_CTRL    0x80
#define KEY_LEFT_SHIFT   0x81
#define KEY_LEFT_ALT     0x82
#define KEY_LEFT_GUI     0x83
#define KEY_RETURN       0xB0
#define KEY_ESC          0xB1
#define KEY_BACKSPACE    0xB2
#define KEY_TAB          0xB3
#define KEY_CAPS_LOCK    0xC1
#define KEY_F1           0xC2
#define KEY_F2           0xC3
#define KEY_F3           0xC4
#define KEY_F4           0xC5
#define KEY_F5           0xC6
#define KEY_F6           0xC7
#define KEY_F7           0xC8
#define KEY_F8           0xC9
#define KEY_F9           0xCA
#define KEY_F10          0xCB
#define KEY_F11          0xCC
#define KEY_F12          0xCD
#define KEY_PRINT_SCREEN 0xCE
#define KEY_SCROLL_LOCK  0xCF
#define KEY_PAUSE        0xD0
#define KEY_INSERT       0xD1
#define KEY_HOME         0xD2
#define KEY_PAGE_UP      0xD3
#define KEY_DELETE       0xD4
#define KEY_END          0xD5
#define KEY_PAGE_DOWN    0xD6
#define KEY_RIGHT_ARROW  0xD7
#define KEY_LEFT_ARROW   0xD8
#define KEY_DOWN_ARROW   0xD9
#define KEY_UP_ARROW     0xDA
#define CONSUMER_CONTROL_PLAY_PAUSE       0x00CD
#define CONSUMER_CONTROL_SCAN_NEXT        0x00B5
#define CONSUMER_CONTROL_SCAN_PREVIOUS    0x00B6
#define CONSUMER_CONTROL_STOP             0x00B7
#define CONSUMER_CONTROL_MUTE             0x00E2
#define CONSUMER_CONTROL_VOLUME_INCREMENT 0x00E9
#define CONSUMER_CONTROL_VOLUME_DECREMENT 0x00EA

class USBHIDKeyboard {
public:
  void begin() {}
  size_t press(uint8_t) { return 0; }
  void releaseAll() {}
  size_t print(const String &) { return 0; }
};

class USBHIDConsumerControl {
public:
  void begin() {}
  size_t press(uint16_t) { return 0; }
  size_t release() { return 0; }
};
#endif

// Selected GPIO wiring. Buttons must connect each GPIO to GND.
static const char *FIRMWARE_VERSION = "1.0.0";
static const uint8_t KEY_COUNT = 10;
static const uint8_t KEY_PINS[KEY_COUNT] = {1, 2, 3, 4, 5, 6, 10, 11, 12, 13};
static const uint8_t STRIP_LED_COUNT = 10;
static const uint8_t RGB_PIN = 7;
static const uint8_t OLED_SDA = 8;
static const uint8_t OLED_SCL = 9;

static const uint8_t OLED_WIDTH = 128;
static const uint8_t OLED_HEIGHT = 64;
static const char *AP_SSID = "MacroLad";
static const uint16_t DEBOUNCE_MS = 25;

struct Choice {
  const char *value;
  const char *label;
};

struct KeySetting {
  String action;
  String longAction;
  uint16_t longPressMs;
  bool useEyes;
  String eyeAnim;
  String eyePos;
  String ledEffect;
};

struct EffectChoice {
  const char *value;
  const char *label;
};

struct DisplayChoice {
  const char *value;
  const char *label;
};

struct ClockFontChoice {
  const char *value;
  const char *label;
  const GFXfont *font;
};

static const Choice ACTIONS[] = {
  {"", "Disabled"},
  {"KEY:A", "A"}, {"KEY:B", "B"}, {"KEY:C", "C"}, {"KEY:D", "D"}, {"KEY:E", "E"},
  {"KEY:F", "F"}, {"KEY:G", "G"}, {"KEY:H", "H"}, {"KEY:I", "I"}, {"KEY:J", "J"},
  {"KEY:K", "K"}, {"KEY:L", "L"}, {"KEY:M", "M"}, {"KEY:N", "N"}, {"KEY:O", "O"},
  {"KEY:P", "P"}, {"KEY:Q", "Q"}, {"KEY:R", "R"}, {"KEY:S", "S"}, {"KEY:T", "T"},
  {"KEY:U", "U"}, {"KEY:V", "V"}, {"KEY:W", "W"}, {"KEY:X", "X"}, {"KEY:Y", "Y"},
  {"KEY:Z", "Z"},
  {"KEY:0", "0"}, {"KEY:1", "1"}, {"KEY:2", "2"}, {"KEY:3", "3"}, {"KEY:4", "4"},
  {"KEY:5", "5"}, {"KEY:6", "6"}, {"KEY:7", "7"}, {"KEY:8", "8"}, {"KEY:9", "9"},
  {"KEY:ENTER", "Enter"}, {"KEY:ESC", "Escape"}, {"KEY:TAB", "Tab"}, {"KEY:SPACE", "Space"},
  {"KEY:BACKSPACE", "Backspace"}, {"KEY:DELETE", "Delete"}, {"KEY:INSERT", "Insert"},
  {"KEY:HOME", "Home"}, {"KEY:END", "End"}, {"KEY:PAGE_UP", "Page Up"}, {"KEY:PAGE_DOWN", "Page Down"},
  {"KEY:UP", "Arrow Up"}, {"KEY:DOWN", "Arrow Down"}, {"KEY:LEFT", "Arrow Left"}, {"KEY:RIGHT", "Arrow Right"},
  {"KEY:CAPS_LOCK", "Caps Lock"}, {"KEY:PRINT_SCREEN", "Print Screen"},
  {"KEY:F1", "F1"}, {"KEY:F2", "F2"}, {"KEY:F3", "F3"}, {"KEY:F4", "F4"}, {"KEY:F5", "F5"},
  {"KEY:F6", "F6"}, {"KEY:F7", "F7"}, {"KEY:F8", "F8"}, {"KEY:F9", "F9"}, {"KEY:F10", "F10"},
  {"KEY:F11", "F11"}, {"KEY:F12", "F12"},
  {"KEY:CTRL+C", "Copy"}, {"KEY:CTRL+V", "Paste"}, {"KEY:CTRL+X", "Cut"}, {"KEY:CTRL+Z", "Undo"},
  {"KEY:CTRL+Y", "Redo"}, {"KEY:CTRL+S", "Save"}, {"KEY:CTRL+A", "Select All"}, {"KEY:ALT+TAB", "Alt Tab"},
  {"KEY:GUI+R", "Run Dialog"}, {"KEY:CTRL+ALT+DELETE", "Ctrl Alt Delete"},
  {"MEDIA:PLAY_PAUSE", "Play / Pause"}, {"MEDIA:NEXT", "Next Track"}, {"MEDIA:PREVIOUS", "Previous Track"},
  {"MEDIA:STOP", "Stop"}, {"MEDIA:VOLUME_UP", "Volume Up"}, {"MEDIA:VOLUME_DOWN", "Volume Down"}, {"MEDIA:MUTE", "Mute"}
};

static const Choice COMBO_PARTS[] = {
  {"", "None"},
  {"CTRL", "Ctrl"}, {"SHIFT", "Shift"}, {"ALT", "Alt"}, {"GUI", "Win / GUI"},
  {"A", "A"}, {"B", "B"}, {"C", "C"}, {"D", "D"}, {"E", "E"}, {"F", "F"}, {"G", "G"}, {"H", "H"}, {"I", "I"}, {"J", "J"},
  {"K", "K"}, {"L", "L"}, {"M", "M"}, {"N", "N"}, {"O", "O"}, {"P", "P"}, {"Q", "Q"}, {"R", "R"}, {"S", "S"}, {"T", "T"},
  {"U", "U"}, {"V", "V"}, {"W", "W"}, {"X", "X"}, {"Y", "Y"}, {"Z", "Z"},
  {"0", "0"}, {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"}, {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"}, {"9", "9"},
  {"ENTER", "Enter"}, {"ESC", "Escape"}, {"TAB", "Tab"}, {"SPACE", "Space"}, {"BACKSPACE", "Backspace"},
  {"DELETE", "Delete"}, {"INSERT", "Insert"}, {"HOME", "Home"}, {"END", "End"}, {"PAGE_UP", "Page Up"}, {"PAGE_DOWN", "Page Down"},
  {"UP", "Arrow Up"}, {"DOWN", "Arrow Down"}, {"LEFT", "Arrow Left"}, {"RIGHT", "Arrow Right"},
  {"CAPS_LOCK", "Caps Lock"}, {"PRINT_SCREEN", "Print Screen"},
  {"F1", "F1"}, {"F2", "F2"}, {"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"}, {"F6", "F6"},
  {"F7", "F7"}, {"F8", "F8"}, {"F9", "F9"}, {"F10", "F10"}, {"F11", "F11"}, {"F12", "F12"}
};

static const uint8_t CUSTOM_COMBO_COUNT = 12;
String customCombos[CUSTOM_COMBO_COUNT];
static const uint8_t MAX_PROFILES = 5;
uint8_t profileCount = 1;
uint8_t activeProfile = 0;
String profileNames[MAX_PROFILES];

static const EffectChoice EFFECTS[] = {
  {"AMBIENT_PULSE", "Ambient pulse"},
  {"WHITE_PULSE", "White pulse"},
  {"RAINBOW_BURST", "Rainbow burst"},
  {"RAINBOW_WIPE", "Rainbow wipe"},
  {"TRAIL", "Color trail"},
  {"RAINBOW_TRAIL", "Rainbow trail"},
  {"COMET", "Comet"},
  {"RAINBOW_COMET", "Rainbow comet"},
  {"COLOR_WIPE", "Color wipe"},
  {"SCANNER", "Scanner"},
  {"POP", "Color pop"},
  {"SPARKLE_POP", "Sparkle pop"},
  {"STROBE", "White strobe"},
  {"NONE", "No press effect"}
};

static const EffectChoice LIGHTING_EFFECTS[] = {
  {"SOLID", "Solid"},
  {"BREATH", "Breathing"},
  {"RAINBOW", "Rainbow"},
  {"CHASE", "Color chase"},
  {"SPARKLE", "Sparkle"}
};

static const DisplayChoice TIME_ZONES[] = {
  {"UTC0", "UTC"},
  {"AEST-10AEDT,M10.1.0,M4.1.0/3", "Australia/Sydney"},
  {"ACST-9:30ACDT,M10.1.0,M4.1.0/3", "Australia/Adelaide"},
  {"AEST-10", "Australia/Brisbane"},
  {"ACST-9:30", "Australia/Darwin"},
  {"AWST-8", "Australia/Perth"},
  {"NZST-12NZDT,M9.5.0,M4.1.0/3", "Pacific/Auckland"},
  {"CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45", "Pacific/Chatham"},
  {"GMT0BST,M3.5.0/1,M10.5.0", "Europe/London"},
  {"IST-1GMT0,M10.5.0,M3.5.0/1", "Europe/Dublin"},
  {"CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Berlin"},
  {"PAR-1PARDT,M3.5.0,M10.5.0/3", "Europe/Paris"},
  {"ROM-1ROMDT,M3.5.0,M10.5.0/3", "Europe/Rome"},
  {"CET-1CEST,M3.5.0,M10.5.0/3|Europe/Amsterdam", "Europe/Amsterdam"},
  {"CET-1CEST,M3.5.0,M10.5.0/3|Europe/Madrid", "Europe/Madrid"},
  {"CET-1CEST,M3.5.0,M10.5.0/3|Europe/Stockholm", "Europe/Stockholm"},
  {"CET-1CEST,M3.5.0,M10.5.0/3|Europe/Warsaw", "Europe/Warsaw"},
  {"EET-2EEST,M3.5.0/3,M10.5.0/4", "Europe/Athens"},
  {"EET-2EEST,M3.5.0/3,M10.5.0/4|Europe/Helsinki", "Europe/Helsinki"},
  {"EET-2EEST,M3.5.0/3,M10.5.0/4|Europe/Bucharest", "Europe/Bucharest"},
  {"TRT-3", "Europe/Istanbul"},
  {"EST5EDT,M3.2.0,M11.1.0", "US/Eastern"},
  {"CST6CDT,M3.2.0,M11.1.0", "US/Central"},
  {"MST7MDT,M3.2.0,M11.1.0", "US/Mountain"},
  {"PST8PDT,M3.2.0,M11.1.0", "US/Pacific"},
  {"MST7", "US/Arizona"},
  {"AKST9AKDT,M3.2.0,M11.1.0", "US/Alaska"},
  {"HST10", "US/Hawaii"},
  {"AST4ADT,M3.2.0,M11.1.0", "Canada/Atlantic"},
  {"NST3:30NDT,M3.2.0,M11.1.0", "Canada/Newfoundland"},
  {"EST5EDT,M3.2.0,M11.1.0|Canada/Toronto", "Canada/Toronto"},
  {"CST6CDT,M3.2.0,M11.1.0|Canada/Winnipeg", "Canada/Winnipeg"},
  {"MST7MDT,M3.2.0,M11.1.0|Canada/Edmonton", "Canada/Edmonton"},
  {"PST8PDT,M3.2.0,M11.1.0|Canada/Vancouver", "Canada/Vancouver"},
  {"CST6CDT,M4.1.0,M10.5.0", "America/Mexico City"},
  {"MST7MDT,M4.1.0,M10.5.0", "America/Chihuahua"},
  {"PST8PDT,M3.2.0,M11.1.0", "America/Tijuana"},
  {"BRT3", "America/Sao Paulo"},
  {"ART3", "America/Argentina/Buenos Aires"},
  {"CLT4CLST,M9.1.6/24,M4.1.6/24", "America/Santiago"},
  {"COT5", "America/Bogota"},
  {"PET5", "America/Lima"},
  {"WET0WEST,M3.5.0/1,M10.5.0", "Atlantic/Canary"},
  {"SAST-2", "Africa/Johannesburg"},
  {"WAT-1", "Africa/Lagos"},
  {"EET-2", "Africa/Cairo"},
  {"EAT-3", "Africa/Nairobi"},
  {"MSK-3", "Europe/Moscow"},
  {"IST-2IDT,M3.4.4/26,M10.5.0", "Asia/Jerusalem"},
  {"GST-4", "Asia/Dubai"},
  {"IST-5:30", "Asia/Kolkata"},
  {"NPT-5:45", "Asia/Kathmandu"},
  {"ICT-7", "Asia/Bangkok"},
  {"WIB-7", "Asia/Jakarta"},
  {"CST-8", "Asia/Shanghai"},
  {"HKT-8", "Asia/Hong Kong"},
  {"SGT-8", "Asia/Singapore"},
  {"PHT-8", "Asia/Manila"},
  {"CST-8", "Asia/Taipei"},
  {"KST-9", "Asia/Seoul"},
  {"JST-9", "Asia/Tokyo"},
  {"FJT-12", "Pacific/Fiji"}
};

static const DisplayChoice DATE_FORMATS[] = {
  {"DOW_D_MON", "Mon 07 May"},
  {"D_MON_Y", "07 May 2026"},
  {"YMD", "2026-05-07"},
  {"DMY", "07/05/2026"},
  {"MDY", "05/07/2026"}
};

static const DisplayChoice AMPM_POSITIONS[] = {
  {"AFTER", "After time"},
  {"BEFORE", "Before time"}
};

static const ClockFontChoice CLOCK_FONTS[] = {
  {"DEFAULT", "Default", NULL},
  {"SANS9", "Sans", &FreeSans9pt7b},
  {"SANS", "Sans bold", &FreeSansBold12pt7b},
  {"SANSOBL", "Sans italic", &FreeSansOblique12pt7b},
  {"SANSBOLDOBL", "Sans bold italic", &FreeSansBoldOblique12pt7b},
  {"MONO9", "Mono", &FreeMono9pt7b},
  {"MONO", "Mono bold", &FreeMonoBold12pt7b},
  {"MONOOBL", "Mono italic", &FreeMonoOblique12pt7b},
  {"MONOBOLDOBL", "Mono bold italic", &FreeMonoBoldOblique12pt7b},
  {"SERIF9", "Serif", &FreeSerif9pt7b},
  {"SERIF", "Serif bold", &FreeSerifBold12pt7b},
  {"SERIFIT", "Serif italic", &FreeSerifItalic12pt7b},
  {"SERIFBOLDIT", "Serif bold italic", &FreeSerifBoldItalic12pt7b},
  {"ORG", "Pixel compact", &Org_01},
  {"PICO", "Pixel tiny", &Picopixel},
  {"TINY3", "Pixel 3x3", &Tiny3x3a2pt7b},
  {"TOM", "Pixel tall", &TomThumb}
};

static const DisplayChoice EYE_PERSONALITIES[] = {
  {"PLAYFUL", "Playful mix"},
  {"CALM", "Calm"},
  {"DRAMATIC", "Dramatic"},
  {"SLEEPY", "Sleepy"},
  {"NERVOUS", "Nervous"},
  {"CYCLOPS", "Cyclops"},
  {"HAPPY", "Happy"},
  {"ANGRY", "Angry"},
  {"LOOKOUT", "Lookout"},
  {"WATCHER", "Watcher"},
  {"CURIOUS", "Curious"},
  {"GLITCHY", "Glitchy"},
  {"JUDGY", "Judgy"},
  {"CHILL", "Chill blink"},
  {"CYCLOPS_SCAN", "Cyclops scan"},
  {"DREAMY", "Dreamy"},
  {"ALERT", "Alert"}
};

static const DisplayChoice KEY_EYE_ANIMS[] = {
  {"BLINK", "Blink"},
  {"DOUBLE_BLINK", "Double blink"},
  {"WINK_LEFT", "Wink left"},
  {"WINK_RIGHT", "Wink right"},
  {"HAPPY", "Happy"},
  {"ANGRY", "Angry"},
  {"TIRED", "Tired"},
  {"LAUGH", "Laugh"},
  {"SHAKE", "Shake"},
  {"CONFUSED", "Confused"},
  {"FLICKER", "Flicker"},
  {"SIDE_EYE", "Side eye"},
  {"SCAN", "Scan"},
  {"DIZZY", "Dizzy"},
  {"PANIC", "Panic"},
  {"SUSPICIOUS", "Suspicious"},
  {"EXCITED", "Excited"},
  {"GLITCH", "Glitch"},
  {"SLEEP_BLINK", "Sleep blink"},
  {"JUDGEMENT", "Judgement"},
  {"STARTLED", "Startled"},
  {"CYCLOPS_BLINK", "Cyclops blink"},
  {"SWEAT_DROP", "Sweat drop"},
  {"CURIOUS_PEEK", "Curious peek"},
  {"GRUMPY_LOOK", "Grumpy look"}
};

static const DisplayChoice EYE_POSITIONS[] = {
  {"DEFAULT", "Center"},
  {"N", "Top"},
  {"NE", "Top right"},
  {"E", "Right"},
  {"SE", "Bottom right"},
  {"S", "Bottom"},
  {"SW", "Bottom left"},
  {"W", "Left"},
  {"NW", "Top left"}
};

Adafruit_NeoPixel pixels(STRIP_LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
RoboEyes<Adafruit_SSD1306> roboEyes(display);
Preferences prefs;
WebServer server(80);
USBHIDKeyboard keyboard;
USBHIDConsumerControl consumer;

KeySetting settings[KEY_COUNT];
bool stableDown[KEY_COUNT];
bool lastRawDown[KEY_COUNT];
bool actionSentOnPress[KEY_COUNT];
bool longActionFired[KEY_COUNT];
unsigned long changedAt[KEY_COUNT];
unsigned long keyPressedAt[KEY_COUNT];
bool oledOk = false;
uint8_t oledAddr = 0;
String statusLine = "Starting";
unsigned long oledReturnAt = 0;
unsigned long nextAddressRefresh = 0;
unsigned long nextWifiCheckAt = 0;
bool setupHotspotActive = false;
bool lastLanConnected = false;
uint16_t oledReturnMs = 1000;
String pressEffect = "AMBIENT_PULSE";
String activePressEffect = "AMBIENT_PULSE";
uint32_t ambientColor = 0x2040FF;
String lightingEffect = "SOLID";
uint8_t lightingSpeed = 5;
uint8_t ledBrightness = 80;
unsigned long lastLightingFrameAt = 0;
unsigned long keyFlashUntil = 0;
unsigned long keyFlashStartedAt = 0;
uint16_t keyFlashFrame = 0;
uint16_t lightingFrame = 0;
String eyePersonality = "PLAYFUL";
uint16_t addressScreenMs = 3000;
uint16_t clockScreenMs = 3000;
uint16_t eyeScreenMs = 5000;
bool screenEnabled = true;
bool eyesEnabled = true;
bool addressScreenEnabled = true;
bool clockScreenEnabled = false;
bool eyeScreenEnabled = true;
bool showPressedOnOled = true;
uint8_t oledBrightness = 255;
String clockTimeZone = "UTC0";
bool clockUse24Hour = true;
bool clockShowAmPm = true;
bool clockAmPmAfter = true;
bool clockShowTimeZone = true;
bool clockShowDate = true;
String clockDateFormat = "DOW_D_MON";
String clockFont = "DEFAULT";
bool eyesActive = false;
unsigned long displayModeUntil = 0;
uint8_t idleScreenIndex = 0;
uint8_t activeIdleScreen = 255;
unsigned long key10HoldStartedAt = 0;
bool key10HoldHandled = false;
unsigned long nextEyeActionAt = 0;
String activeKeyEyeAnim = "";
uint8_t activeKeyEyeStep = 0;
unsigned long nextKeyEyeStepAt = 0;
unsigned long restartAt = 0;
bool usbUploadModePending = false;

enum OledMenuPage {
  MENU_TOP,
  MENU_WIFI,
  MENU_LEDS,
  MENU_LED_KEY_SELECT,
  MENU_LED_KEY_EFFECT,
  MENU_SCREEN,
  MENU_CLOCK_SETTINGS,
  MENU_CLOCK_FONT,
  MENU_CLOCK_TIMEZONE,
  MENU_CLOCK_DATE_FORMAT,
  MENU_PROFILES,
  MENU_RESET_CONFIRM,
  MENU_UPDATE_CONFIRM,
  MENU_UPDATE_NOW,
  MENU_MESSAGE
};

bool oledMenuOpen = false;
OledMenuPage oledMenuPage = MENU_TOP;
uint8_t oledMenuIndex = 0;
uint8_t oledMenuLedKey = 0;
String oledMenuMessageTitle = "";
String oledMenuMessageBody = "";

static String htmlEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '&') out += F("&amp;");
    else if (c == '<') out += F("&lt;");
    else if (c == '>') out += F("&gt;");
    else if (c == '"') out += F("&quot;");
    else if (c == '\'') out += F("&#39;");
    else out += c;
  }
  return out;
}

static String jsonEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\') out += F("\\\\");
    else if (c == '"') out += F("\\\"");
    else if (c == '\n') out += F("\\n");
    else if (c == '\r') out += F("\\r");
    else out += c;
  }
  return out;
}

static String colorHex(uint32_t color) {
  char buf[8];
  snprintf(buf, sizeof(buf), "#%06lX", color & 0xFFFFFF);
  return String(buf);
}

static uint32_t parseColor(String value, uint32_t fallback) {
  value.trim();
  if (value.startsWith("#")) value.remove(0, 1);
  if (value.length() != 6) return fallback;
  char *endPtr = nullptr;
  uint32_t color = strtoul(value.c_str(), &endPtr, 16);
  return endPtr && *endPtr == '\0' ? color & 0xFFFFFF : fallback;
}

static uint32_t scaleColor(uint32_t color, uint8_t scale) {
  uint8_t r = ((color >> 16) & 0xFF) * scale / 255;
  uint8_t g = ((color >> 8) & 0xFF) * scale / 255;
  uint8_t b = (color & 0xFF) * scale / 255;
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static uint32_t colorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) return ((uint32_t)(255 - pos * 3) << 16) | (pos * 3);
  if (pos < 170) {
    pos -= 85;
    return ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  }
  pos -= 170;
  return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8);
}

static bool validPressEffect(const String &value) {
  for (size_t i = 0; i < sizeof(EFFECTS) / sizeof(EFFECTS[0]); i++) {
    if (value == EFFECTS[i].value) return true;
  }
  return false;
}

static bool validLightingEffect(const String &value) {
  for (size_t i = 0; i < sizeof(LIGHTING_EFFECTS) / sizeof(LIGHTING_EFFECTS[0]); i++) {
    if (value == LIGHTING_EFFECTS[i].value) return true;
  }
  return false;
}

static bool validAction(const String &value) {
  if (value.startsWith("RUN:") && value.length() > 4 && value.length() <= 180) return true;
  if (value == "PROFILE:NEXT" || value == "PROFILE:PREV") return true;
  if (value.startsWith("PROFILE:")) {
    int idx = value.substring(8).toInt();
    return idx >= 0 && idx < profileCount;
  }
  for (size_t i = 0; i < sizeof(ACTIONS) / sizeof(ACTIONS[0]); i++) {
    if (value == ACTIONS[i].value) return true;
  }
  for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
    if (customCombos[i].length() && value == customCombos[i]) return true;
  }
  return false;
}

static bool validComboPart(const String &value) {
  for (size_t i = 0; i < sizeof(COMBO_PARTS) / sizeof(COMBO_PARTS[0]); i++) {
    if (value == COMBO_PARTS[i].value) return true;
  }
  return false;
}

static String comboLabel(String action) {
  if (action.startsWith("RUN:")) return "Run: " + action.substring(4);
  if (action.startsWith("KEY:")) action = action.substring(4);
  action.replace("+", " + ");
  return action;
}

static String defaultActionFor(uint8_t i) {
  return i < 9 ? String("KEY:") + String(i + 1) : String("KEY:0");
}

static String profilePrefix(uint8_t index = 255) {
  if (index == 255) index = activeProfile;
  return "p" + String(index) + "_";
}

static String profileName(uint8_t index) {
  if (index >= profileCount || !profileNames[index].length()) return "Profile " + String(index + 1);
  return profileNames[index];
}

static void saveProfileMeta() {
  prefs.putUChar("profileCount", profileCount);
  prefs.putUChar("activeProfile", activeProfile);
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    prefs.putString(("profileName" + String(i)).c_str(), profileNames[i]);
  }
}

static void loadProfileMeta() {
  profileCount = prefs.getUChar("profileCount", 1);
  if (profileCount < 1) profileCount = 1;
  if (profileCount > MAX_PROFILES) profileCount = MAX_PROFILES;
  activeProfile = prefs.getUChar("activeProfile", 0);
  if (activeProfile >= profileCount) activeProfile = 0;
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    profileNames[i] = prefs.getString(("profileName" + String(i)).c_str(), "Profile " + String(i + 1));
    profileNames[i].trim();
    if (!profileNames[i].length()) profileNames[i] = "Profile " + String(i + 1);
  }
}

static void loadCustomCombos() {
  for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
    String key = "combo" + String(i);
    customCombos[i] = prefs.getString(key.c_str(), "");
    customCombos[i].trim();
    if (customCombos[i].startsWith("KEY:")) {
      customCombos[i].toUpperCase();
    }
    if (customCombos[i].length() && !customCombos[i].startsWith("KEY:") && !customCombos[i].startsWith("RUN:")) customCombos[i] = "";
  }
}

static void drawCenteredLine(const String &text, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int16_t x = (OLED_WIDTH - (int16_t)w) / 2;
  if (x < 0) x = 0;
  display.setCursor(x, y);
  display.print(text);
}

static void drawCenteredLineSize(const String &text, int16_t y, uint8_t size) {
  if (!text.length()) return;
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(size);
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  if (w > OLED_WIDTH && size > 1) {
    size = 1;
    display.setTextSize(size);
    display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  }
  int16_t x = (OLED_WIDTH - (int16_t)w) / 2;
  if (x < 0) x = 0;
  display.setCursor(x, y);
  display.print(text);
}

static void drawCenteredLineFit(const String &text, int16_t y, uint8_t maxSize) {
  if (!text.length()) return;
  int16_t x1, y1;
  uint16_t w, h;
  uint8_t size = maxSize;
  while (size > 1) {
    display.setTextSize(size);
    display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    if (w <= OLED_WIDTH) break;
    size--;
  }
  display.setTextSize(size);
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int16_t x = (OLED_WIDTH - (int16_t)w) / 2;
  if (x < 0) x = 0;
  display.setCursor(x, y);
  display.print(text);
}

static uint8_t clockFontMaxTextSize() {
  if (clockFont == "DEFAULT") return 4;
  if (clockFont == "ORG" || clockFont == "PICO" || clockFont == "TINY3") return 7;
  if (clockFont == "TOM") return 5;
  return 2;
}

static void drawClockTimeLine(const String &text, int16_t top, uint8_t height) {
  if (!text.length()) return;
  display.setFont(clockFontPtr(clockFont));
  display.setTextWrap(false);
  int16_t x1, y1;
  uint16_t w, h;
  uint8_t maxSize = clockFontMaxTextSize();
  uint8_t size = maxSize;
  while (size > 1) {
    display.setTextSize(size);
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    if (w <= OLED_WIDTH && h <= height) break;
    size--;
  }
  display.setTextSize(size);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  if ((w > OLED_WIDTH || h > height) && clockFont != "DEFAULT") {
    display.setFont(NULL);
    maxSize = 4;
    size = maxSize;
    while (size > 1) {
      display.setTextSize(size);
      display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
      if (w <= OLED_WIDTH && h <= height) break;
      size--;
    }
    display.setTextSize(size);
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  }
  int16_t x = (OLED_WIDTH - (int16_t)w) / 2 - x1;
  int16_t y = top + ((int16_t)height - (int16_t)h) / 2 - y1;
  if (x < -x1) x = -x1;
  display.setCursor(x, y);
  display.print(text);
  display.setFont(NULL);
  display.setTextSize(1);
  display.setTextWrap(true);
}

static void drawReadableIpAddress(const String &text, int16_t y, uint8_t scale = 1) {
  if (!text.length()) return;
  if (scale > 1) {
    const uint8_t srcCharWidth = 12;
    const uint8_t srcDotExtraGap = 4;
    const uint8_t srcHeight = 16;
    uint16_t srcWidth = 0;
    for (uint16_t i = 0; i < text.length(); i++) {
      srcWidth += srcCharWidth;
      if (text[i] == '.') srcWidth += srcDotExtraGap;
    }
    if (srcWidth < 1) return;

    GFXcanvas1 canvas(srcWidth, srcHeight);
    canvas.setFont(NULL);
    canvas.setTextColor(1);
    canvas.setTextSize(2);
    canvas.setTextWrap(false);
    int16_t cx = 0;
    for (uint16_t i = 0; i < text.length(); i++) {
      canvas.setCursor(cx, 0);
      canvas.print(text[i]);
      cx += srcCharWidth;
      if (text[i] == '.') cx += srcDotExtraGap;
    }

    uint16_t targetWidth = ((uint32_t)srcWidth * 3) / 4;
    if (targetWidth > OLED_WIDTH - 2) targetWidth = OLED_WIDTH - 2;
    if (targetWidth < 1) targetWidth = 1;
    int16_t x = (OLED_WIDTH - (int16_t)targetWidth) / 2;
    if (x < 0) x = 0;
    display.setTextWrap(false);
    for (uint16_t dx = 0; dx < targetWidth; dx++) {
      uint16_t sx0 = ((uint32_t)dx * srcWidth) / targetWidth;
      uint16_t sx1 = ((uint32_t)(dx + 1) * srcWidth) / targetWidth;
      if (sx1 <= sx0) sx1 = sx0 + 1;
      for (uint8_t sy = 0; sy < srcHeight; sy++) {
        bool lit = false;
        for (uint16_t sx = sx0; sx < sx1 && sx < srcWidth; sx++) {
          if (canvas.getPixel(sx, sy)) {
            lit = true;
            break;
          }
        }
        if (lit) display.drawPixel(x + dx, y + sy, SSD1306_WHITE);
      }
    }
    return;
  }

  const uint8_t size = 1;
  const uint8_t charWidth = 6;
  const uint8_t dotExtraGap = 2;
  int16_t w = 0;
  for (uint16_t i = 0; i < text.length(); i++) {
    w += charWidth;
    if (text[i] == '.') w += dotExtraGap;
  }
  display.setTextSize(size);
  display.setTextWrap(false);
  int16_t x = (OLED_WIDTH - w) / 2;
  if (x < 0) x = 0;
  for (uint16_t i = 0; i < text.length(); i++) {
    display.setCursor(x, y);
    display.print(text[i]);
    x += charWidth;
    if (text[i] == '.') x += dotExtraGap;
  }
}

static void applyOledDisplaySettings(bool forceOn = false) {
  if (!oledOk) return;
  display.ssd1306_command((screenEnabled || forceOn) ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(oledBrightness);
}

static void stopEyes() {
  eyesActive = false;
}

static void configureEyes() {
  roboEyes.setDisplayColors(0, 1);
  roboEyes.setWidth(36, 36);
  roboEyes.setHeight(34, 34);
  roboEyes.setBorderradius(8, 8);
  roboEyes.setSpacebetween(10);
  roboEyes.setCyclops(false);
  roboEyes.setCuriosity(true);
  roboEyes.setSweat(false);
  roboEyes.setHFlicker(false, 0);
  roboEyes.setVFlicker(false, 0);
  roboEyes.setMood(DEFAULT);
  roboEyes.setPosition(DEFAULT);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);

  if (eyePersonality == "CALM") {
    roboEyes.setWidth(34, 34);
    roboEyes.setHeight(30, 30);
    roboEyes.setCuriosity(false);
    roboEyes.setAutoblinker(ON, 4, 3);
    roboEyes.setIdleMode(ON, 4, 2);
  } else if (eyePersonality == "DRAMATIC") {
    roboEyes.setWidth(40, 40);
    roboEyes.setHeight(38, 38);
    roboEyes.setMood(ANGRY);
    roboEyes.setSweat(true);
    roboEyes.setAutoblinker(ON, 2, 2);
    roboEyes.setIdleMode(ON, 1, 2);
  } else if (eyePersonality == "SLEEPY") {
    roboEyes.setWidth(38, 38);
    roboEyes.setHeight(26, 26);
    roboEyes.setMood(TIRED);
    roboEyes.setPosition(S);
    roboEyes.setAutoblinker(ON, 2, 1);
    roboEyes.setIdleMode(ON, 5, 2);
  } else if (eyePersonality == "NERVOUS") {
    roboEyes.setWidth(34, 34);
    roboEyes.setHeight(34, 34);
    roboEyes.setMood(TIRED);
    roboEyes.setSweat(true);
    roboEyes.setHFlicker(true, 1);
    roboEyes.setAutoblinker(ON, 1, 2);
    roboEyes.setIdleMode(ON, 1, 1);
  } else if (eyePersonality == "CYCLOPS") {
    roboEyes.setWidth(54, 54);
    roboEyes.setHeight(38, 38);
    roboEyes.setCyclops(true);
    roboEyes.setBorderradius(12, 12);
    roboEyes.setSpacebetween(0);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(ON, 2, 2);
  } else if (eyePersonality == "HAPPY") {
    roboEyes.setWidth(38, 38);
    roboEyes.setHeight(32, 32);
    roboEyes.setMood(HAPPY);
    roboEyes.setCuriosity(true);
    roboEyes.setAutoblinker(ON, 2, 3);
    roboEyes.setIdleMode(ON, 2, 3);
  } else if (eyePersonality == "ANGRY") {
    roboEyes.setWidth(42, 42);
    roboEyes.setHeight(34, 34);
    roboEyes.setBorderradius(6, 6);
    roboEyes.setMood(ANGRY);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(ON, 2, 1);
  } else if (eyePersonality == "LOOKOUT") {
    roboEyes.setWidth(32, 32);
    roboEyes.setHeight(36, 36);
    roboEyes.setCuriosity(true);
    roboEyes.setAutoblinker(ON, 4, 2);
    roboEyes.setIdleMode(ON, 1, 1);
  } else if (eyePersonality == "WATCHER") {
    roboEyes.setWidth(36, 36);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(5, 5);
    roboEyes.setCuriosity(true);
    roboEyes.setAutoblinker(ON, 5, 2);
    roboEyes.setIdleMode(ON, 1, 2);
  } else if (eyePersonality == "CURIOUS") {
    roboEyes.setWidth(34, 34);
    roboEyes.setHeight(38, 38);
    roboEyes.setMood(HAPPY);
    roboEyes.setCuriosity(true);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(ON, 1, 3);
  } else if (eyePersonality == "GLITCHY") {
    roboEyes.setWidth(35, 37);
    roboEyes.setHeight(33, 35);
    roboEyes.setCuriosity(false);
    roboEyes.setHFlicker(true, 1);
    roboEyes.setVFlicker(true, 1);
    roboEyes.setAutoblinker(ON, 2, 1);
    roboEyes.setIdleMode(ON, 1, 1);
  } else if (eyePersonality == "JUDGY") {
    roboEyes.setWidth(40, 40);
    roboEyes.setHeight(24, 24);
    roboEyes.setMood(ANGRY);
    roboEyes.setPosition(E);
    roboEyes.setAutoblinker(ON, 4, 2);
    roboEyes.setIdleMode(ON, 3, 1);
  } else if (eyePersonality == "CHILL") {
    roboEyes.setWidth(38, 38);
    roboEyes.setHeight(28, 28);
    roboEyes.setMood(TIRED);
    roboEyes.setCuriosity(false);
    roboEyes.setAutoblinker(ON, 5, 3);
    roboEyes.setIdleMode(ON, 5, 2);
  } else if (eyePersonality == "CYCLOPS_SCAN") {
    roboEyes.setWidth(58, 58);
    roboEyes.setHeight(34, 34);
    roboEyes.setCyclops(true);
    roboEyes.setBorderradius(10, 10);
    roboEyes.setSpacebetween(0);
    roboEyes.setAutoblinker(ON, 4, 2);
    roboEyes.setIdleMode(ON, 1, 2);
  } else if (eyePersonality == "DREAMY") {
    roboEyes.setWidth(36, 36);
    roboEyes.setHeight(26, 26);
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(N);
    roboEyes.setAutoblinker(ON, 5, 3);
    roboEyes.setIdleMode(ON, 4, 3);
  } else if (eyePersonality == "ALERT") {
    roboEyes.setWidth(42, 42);
    roboEyes.setHeight(38, 38);
    roboEyes.setBorderradius(6, 6);
    roboEyes.setMood(DEFAULT);
    roboEyes.setAutoblinker(ON, 1, 1);
    roboEyes.setIdleMode(ON, 1, 1);
  }

  roboEyes.open();
}

static bool validKeyEyeAnim(const String &anim) {
  for (size_t i = 0; i < sizeof(KEY_EYE_ANIMS) / sizeof(KEY_EYE_ANIMS[0]); i++) {
    if (anim == KEY_EYE_ANIMS[i].value) return true;
  }
  return false;
}

static bool validEyePersonality(const String &personality) {
  for (size_t i = 0; i < sizeof(EYE_PERSONALITIES) / sizeof(EYE_PERSONALITIES[0]); i++) {
    if (personality == EYE_PERSONALITIES[i].value) return true;
  }
  return false;
}

static bool validEyePosition(const String &pos) {
  for (size_t i = 0; i < sizeof(EYE_POSITIONS) / sizeof(EYE_POSITIONS[0]); i++) {
    if (pos == EYE_POSITIONS[i].value) return true;
  }
  return false;
}

static bool validTimeZone(const String &tz) {
  for (size_t i = 0; i < sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]); i++) {
    if (tz == TIME_ZONES[i].value) return true;
  }
  return false;
}

static const char *timeZoneLabel(const String &tz) {
  for (size_t i = 0; i < sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]); i++) {
    if (tz == TIME_ZONES[i].value) return TIME_ZONES[i].label;
  }
  return "UTC";
}

static String timeZoneRule(const String &tz) {
  int marker = tz.indexOf('|');
  if (marker >= 0) return tz.substring(0, marker);
  return tz;
}

static bool validDateFormat(const String &fmt) {
  for (size_t i = 0; i < sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]); i++) {
    if (fmt == DATE_FORMATS[i].value) return true;
  }
  return false;
}

static bool validClockFont(const String &font) {
  for (size_t i = 0; i < sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]); i++) {
    if (font == CLOCK_FONTS[i].value) return true;
  }
  return false;
}

static const char *clockFontLabel(const String &font) {
  for (size_t i = 0; i < sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]); i++) {
    if (font == CLOCK_FONTS[i].value) return CLOCK_FONTS[i].label;
  }
  return "Default";
}

static const char *dateFormatLabel(const String &fmt) {
  for (size_t i = 0; i < sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]); i++) {
    if (fmt == DATE_FORMATS[i].value) return DATE_FORMATS[i].label;
  }
  return "Mon 07 May";
}

static uint8_t timeZoneIndex() {
  for (uint8_t i = 0; i < sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]); i++) {
    if (clockTimeZone == TIME_ZONES[i].value) return i;
  }
  return 0;
}

static uint8_t dateFormatIndex() {
  for (uint8_t i = 0; i < sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]); i++) {
    if (clockDateFormat == DATE_FORMATS[i].value) return i;
  }
  return 0;
}

static const GFXfont *clockFontPtr(const String &font) {
  for (size_t i = 0; i < sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]); i++) {
    if (font == CLOCK_FONTS[i].value) return CLOCK_FONTS[i].font;
  }
  return NULL;
}

static void configureClockTime() {
  if (!validTimeZone(clockTimeZone)) clockTimeZone = "UTC0";
  String tzRule = timeZoneRule(clockTimeZone);
  configTzTime(tzRule.c_str(), "pool.ntp.org", "time.nist.gov");
  Serial.printf("Clock timezone=%s format=%s\n", timeZoneLabel(clockTimeZone), clockUse24Hour ? "24h" : "12h");
}

static String formattedClockTime(const struct tm &timeinfo) {
  char mainText[10];
  char ampmText[4] = "";
  if (clockUse24Hour) {
    strftime(mainText, sizeof(mainText), "%H:%M", &timeinfo);
  } else {
    strftime(mainText, sizeof(mainText), "%I:%M", &timeinfo);
    if (mainText[0] == '0') memmove(mainText, mainText + 1, strlen(mainText));
    if (clockShowAmPm) strftime(ampmText, sizeof(ampmText), "%p", &timeinfo);
  }
  if (!clockUse24Hour && clockShowAmPm && strlen(ampmText)) {
    return clockAmPmAfter ? String(mainText) + " " + String(ampmText) : String(ampmText) + " " + String(mainText);
  }
  return String(mainText);
}

static String formattedClockDate(const struct tm &timeinfo) {
  char dateText[24];
  const char *fmt = "%a %d %b";
  if (clockDateFormat == "D_MON_Y") fmt = "%d %b %Y";
  else if (clockDateFormat == "YMD") fmt = "%Y-%m-%d";
  else if (clockDateFormat == "DMY") fmt = "%d/%m/%Y";
  else if (clockDateFormat == "MDY") fmt = "%m/%d/%Y";
  strftime(dateText, sizeof(dateText), fmt, &timeinfo);
  return String(dateText);
}

static uint8_t eyePositionCode(const String &pos) {
  if (pos == "N") return N;
  if (pos == "NE") return NE;
  if (pos == "E") return E;
  if (pos == "SE") return SE;
  if (pos == "S") return S;
  if (pos == "SW") return SW;
  if (pos == "W") return W;
  if (pos == "NW") return NW;
  return DEFAULT;
}

static uint8_t eyePositionAt(uint8_t i) {
  static const uint8_t positions[] = {N, NE, E, SE, S, SW, W, NW, DEFAULT};
  return positions[i % (sizeof(positions) / sizeof(positions[0]))];
}

static void updateKeyEyeAnimation() {
  if (activeKeyEyeAnim.length() == 0 || millis() < nextKeyEyeStepAt) return;

  String anim = activeKeyEyeAnim;
  uint8_t step = activeKeyEyeStep++;
  uint16_t interval = 220;

  if (anim == "DOUBLE_BLINK") {
    if (step == 0 || step == 2) roboEyes.blink();
    if (step >= 3) activeKeyEyeAnim = "";
    interval = 180;
  } else if (anim == "SIDE_EYE") {
    static const uint8_t sequence[] = {W, W, DEFAULT, E, E, DEFAULT};
    roboEyes.setMood(TIRED);
    roboEyes.setPosition(sequence[step % (sizeof(sequence) / sizeof(sequence[0]))]);
    if (step == 1 || step == 4) roboEyes.blink();
    interval = 240;
  } else if (anim == "SCAN") {
    static const uint8_t sequence[] = {W, DEFAULT, E, DEFAULT, NW, N, NE, DEFAULT};
    roboEyes.setPosition(sequence[step % (sizeof(sequence) / sizeof(sequence[0]))]);
    if (step % 8 == 7) roboEyes.blink();
    interval = 180;
  } else if (anim == "DIZZY") {
    static const uint8_t sequence[] = {NW, N, NE, E, SE, S, SW, W};
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(sequence[step % (sizeof(sequence) / sizeof(sequence[0]))]);
    interval = 140;
  } else if (anim == "PANIC") {
    roboEyes.setMood(TIRED);
    roboEyes.setSweat(true);
    roboEyes.setHFlicker(true, 2);
    roboEyes.setVFlicker(true, 1);
    roboEyes.setPosition(eyePositionAt(random(8)));
    if (step % 3 == 0) roboEyes.blink(random(2), random(2));
    interval = 130;
  } else if (anim == "SUSPICIOUS") {
    static const uint8_t sequence[] = {W, W, W, DEFAULT};
    roboEyes.setMood(TIRED);
    roboEyes.setPosition(sequence[step % (sizeof(sequence) / sizeof(sequence[0]))]);
    if (step % 4 == 2) roboEyes.blink();
    interval = 300;
  } else if (anim == "EXCITED") {
    roboEyes.setMood(HAPPY);
    if (step % 4 == 0) roboEyes.anim_laugh();
    else if (step % 2 == 0) roboEyes.blink();
    else roboEyes.setPosition(eyePositionAt(random(8)));
    interval = 180;
  } else if (anim == "GLITCH") {
    roboEyes.setHFlicker(true, 2);
    roboEyes.setVFlicker(true, 2);
    roboEyes.setMood((step % 2) ? ANGRY : DEFAULT);
    roboEyes.setPosition(eyePositionAt(random(8)));
    interval = 110;
  } else if (anim == "SLEEP_BLINK") {
    roboEyes.setMood(TIRED);
    if (step == 0) roboEyes.close();
    else if (step == 2) roboEyes.open();
    else if (step == 4) roboEyes.blink();
    if (step >= 5) activeKeyEyeAnim = "";
    interval = 300;
  } else if (anim == "JUDGEMENT") {
    roboEyes.setMood(ANGRY);
    roboEyes.setHeight(24, 24);
    roboEyes.setPosition((step % 2) ? W : E);
    if (step % 4 == 3) roboEyes.blink();
    interval = 320;
  } else if (anim == "STARTLED") {
    roboEyes.setMood(DEFAULT);
    roboEyes.setWidth(44, 44);
    roboEyes.setHeight(40, 40);
    roboEyes.setPosition(DEFAULT);
    if (step == 1) roboEyes.blink();
    if (step >= 3) activeKeyEyeAnim = "";
    interval = 160;
  } else if (anim == "CURIOUS_PEEK") {
    static const uint8_t sequence[] = {SW, W, NW, W, DEFAULT};
    roboEyes.setMood(HAPPY);
    roboEyes.setCuriosity(true);
    roboEyes.setPosition(sequence[step % (sizeof(sequence) / sizeof(sequence[0]))]);
    interval = 260;
  } else if (anim == "GRUMPY_LOOK") {
    roboEyes.setMood(ANGRY);
    roboEyes.setPosition((step % 3 == 0) ? E : DEFAULT);
    if (step % 5 == 4) roboEyes.blink();
    interval = 260;
  } else {
    activeKeyEyeAnim = "";
  }

  nextKeyEyeStepAt = millis() + interval;
}

static void startKeyEyes(uint8_t i) {
  if (!oledOk) return;

  configureEyes();
  roboEyes.setIdleMode(OFF, 0, 0);
  activeKeyEyeAnim = "";
  activeKeyEyeStep = 0;
  nextKeyEyeStepAt = 0;
  roboEyes.setPosition(eyePositionCode(settings[i].eyePos));
  String anim = settings[i].eyeAnim;
  anim.trim();

  if (anim == "DOUBLE_BLINK" || anim == "SIDE_EYE" || anim == "SCAN" || anim == "DIZZY" || anim == "PANIC" || anim == "SUSPICIOUS" || anim == "EXCITED" || anim == "GLITCH" || anim == "SLEEP_BLINK" || anim == "JUDGEMENT" || anim == "STARTLED" || anim == "CURIOUS_PEEK" || anim == "GRUMPY_LOOK") {
    activeKeyEyeAnim = anim;
    updateKeyEyeAnimation();
  } else if (anim == "WINK_LEFT") {
    roboEyes.blink(true, false);
  } else if (anim == "WINK_RIGHT") {
    roboEyes.blink(false, true);
  } else if (anim == "HAPPY") {
    roboEyes.setMood(HAPPY);
    roboEyes.blink();
  } else if (anim == "ANGRY") {
    roboEyes.setMood(ANGRY);
    roboEyes.blink();
  } else if (anim == "TIRED") {
    roboEyes.setMood(TIRED);
    roboEyes.blink();
  } else if (anim == "LAUGH") {
    roboEyes.setMood(HAPPY);
    roboEyes.anim_laugh();
  } else if (anim == "SHAKE") {
    roboEyes.setHFlicker(true, 2);
    roboEyes.anim_confused();
  } else if (anim == "CONFUSED") {
    roboEyes.anim_confused();
  } else if (anim == "FLICKER") {
    roboEyes.setHFlicker(true, 1);
    roboEyes.setVFlicker(true, 1);
  } else if (anim == "CYCLOPS_BLINK") {
    roboEyes.setCyclops(true);
    roboEyes.setSpacebetween(0);
    roboEyes.setWidth(56, 56);
    roboEyes.blink();
  } else if (anim == "SWEAT_DROP") {
    roboEyes.setSweat(true);
    roboEyes.setMood(TIRED);
    roboEyes.blink();
  } else {
    roboEyes.blink();
  }

  eyesActive = true;
  statusLine = "Key " + String(i + 1) + " eyes " + anim;
  oledReturnAt = millis() + oledReturnMs;
}

static void triggerEyeAction() {
  static const uint8_t positions[] = {N, NE, E, SE, S, SW, W, NW, DEFAULT};
  const uint8_t positionCount = sizeof(positions) / sizeof(positions[0]);
  uint8_t roll = random(100);

  if (eyePersonality == "CALM") {
    if (roll < 35) roboEyes.blink();
    else if (roll < 70) roboEyes.setPosition(positions[random(positionCount)]);
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "SLEEPY") {
    if (roll < 30) roboEyes.blink();
    else if (roll < 55) roboEyes.setMood(TIRED);
    else if (roll < 75) roboEyes.close();
    else roboEyes.open();
  } else if (eyePersonality == "NERVOUS") {
    if (roll < 35) roboEyes.blink(random(2), random(2));
    else if (roll < 70) roboEyes.setPosition(positions[random(8)]);
    else if (roll < 85) roboEyes.setSweat(true);
    else roboEyes.setMood(TIRED);
  } else if (eyePersonality == "DRAMATIC") {
    if (roll < 35) roboEyes.anim_laugh();
    else if (roll < 60) roboEyes.setMood(ANGRY);
    else if (roll < 75) roboEyes.setMood(HAPPY);
    else if (roll < 90) roboEyes.blink();
    else roboEyes.setPosition(positions[random(positionCount)]);
  } else if (eyePersonality == "HAPPY") {
    if (roll < 25) roboEyes.anim_laugh();
    else if (roll < 45) roboEyes.blink();
    else if (roll < 70) roboEyes.setMood(HAPPY);
    else roboEyes.setPosition(positions[random(positionCount)]);
  } else if (eyePersonality == "ANGRY") {
    if (roll < 30) roboEyes.setMood(ANGRY);
    else if (roll < 55) roboEyes.blink();
    else if (roll < 75) roboEyes.setMood(DEFAULT);
    else roboEyes.setPosition(positions[random(positionCount)]);
  } else if (eyePersonality == "LOOKOUT") {
    if (roll < 45) roboEyes.setPosition(positions[random(positionCount)]);
    else if (roll < 65) roboEyes.blink();
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "WATCHER") {
    if (roll < 55) roboEyes.setPosition(positions[random(positionCount)]);
    else if (roll < 75) roboEyes.blink();
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "CURIOUS") {
    if (roll < 35) roboEyes.setPosition(positions[random(positionCount)]);
    else if (roll < 55) roboEyes.setMood(HAPPY);
    else if (roll < 75) roboEyes.blink(random(2), random(2));
    else roboEyes.anim_laugh();
  } else if (eyePersonality == "GLITCHY") {
    if (roll < 40) roboEyes.setPosition(positions[random(8)]);
    else if (roll < 65) roboEyes.blink(random(2), random(2));
    else if (roll < 82) roboEyes.setMood(ANGRY);
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "JUDGY") {
    if (roll < 40) roboEyes.setMood(ANGRY);
    else if (roll < 65) roboEyes.setPosition((roll % 2) ? W : E);
    else if (roll < 85) roboEyes.blink();
    else roboEyes.setMood(TIRED);
  } else if (eyePersonality == "CHILL") {
    if (roll < 35) roboEyes.blink();
    else if (roll < 60) roboEyes.setMood(TIRED);
    else if (roll < 80) roboEyes.setPosition(S);
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "CYCLOPS_SCAN") {
    if (roll < 60) roboEyes.setPosition(positions[random(8)]);
    else if (roll < 82) roboEyes.blink();
    else roboEyes.setMood(DEFAULT);
  } else if (eyePersonality == "DREAMY") {
    if (roll < 35) roboEyes.setPosition(positions[random(positionCount)]);
    else if (roll < 65) roboEyes.setMood(HAPPY);
    else if (roll < 82) roboEyes.close();
    else roboEyes.open();
  } else if (eyePersonality == "ALERT") {
    if (roll < 30) roboEyes.blink(random(2), random(2));
    else if (roll < 70) roboEyes.setPosition(positions[random(8)]);
    else if (roll < 86) roboEyes.setMood(ANGRY);
    else roboEyes.setMood(DEFAULT);
  } else {
    if (roll < 14) roboEyes.blink();
    else if (roll < 24) roboEyes.blink(true, false);
    else if (roll < 34) roboEyes.blink(false, true);
    else if (roll < 46) roboEyes.anim_laugh();
    else if (roll < 70) roboEyes.setMood(HAPPY);
    else if (roll < 78) roboEyes.setMood(TIRED);
    else if (roll < 86) roboEyes.setMood(ANGRY);
    else if (roll < 94) roboEyes.setMood(DEFAULT);
    else roboEyes.setPosition(positions[random(positionCount)]);
  }

  nextEyeActionAt = millis() + random(900, 2600);
}

static void startEyes() {
  if (!oledOk || !screenEnabled || !eyesEnabled) return;
  activeIdleScreen = 2;
  activeKeyEyeAnim = "";
  activeKeyEyeStep = 0;
  nextKeyEyeStepAt = 0;
  configureEyes();
  eyesActive = true;
  statusLine = "RoboEyes " + eyePersonality;
  nextEyeActionAt = millis() + 700;
  displayModeUntil = millis() + eyeScreenMs;
}

static void oled(const String &a, const String &b = "", const String &c = "", const String &d = "") {
  statusLine = a;
  stopEyes();
  if (!oledOk || !screenEnabled) return;
  applyOledDisplaySettings();
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);
  uint8_t lines = 1 + (b.length() ? 1 : 0) + (c.length() ? 1 : 0) + (d.length() ? 1 : 0);
  int16_t lineHeight = 10;
  int16_t y = max(0, (int)(OLED_HEIGHT - lines * lineHeight) / 2);
  drawCenteredLineFit(a, y, 1);
  if (b.length()) {
    y += lineHeight;
    drawCenteredLineFit(b, y, 1);
  }
  if (c.length()) {
    y += lineHeight;
    drawCenteredLineFit(c, y, 1);
  }
  if (d.length()) {
    y += lineHeight;
    drawCenteredLineFit(d, y, 1);
  }
  display.display();
  display.setFont(NULL);
  display.setTextSize(1);
}

static void oledAddressScreen(const String &title, const String &primary, const String &secondaryTitle, const String &secondary, uint8_t primaryIpScale = 1) {
  statusLine = title + " " + primary;
  stopEyes();
  if (!oledOk || !screenEnabled) return;
  applyOledDisplaySettings();
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);

  drawCenteredLineFit(title, 2, 1);
  drawReadableIpAddress(primary, 18, primaryIpScale);
  drawCenteredLineFit(secondaryTitle, 42, 1);
  drawCenteredLineFit(secondary, 54, 1);

  display.display();
  display.setFont(NULL);
  display.setTextSize(1);
}

static String wifiText() {
  if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
  return WiFi.softAPIP().toString();
}

static void startSetupHotspot() {
  if (setupHotspotActive) return;
  WiFi.softAP(AP_SSID);
  setupHotspotActive = true;
}

static void showAddressScreen(bool resetTimer = true) {
  activeIdleScreen = 0;
  if (WiFi.status() == WL_CONNECTED) {
    oledAddressScreen("LAN address", WiFi.localIP().toString(), String(AP_SSID), WiFi.softAPIP().toString(), 2);
  } else {
    oledAddressScreen(String(AP_SSID), WiFi.softAPIP().toString(), "Join hotspot to setup", "");
  }
  nextAddressRefresh = millis() + 5000;
  if (resetTimer) displayModeUntil = millis() + addressScreenMs;
}

static void showClockScreen() {
  statusLine = "Clock";
  stopEyes();
  activeIdleScreen = 1;
  if (!oledOk || !screenEnabled) return;
  applyOledDisplaySettings();
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);

  time_t now = time(nullptr);
  struct tm timeinfo;
  if (now < 1700000000 || !localtime_r(&now, &timeinfo)) {
    drawCenteredLineFit("Clock syncing", 20, 1);
    drawCenteredLineFit(WiFi.status() == WL_CONNECTED ? "Waiting for NTP" : "Connect WiFi", 36, 1);
  } else {
    uint8_t infoLines = (clockShowDate ? 1 : 0) + (clockShowTimeZone ? 1 : 0);
    drawClockTimeLine(formattedClockTime(timeinfo), 0, infoLines ? 44 : 64);
    int16_t lineY = infoLines == 2 ? 46 : 52;
    if (clockShowDate) {
      drawCenteredLineFit(formattedClockDate(timeinfo), lineY, 1);
      lineY += 11;
    }
    if (clockShowTimeZone) drawCenteredLineFit(timeZoneLabel(clockTimeZone), lineY, 1);
  }
  display.display();
  display.setFont(NULL);
  display.setTextSize(1);
  displayModeUntil = millis() + clockScreenMs;
}

static bool idleScreenEnabled(uint8_t mode) {
  if (mode == 0) return addressScreenEnabled;
  if (mode == 1) return clockScreenEnabled;
  return eyeScreenEnabled && eyesEnabled;
}

static void showNextIdleScreen() {
  if (!screenEnabled) {
    stopEyes();
    applyOledDisplaySettings();
    return;
  }

  uint8_t start = activeIdleScreen < 3 ? (activeIdleScreen + 1) % 3 : idleScreenIndex % 3;
  for (uint8_t attempt = 0; attempt < 3; attempt++) {
    uint8_t mode = (start + attempt) % 3;
    if (!idleScreenEnabled(mode)) continue;
    idleScreenIndex = (mode + 1) % 3;
    if (mode == 0) {
      showAddressScreen();
    } else if (mode == 1) {
      showClockScreen();
    } else {
      startEyes();
    }
    return;
  }

  stopEyes();
  oled("Idle screen", "All modes off");
  displayModeUntil = millis() + 3000;
}

static void showIdleDisplay() {
  showNextIdleScreen();
}

static void maintainWifiAccess() {
  if (millis() < nextWifiCheckAt) return;
  nextWifiCheckAt = millis() + 1000;

  bool lanConnected = WiFi.status() == WL_CONNECTED;
  if (lanConnected != lastLanConnected) {
    lastLanConnected = lanConnected;
    if (!oledMenuOpen && !oledReturnAt && activeIdleScreen == 0) showAddressScreen(false);
  }
}

static void updateIdleDisplay() {
  if (oledMenuOpen) return;
  if (oledReturnAt) return;
  if (!screenEnabled) return;

  if (eyesActive) {
    if (millis() >= nextEyeActionAt) triggerEyeAction();
    roboEyes.update();
    if ((!eyesEnabled || !eyeScreenEnabled || millis() >= displayModeUntil) && millis() >= displayModeUntil) showNextIdleScreen();
    return;
  }

  if (millis() >= displayModeUntil) {
    showNextIdleScreen();
  } else if (activeIdleScreen == 0 && addressScreenEnabled && millis() >= nextAddressRefresh) {
    showAddressScreen(false);
  }
}

static bool startOled() {
  const uint8_t addresses[] = {0x3C, 0x3D};
  for (uint8_t i = 0; i < 2; i++) {
    if (display.begin(SSD1306_SWITCHCAPVCC, addresses[i])) {
      oledAddr = addresses[i];
      display.clearDisplay();
      display.display();
      Serial.print("OLED OK at 0x");
      Serial.println(oledAddr, HEX);
      return true;
    }
  }
  Serial.println("OLED not found at 0x3C or 0x3D");
  return false;
}

static bool rawDown(uint8_t i) {
  return digitalRead(KEY_PINS[i]) == LOW;
}

static void setAllLedsColor(uint32_t color) {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
    pixels.setPixelColor(led, pixels.Color(r, g, b));
  }
}

static void setAllLedsRgb(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
    pixels.setPixelColor(led, pixels.Color(r, g, b));
  }
}

static void setPixelFromColor(uint8_t led, uint32_t color) {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  pixels.setPixelColor(led, pixels.Color(r, g, b));
}

static void setPixelWrapped(int16_t led, uint32_t color) {
  while (led < 0) led += STRIP_LED_COUNT;
  led %= STRIP_LED_COUNT;
  setPixelFromColor((uint8_t)led, color);
}

static uint16_t lightingIntervalMs() {
  uint8_t speed = lightingSpeed;
  if (speed < 1) speed = 1;
  if (speed > 10) speed = 10;
  return 220 - (speed * 18);
}

static uint16_t keyEffectDurationMs() {
  if (activePressEffect == "WHITE_PULSE") return 120;
  if (activePressEffect == "AMBIENT_PULSE") return 260;
  if (activePressEffect == "STROBE") return 360;
  if (activePressEffect == "POP" || activePressEffect == "SPARKLE_POP") return 420;
  return 760;
}

static void renderKeypressEffect() {
  String effect = activePressEffect;
  unsigned long elapsed = millis() - keyFlashStartedAt;
  uint16_t elapsed760 = elapsed > 760 ? 760 : (uint16_t)elapsed;
  uint16_t elapsed650 = elapsed > 650 ? 650 : (uint16_t)elapsed;

  if (effect == "WHITE_PULSE") {
    setAllLedsRgb(255, 255, 255);
  } else if (effect == "RAINBOW_BURST") {
    for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
      setPixelFromColor(led, colorWheel((keyFlashFrame * 14 + led * 256 / STRIP_LED_COUNT) & 0xFF));
    }
  } else if (effect == "RAINBOW_WIPE") {
    pixels.clear();
    uint8_t lit = (uint8_t)map(elapsed760, 0, 760, 1, STRIP_LED_COUNT);
    for (uint8_t led = 0; led < lit; led++) {
      setPixelFromColor(led, colorWheel((led * 256 / STRIP_LED_COUNT + keyFlashFrame * 7) & 0xFF));
    }
  } else if (effect == "TRAIL") {
    pixels.clear();
    uint8_t head = keyFlashFrame % (STRIP_LED_COUNT + 5);
    for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
      int8_t distance = (int8_t)head - (int8_t)led;
      if (distance >= 0 && distance <= 4) {
        uint8_t level = 255 - distance * 48;
        setPixelFromColor(led, scaleColor(ambientColor, level));
      }
    }
  } else if (effect == "RAINBOW_TRAIL") {
    pixels.clear();
    uint8_t head = keyFlashFrame % (STRIP_LED_COUNT + 6);
    for (uint8_t tail = 0; tail < 6; tail++) {
      int16_t led = (int16_t)head - tail;
      if (led >= 0 && led < STRIP_LED_COUNT) {
        uint8_t level = 255 - tail * 36;
        setPixelFromColor((uint8_t)led, scaleColor(colorWheel((keyFlashFrame * 12 + tail * 18) & 0xFF), level));
      }
    }
  } else if (effect == "COMET") {
    pixels.clear();
    uint8_t head = keyFlashFrame % (STRIP_LED_COUNT + 6);
    for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
      int8_t distance = (int8_t)head - (int8_t)led;
      if (distance == 0) {
        setPixelFromColor(led, 0xFFFFFF);
      } else if (distance > 0 && distance <= 5) {
        setPixelFromColor(led, scaleColor(ambientColor, 230 - distance * 38));
      }
    }
  } else if (effect == "RAINBOW_COMET") {
    pixels.clear();
    int16_t head = keyFlashFrame % STRIP_LED_COUNT;
    setPixelFromColor((uint8_t)head, 0xFFFFFF);
    for (uint8_t tail = 1; tail <= 5; tail++) {
      setPixelWrapped(head - tail, scaleColor(colorWheel((keyFlashFrame * 10 + tail * 24) & 0xFF), 240 - tail * 36));
    }
  } else if (effect == "COLOR_WIPE") {
    pixels.clear();
    uint8_t lit = (uint8_t)map(elapsed650, 0, 650, 1, STRIP_LED_COUNT);
    for (uint8_t led = 0; led < lit; led++) {
      setPixelFromColor(led, ambientColor);
    }
  } else if (effect == "SCANNER") {
    pixels.clear();
    uint8_t span = STRIP_LED_COUNT > 1 ? STRIP_LED_COUNT - 1 : 1;
    uint8_t phase = keyFlashFrame % (span * 2);
    uint8_t head = phase <= span ? phase : span * 2 - phase;
    setPixelFromColor(head, 0xFFFFFF);
    setPixelWrapped((int16_t)head - 1, scaleColor(ambientColor, 120));
    setPixelWrapped((int16_t)head + 1, scaleColor(ambientColor, 120));
  } else if (effect == "POP") {
    uint16_t fade = elapsed > 420 ? 420 : elapsed;
    uint8_t level = 255 - (fade * 210 / 420);
    setAllLedsColor(scaleColor(ambientColor, level));
  } else if (effect == "SPARKLE_POP") {
    uint16_t fade = elapsed > 420 ? 420 : elapsed;
    uint8_t base = 120 - (fade * 95 / 420);
    setAllLedsColor(scaleColor(ambientColor, base));
    setPixelFromColor(random(STRIP_LED_COUNT), 0xFFFFFF);
    setPixelFromColor(random(STRIP_LED_COUNT), colorWheel((keyFlashFrame * 31) & 0xFF));
  } else if (effect == "STROBE") {
    if (keyFlashFrame & 1) {
      pixels.clear();
    } else {
      setAllLedsRgb(255, 255, 255);
    }
  } else {
    unsigned long fade = elapsed > 130 ? elapsed - 130 : 0;
    if (fade > 180) fade = 180;
    uint8_t level = elapsed < 130 ? 80 + elapsed : 255 - fade;
    setAllLedsColor(scaleColor(ambientColor, level));
  }
  pixels.show();
}

static void renderLighting() {
  if (keyFlashUntil && millis() < keyFlashUntil) {
    renderKeypressEffect();
    return;
  }
  keyFlashUntil = 0;

  if (lightingEffect == "RAINBOW") {
    for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
      setPixelFromColor(led, colorWheel((lightingFrame * 4 + led * 256 / STRIP_LED_COUNT) & 0xFF));
    }
  } else if (lightingEffect == "CHASE") {
    uint8_t head = lightingFrame % STRIP_LED_COUNT;
    for (uint8_t led = 0; led < STRIP_LED_COUNT; led++) {
      uint8_t distance = (led + STRIP_LED_COUNT - head) % STRIP_LED_COUNT;
      uint8_t level = distance == 0 ? 255 : (distance == 1 ? 110 : (distance == 2 ? 40 : 8));
      setPixelFromColor(led, scaleColor(ambientColor, level));
    }
  } else if (lightingEffect == "SPARKLE") {
    setAllLedsColor(scaleColor(ambientColor, 40));
    setPixelFromColor(random(STRIP_LED_COUNT), 0xFFFFFF);
  } else if (lightingEffect == "BREATH") {
    uint8_t phase = lightingFrame & 0x3F;
    uint8_t level = phase < 32 ? 30 + phase * 6 : 30 + (63 - phase) * 6;
    setAllLedsColor(scaleColor(ambientColor, level));
  } else {
    setAllLedsColor(ambientColor);
  }
  pixels.show();
}

static void applyLeds() {
  pixels.setBrightness(ledBrightness);
  renderLighting();
}

static void updateLighting() {
  if (lightingEffect == "SOLID" && !keyFlashUntil) return;
  unsigned long now = millis();
  if (keyFlashUntil && now >= keyFlashUntil) {
    keyFlashUntil = 0;
    renderLighting();
    return;
  }
  uint16_t interval = lightingIntervalMs();
  if (now - lastLightingFrameAt < interval) return;
  lastLightingFrameAt = now;
  lightingFrame++;
  if (keyFlashUntil) keyFlashFrame++;
  renderLighting();
}

static void playKeyEffect(uint8_t i) {
  String effect = settings[i].ledEffect.length() ? settings[i].ledEffect : pressEffect;
  if (effect == "NONE") return;
  activePressEffect = effect;
  keyFlashStartedAt = millis();
  keyFlashUntil = keyFlashStartedAt + keyEffectDurationMs();
  keyFlashFrame = 0;
  lastLightingFrameAt = 0;
  renderLighting();
}

static void loadSettings() {
  prefs.begin("macropad2", false);
  loadProfileMeta();
  loadCustomCombos();
  oledReturnMs = prefs.getUShort("oledms", 1000);
  if (oledReturnMs < 250) oledReturnMs = 250;
  if (oledReturnMs > 30000) oledReturnMs = 30000;
  Serial.printf("OLED button display timeout=%u ms\n", oledReturnMs);
  String pfx = profilePrefix();
  pressEffect = prefs.getString((pfx + "effect").c_str(), prefs.getString("effect", "AMBIENT_PULSE"));
  if (!validPressEffect(pressEffect)) {
    pressEffect = "AMBIENT_PULSE";
  }
  ambientColor = prefs.getUInt((pfx + "ambient").c_str(), prefs.getUInt("ambient", prefs.getUInt("c0", 0x2040FF)));
  lightingEffect = prefs.getString((pfx + "lightfx").c_str(), prefs.getString("lightfx", "SOLID"));
  if (lightingEffect != "SOLID" && lightingEffect != "BREATH" && lightingEffect != "RAINBOW" && lightingEffect != "CHASE" && lightingEffect != "SPARKLE") {
    lightingEffect = "SOLID";
  }
  lightingSpeed = prefs.getUChar((pfx + "lightspeed").c_str(), prefs.getUChar("lightspeed", 5));
  if (lightingSpeed < 1) lightingSpeed = 1;
  if (lightingSpeed > 10) lightingSpeed = 10;
  ledBrightness = prefs.getUChar((pfx + "ledbright").c_str(), prefs.getUChar("ledbright", 80));
  Serial.printf("Backlight color=%s lighting=%s speed=%u brightness=%u press=%s\n", colorHex(ambientColor).c_str(), lightingEffect.c_str(), lightingSpeed, ledBrightness, pressEffect.c_str());
  eyePersonality = prefs.getString("eyes", "PLAYFUL");
  if (eyePersonality == "SHAKY") eyePersonality = "PLAYFUL";
  if (!validEyePersonality(eyePersonality)) eyePersonality = "PLAYFUL";
  addressScreenMs = prefs.getUShort("addrms", 3000);
  clockScreenMs = prefs.getUShort("clockms", 3000);
  eyeScreenMs = prefs.getUShort("eyems", 5000);
  if (addressScreenMs < 1000) addressScreenMs = 1000;
  if (addressScreenMs > 60000) addressScreenMs = 60000;
  if (clockScreenMs < 1000) clockScreenMs = 1000;
  if (clockScreenMs > 60000) clockScreenMs = 60000;
  if (eyeScreenMs < 1000) eyeScreenMs = 1000;
  if (eyeScreenMs > 60000) eyeScreenMs = 60000;
  screenEnabled = prefs.getBool("screenon", true);
  eyesEnabled = prefs.getBool("eyeson", true);
  addressScreenEnabled = prefs.getBool("addrOn", true);
  clockScreenEnabled = prefs.getBool("clockOn", false);
  eyeScreenEnabled = prefs.getBool("eyeOn", true);
  showPressedOnOled = prefs.getBool("showpress", true);
  oledBrightness = prefs.getUChar("oledbright", 255);
  clockTimeZone = prefs.getString("clocktz", "UTC0");
  if (!validTimeZone(clockTimeZone)) clockTimeZone = "UTC0";
  clockUse24Hour = prefs.getBool("clock24", true);
  clockShowAmPm = prefs.getBool("clockampm", true);
  clockAmPmAfter = prefs.getBool("clockampmafter", true);
  clockShowTimeZone = prefs.getBool("clockshowtz", true);
  clockShowDate = prefs.getBool("clockshowdate", true);
  clockDateFormat = prefs.getString("clockdatefmt", "DOW_D_MON");
  if (!validDateFormat(clockDateFormat)) clockDateFormat = "DOW_D_MON";
  clockFont = prefs.getString("clockfont", "DEFAULT");
  if (!validClockFont(clockFont)) clockFont = "DEFAULT";
  Serial.printf("OLED eyes=%s address=%u ms clock=%u ms eyes=%u ms screen=%u addressOn=%u clockOn=%u eyeOn=%u eyesOn=%u brightness=%u showPress=%u\n", eyePersonality.c_str(), addressScreenMs, clockScreenMs, eyeScreenMs, screenEnabled ? 1 : 0, addressScreenEnabled ? 1 : 0, clockScreenEnabled ? 1 : 0, eyeScreenEnabled ? 1 : 0, eyesEnabled ? 1 : 0, oledBrightness, showPressedOnOled ? 1 : 0);
  Serial.println("Loading settings");
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    String aKey = "a" + String(i);
    String defaultAction = defaultActionFor(i);
    settings[i].action = prefs.getString((pfx + aKey).c_str(), prefs.getString(aKey.c_str(), defaultAction));
    if (!validAction(settings[i].action)) settings[i].action = defaultAction;
    settings[i].longAction = prefs.getString((pfx + "la" + String(i)).c_str(), prefs.getString(("la" + String(i)).c_str(), ""));
    settings[i].longAction.trim();
    if (!validAction(settings[i].longAction)) settings[i].longAction = "";
    settings[i].longPressMs = prefs.getUShort((pfx + "lm" + String(i)).c_str(), prefs.getUShort(("lm" + String(i)).c_str(), 800));
    if (settings[i].longPressMs < 100) settings[i].longPressMs = 100;
    if (settings[i].longPressMs > 10000) settings[i].longPressMs = 10000;
    settings[i].useEyes = prefs.getBool((pfx + "ue" + String(i)).c_str(), prefs.getBool(("ue" + String(i)).c_str(), false));
    settings[i].eyeAnim = prefs.getString((pfx + "ea" + String(i)).c_str(), prefs.getString(("ea" + String(i)).c_str(), "BLINK"));
    settings[i].eyeAnim.trim();
    if (!validKeyEyeAnim(settings[i].eyeAnim)) settings[i].eyeAnim = "BLINK";
    settings[i].eyePos = prefs.getString((pfx + "ep" + String(i)).c_str(), prefs.getString(("ep" + String(i)).c_str(), "DEFAULT"));
    settings[i].eyePos.trim();
    if (!validEyePosition(settings[i].eyePos)) settings[i].eyePos = "DEFAULT";
    settings[i].ledEffect = prefs.getString((pfx + "ke" + String(i)).c_str(), prefs.getString(("ke" + String(i)).c_str(), pressEffect));
    settings[i].ledEffect.trim();
    if (!validPressEffect(settings[i].ledEffect)) settings[i].ledEffect = pressEffect;
    Serial.printf("Key %u GPIO %u action=%s longAction=%s longMs=%u useEyes=%u eyeAnim=%s eyePos=%s ledEffect=%s\n", i + 1, KEY_PINS[i], settings[i].action.c_str(), settings[i].longAction.c_str(), settings[i].longPressMs, settings[i].useEyes ? 1 : 0, settings[i].eyeAnim.c_str(), settings[i].eyePos.c_str(), settings[i].ledEffect.c_str());
  }
}

static void saveSetting(uint8_t i) {
  String pfx = profilePrefix();
  prefs.putString((pfx + "a" + String(i)).c_str(), settings[i].action);
  prefs.putString((pfx + "la" + String(i)).c_str(), settings[i].longAction);
  prefs.putUShort((pfx + "lm" + String(i)).c_str(), settings[i].longPressMs);
  prefs.putBool((pfx + "ue" + String(i)).c_str(), settings[i].useEyes);
  prefs.putString((pfx + "ea" + String(i)).c_str(), settings[i].eyeAnim);
  prefs.putString((pfx + "ep" + String(i)).c_str(), settings[i].eyePos);
  prefs.putString((pfx + "ke" + String(i)).c_str(), settings[i].ledEffect);
}

static void saveLightingSettings() {
  String pfx = profilePrefix();
  prefs.putString((pfx + "effect").c_str(), pressEffect);
  prefs.putUInt((pfx + "ambient").c_str(), ambientColor);
  prefs.putString((pfx + "lightfx").c_str(), lightingEffect);
  prefs.putUChar((pfx + "lightspeed").c_str(), lightingSpeed);
  prefs.putUChar((pfx + "ledbright").c_str(), ledBrightness);
}

static void switchProfile(uint8_t index, bool announce = true) {
  if (index >= profileCount) return;
  activeProfile = index;
  prefs.putUChar("activeProfile", activeProfile);
  loadSettings();
  applyLeds();
  if (announce && screenEnabled) {
    oled("Profile", profileName(activeProfile));
    oledReturnAt = millis() + oledReturnMs;
  }
}

static void nextProfile() {
  switchProfile((activeProfile + 1) % profileCount);
}

static void previousProfile() {
  switchProfile(activeProfile == 0 ? profileCount - 1 : activeProfile - 1);
}

static uint8_t fKey(uint8_t n) {
  const uint8_t keys[] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12};
  return n >= 1 && n <= 12 ? keys[n - 1] : 0;
}

static bool pressOne(String key) {
  key.trim();
  key.toUpperCase();
  if (key.length() == 1) keyboard.press(key[0]);
  else if (key == "ENTER") keyboard.press(KEY_RETURN);
  else if (key == "ESC") keyboard.press(KEY_ESC);
  else if (key == "TAB") keyboard.press(KEY_TAB);
  else if (key == "SPACE") keyboard.press(' ');
  else if (key == "BACKSPACE") keyboard.press(KEY_BACKSPACE);
  else if (key == "DELETE" || key == "DEL") keyboard.press(KEY_DELETE);
  else if (key == "INSERT") keyboard.press(KEY_INSERT);
  else if (key == "HOME") keyboard.press(KEY_HOME);
  else if (key == "END") keyboard.press(KEY_END);
  else if (key == "PAGE_UP" || key == "PGUP") keyboard.press(KEY_PAGE_UP);
  else if (key == "PAGE_DOWN" || key == "PGDN") keyboard.press(KEY_PAGE_DOWN);
  else if (key == "UP") keyboard.press(KEY_UP_ARROW);
  else if (key == "DOWN") keyboard.press(KEY_DOWN_ARROW);
  else if (key == "LEFT") keyboard.press(KEY_LEFT_ARROW);
  else if (key == "RIGHT") keyboard.press(KEY_RIGHT_ARROW);
  else if (key == "CAPS_LOCK") keyboard.press(KEY_CAPS_LOCK);
  else if (key == "PRINT_SCREEN" || key == "PRTSC" || key == "PRTSCR") keyboard.press(KEY_PRINT_SCREEN);
  else if (key.startsWith("F")) {
    uint8_t code = fKey(key.substring(1).toInt());
    if (!code) return false;
    keyboard.press(code);
  } else {
    return false;
  }
  return true;
}

static bool sendMedia(String name) {
  name.trim();
  name.toUpperCase();
  uint16_t code = 0;
  if (name == "PLAY_PAUSE") code = CONSUMER_CONTROL_PLAY_PAUSE;
  else if (name == "NEXT") code = CONSUMER_CONTROL_SCAN_NEXT;
  else if (name == "PREVIOUS") code = CONSUMER_CONTROL_SCAN_PREVIOUS;
  else if (name == "STOP") code = CONSUMER_CONTROL_STOP;
  else if (name == "VOLUME_UP") code = CONSUMER_CONTROL_VOLUME_INCREMENT;
  else if (name == "VOLUME_DOWN") code = CONSUMER_CONTROL_VOLUME_DECREMENT;
  else if (name == "MUTE") code = CONSUMER_CONTROL_MUTE;
  else return false;

  consumer.press(code);
  delay(20);
  consumer.release();
  return true;
}

static void runWindowsCommand(const String &command) {
  if (!command.length()) return;
  keyboard.press(KEY_LEFT_GUI);
  keyboard.press('r');
  delay(40);
  keyboard.releaseAll();
  delay(260);
  keyboard.print(command);
  delay(40);
  keyboard.press(KEY_RETURN);
  delay(30);
  keyboard.releaseAll();
}

static const char *effectLabel(const String &value) {
  for (size_t i = 0; i < sizeof(EFFECTS) / sizeof(EFFECTS[0]); i++) {
    if (value == EFFECTS[i].value) return EFFECTS[i].label;
  }
  return "Unknown";
}

static const char *lightingLabel(const String &value) {
  for (size_t i = 0; i < sizeof(LIGHTING_EFFECTS) / sizeof(LIGHTING_EFFECTS[0]); i++) {
    if (value == LIGHTING_EFFECTS[i].value) return LIGHTING_EFFECTS[i].label;
  }
  return "Unknown";
}

static int choiceIndex(const EffectChoice *choices, size_t count, const String &value) {
  for (size_t i = 0; i < count; i++) {
    if (value == choices[i].value) return (int)i;
  }
  return 0;
}

static uint8_t clockFontIndex() {
  for (uint8_t i = 0; i < sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]); i++) {
    if (clockFont == CLOCK_FONTS[i].value) return i;
  }
  return 0;
}

static void setMenuMessage(const String &title, const String &body) {
  oledMenuPage = MENU_MESSAGE;
  oledMenuIndex = 0;
  oledMenuMessageTitle = title;
  oledMenuMessageBody = body;
}

static void drawMenuList(const String &title, const char *const *items, uint8_t count, uint8_t selected) {
  if (!oledOk) return;
  applyOledDisplaySettings(true);
  stopEyes();
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(title);
  uint8_t first = 0;
  if (selected > 4) first = selected - 4;
  for (uint8_t row = 0; row < 5 && first + row < count; row++) {
    uint8_t idx = first + row;
    int16_t y = 12 + row * 10;
    display.setCursor(0, y);
    display.print(idx == selected ? ">" : " ");
    display.print(items[idx]);
  }
  display.display();
}

static void drawBackHint(int16_t y) {
  drawCenteredLineFit("Back", y, 1);
  display.fillTriangle(42, y, 46, y, 44, y + 4, SSD1306_WHITE);
  display.fillTriangle(82, y, 86, y, 84, y + 4, SSD1306_WHITE);
}

static void drawOledMenuMessage() {
  if (!oledOk) return;
  applyOledDisplaySettings(true);
  stopEyes();
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);
  drawCenteredLineFit(oledMenuMessageTitle, oledMenuMessageBody.length() ? 14 : 22, 1);
  if (oledMenuMessageBody.length()) drawCenteredLineFit(oledMenuMessageBody, 30, 1);
  drawBackHint(54);
  display.display();
}

static void drawOledMenu() {
  if (!oledMenuOpen || !oledOk) return;
  applyOledDisplaySettings(true);
  if (oledMenuPage == MENU_TOP) {
    static const char *const items[] = {"WiFi Setup", "Profiles", "LEDs", "Screen", "Factory Reset", "Update Mode"};
    drawMenuList("OLED Menu", items, 6, oledMenuIndex);
  } else if (oledMenuPage == MENU_WIFI) {
    static const char *const items[] = {"LAN IP", "Current Network", "Reset WiFi"};
    drawMenuList("WiFi Setup", items, 3, oledMenuIndex);
  } else if (oledMenuPage == MENU_LEDS) {
    String color = "Color " + colorHex(ambientColor);
    String effect = String("Effect ") + lightingLabel(lightingEffect);
    String bright = "Brightness " + String(ledBrightness);
    String key = "Keypress effect";
    const char *items[4] = {color.c_str(), effect.c_str(), bright.c_str(), key.c_str()};
    drawMenuList("LEDs", items, 4, oledMenuIndex);
  } else if (oledMenuPage == MENU_LED_KEY_SELECT) {
    const char *items[KEY_COUNT];
    String labels[KEY_COUNT];
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      labels[i] = "Key " + String(i + 1);
      items[i] = labels[i].c_str();
    }
    drawMenuList("LED key", items, KEY_COUNT, oledMenuIndex);
  } else if (oledMenuPage == MENU_LED_KEY_EFFECT) {
    const uint8_t count = sizeof(EFFECTS) / sizeof(EFFECTS[0]);
    const char *items[count];
    for (uint8_t i = 0; i < count; i++) items[i] = EFFECTS[i].label;
    drawMenuList("Key " + String(oledMenuLedKey + 1), items, count, oledMenuIndex);
  } else if (oledMenuPage == MENU_SCREEN) {
    String on = String("Screen ") + (screenEnabled ? "On" : "Off");
    String address = String("IP ") + (addressScreenEnabled ? "On" : "Off");
    String clock = String("Clock ") + (clockScreenEnabled ? "On" : "Off");
    String eyeCycle = String("Eye cycle ") + (eyeScreenEnabled ? "On" : "Off");
    String eyes = String("Key eyes ") + (eyesEnabled ? "On" : "Off");
    String bright = "Brightness " + String(oledBrightness);
    String show = String("Show key ") + (showPressedOnOled ? "On" : "Off");
    String clockSetup = "Clock settings";
    const char *items[8] = {on.c_str(), address.c_str(), clock.c_str(), eyeCycle.c_str(), eyes.c_str(), bright.c_str(), show.c_str(), clockSetup.c_str()};
    drawMenuList("Screen", items, 8, oledMenuIndex);
  } else if (oledMenuPage == MENU_CLOCK_SETTINGS) {
    String font = String("Font ") + clockFontLabel(clockFont);
    String format = String("Format ") + (clockUse24Hour ? "24h" : "12h");
    String ampm = String("AM/PM ") + (clockShowAmPm ? "On" : "Off");
    String ampmPos = String("AM/PM ") + (clockAmPmAfter ? "After" : "Before");
    String zone = String("Zone ") + timeZoneLabel(clockTimeZone);
    String showZone = String("Show zone ") + (clockShowTimeZone ? "On" : "Off");
    String date = String("Date ") + (clockShowDate ? "On" : "Off");
    String dateFmt = String("Date fmt ") + dateFormatLabel(clockDateFormat);
    const char *items[8] = {font.c_str(), format.c_str(), ampm.c_str(), ampmPos.c_str(), zone.c_str(), showZone.c_str(), date.c_str(), dateFmt.c_str()};
    drawMenuList("Clock Settings", items, 8, oledMenuIndex);
  } else if (oledMenuPage == MENU_CLOCK_FONT) {
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextColor(SSD1306_WHITE);
    drawCenteredLineFit("Clock Font", 0, 1);
    uint8_t count = sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]);
    int8_t first = oledMenuIndex > 1 ? oledMenuIndex - 1 : 0;
    if (first > (int8_t)count - 3) first = count > 3 ? count - 3 : 0;
    for (uint8_t row = 0; row < 3 && first + row < count; row++) {
      uint8_t idx = first + row;
      display.setFont(NULL);
      display.setTextSize(1);
      display.setCursor(0, 14 + row * 11);
      display.print(idx == oledMenuIndex ? ">" : " ");
      display.print(CLOCK_FONTS[idx].label);
    }
    String was = clockFont;
    clockFont = CLOCK_FONTS[oledMenuIndex].value;
    drawClockTimeLine(clockUse24Hour ? String("23:59") : (clockShowAmPm ? String("12:34 PM") : String("12:34")), 46, 18);
    clockFont = was;
    display.display();
  } else if (oledMenuPage == MENU_CLOCK_TIMEZONE) {
    uint8_t count = sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]);
    const char *items[count];
    for (uint8_t i = 0; i < count; i++) items[i] = TIME_ZONES[i].label;
    drawMenuList("Time Zone", items, count, oledMenuIndex);
  } else if (oledMenuPage == MENU_CLOCK_DATE_FORMAT) {
    uint8_t count = sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]);
    const char *items[count];
    for (uint8_t i = 0; i < count; i++) items[i] = DATE_FORMATS[i].label;
    drawMenuList("Date Format", items, count, oledMenuIndex);
  } else if (oledMenuPage == MENU_PROFILES) {
    const char *items[MAX_PROFILES];
    String labels[MAX_PROFILES];
    for (uint8_t i = 0; i < profileCount; i++) {
      labels[i] = String(i == activeProfile ? "* " : "  ") + profileName(i);
      items[i] = labels[i].c_str();
    }
    drawMenuList("Profiles", items, profileCount, oledMenuIndex);
  } else if (oledMenuPage == MENU_RESET_CONFIRM) {
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextColor(SSD1306_WHITE);
    drawCenteredLineFit("Factory reset", 0, 1);
    drawCenteredLineFit("Are you sure?", 14, 1);
    display.setTextSize(1);
    display.setCursor(0, 34);
    display.print(oledMenuIndex == 0 ? ">" : " ");
    display.print("Reset all");
    display.setCursor(0, 46);
    display.print(oledMenuIndex == 1 ? ">" : " ");
    display.print("Cancel");
    display.display();
  } else if (oledMenuPage == MENU_UPDATE_CONFIRM) {
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextColor(SSD1306_WHITE);
    drawCenteredLineFit("Update mode", 0, 1);
    drawCenteredLineFit("Are you sure?", 14, 1);
    display.setTextSize(1);
    display.setCursor(0, 34);
    display.print(oledMenuIndex == 0 ? ">" : " ");
    display.print("Start update");
    display.setCursor(0, 46);
    display.print(oledMenuIndex == 1 ? ">" : " ");
    display.print("Cancel");
    display.display();
  } else if (oledMenuPage == MENU_UPDATE_NOW) {
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextColor(SSD1306_WHITE);
    drawCenteredLineFit("Update mode", 26, 1);
    display.display();
  } else {
    drawOledMenuMessage();
  }
}

static void openOledMenu() {
  oledMenuOpen = true;
  oledMenuPage = MENU_TOP;
  oledMenuIndex = 0;
  oledReturnAt = 0;
  drawOledMenu();
}

static void closeOledMenu() {
  oledMenuOpen = false;
  oledMenuPage = MENU_TOP;
  oledMenuIndex = 0;
  showIdleDisplay();
}

static void adjustOledMenu(int8_t delta) {
  if (oledMenuPage == MENU_LEDS) {
    if (oledMenuIndex == 0) {
      static const uint32_t colors[] = {0x2040FF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF, 0xFF8000, 0xFF00FF, 0x00FFFF};
      uint8_t index = 0;
      for (uint8_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
        if (colors[i] == ambientColor) index = i;
      }
      index = (index + (delta > 0 ? 1 : (sizeof(colors) / sizeof(colors[0]) - 1))) % (sizeof(colors) / sizeof(colors[0]));
      ambientColor = colors[index];
      saveLightingSettings();
      applyLeds();
    } else if (oledMenuIndex == 1) {
      int idx = choiceIndex(LIGHTING_EFFECTS, sizeof(LIGHTING_EFFECTS) / sizeof(LIGHTING_EFFECTS[0]), lightingEffect);
      uint8_t count = sizeof(LIGHTING_EFFECTS) / sizeof(LIGHTING_EFFECTS[0]);
      idx = (idx + (delta > 0 ? 1 : count - 1)) % count;
      lightingEffect = LIGHTING_EFFECTS[idx].value;
      saveLightingSettings();
      applyLeds();
    } else if (oledMenuIndex == 2) {
      int value = ledBrightness + delta * 16;
      if (value < 0) value = 0;
      if (value > 255) value = 255;
      ledBrightness = (uint8_t)value;
      saveLightingSettings();
      applyLeds();
    }
  } else if (oledMenuPage == MENU_LED_KEY_EFFECT) {
    uint8_t count = sizeof(EFFECTS) / sizeof(EFFECTS[0]);
    int idx = oledMenuIndex + (delta > 0 ? 1 : count - 1);
    idx %= count;
    oledMenuIndex = (uint8_t)idx;
    settings[oledMenuLedKey].ledEffect = EFFECTS[oledMenuIndex].value;
    saveSetting(oledMenuLedKey);
  } else if (oledMenuPage == MENU_CLOCK_FONT) {
    uint8_t count = sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]);
    int idx = oledMenuIndex + (delta > 0 ? 1 : count - 1);
    idx %= count;
    oledMenuIndex = (uint8_t)idx;
  } else if (oledMenuPage == MENU_CLOCK_TIMEZONE) {
    uint8_t count = sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]);
    int idx = oledMenuIndex + (delta > 0 ? 1 : count - 1);
    idx %= count;
    oledMenuIndex = (uint8_t)idx;
  } else if (oledMenuPage == MENU_CLOCK_DATE_FORMAT) {
    uint8_t count = sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]);
    int idx = oledMenuIndex + (delta > 0 ? 1 : count - 1);
    idx %= count;
    oledMenuIndex = (uint8_t)idx;
  } else if (oledMenuPage == MENU_CLOCK_SETTINGS) {
    if (oledMenuIndex == 1) {
      clockUse24Hour = !clockUse24Hour;
      prefs.putBool("clock24", clockUse24Hour);
    } else if (oledMenuIndex == 2) {
      clockShowAmPm = !clockShowAmPm;
      prefs.putBool("clockampm", clockShowAmPm);
    } else if (oledMenuIndex == 3) {
      clockAmPmAfter = !clockAmPmAfter;
      prefs.putBool("clockampmafter", clockAmPmAfter);
    } else if (oledMenuIndex == 5) {
      clockShowTimeZone = !clockShowTimeZone;
      prefs.putBool("clockshowtz", clockShowTimeZone);
    } else if (oledMenuIndex == 6) {
      clockShowDate = !clockShowDate;
      prefs.putBool("clockshowdate", clockShowDate);
    }
  } else if (oledMenuPage == MENU_SCREEN && oledMenuIndex == 5) {
    int value = oledBrightness + delta * 16;
    if (value < 1) value = 1;
    if (value > 255) value = 255;
    oledBrightness = (uint8_t)value;
    prefs.putUChar("oledbright", oledBrightness);
    applyOledDisplaySettings(true);
  }
  drawOledMenu();
}

static void selectOledMenu() {
  if (oledMenuPage == MENU_TOP) {
    if (oledMenuIndex == 0) {
      oledMenuPage = MENU_WIFI;
      oledMenuIndex = 0;
    } else if (oledMenuIndex == 1) {
      oledMenuPage = MENU_PROFILES;
      oledMenuIndex = 0;
    } else if (oledMenuIndex == 2) {
      oledMenuPage = MENU_LEDS;
      oledMenuIndex = 0;
    } else if (oledMenuIndex == 3) {
      oledMenuPage = MENU_SCREEN;
      oledMenuIndex = 0;
    } else if (oledMenuIndex == 4) {
      oledMenuPage = MENU_RESET_CONFIRM;
      oledMenuIndex = 1;
    } else {
      oledMenuPage = MENU_UPDATE_CONFIRM;
      oledMenuIndex = 1;
    }
  } else if (oledMenuPage == MENU_PROFILES) {
    switchProfile(oledMenuIndex, false);
    setMenuMessage("Profile", profileName(activeProfile));
  } else if (oledMenuPage == MENU_RESET_CONFIRM) {
    if (oledMenuIndex == 0) {
      prefs.clear();
      WiFi.disconnect(false, true);
      loadSettings();
      configureClockTime();
      applyOledDisplaySettings(true);
      applyLeds();
      setMenuMessage("Factory reset", "Complete");
    } else {
      oledMenuPage = MENU_TOP;
      oledMenuIndex = 4;
    }
  } else if (oledMenuPage == MENU_UPDATE_CONFIRM) {
    if (oledMenuIndex == 0) {
      usbUploadModePending = true;
      restartAt = millis() + 900;
      oledMenuPage = MENU_UPDATE_NOW;
      oledMenuIndex = 0;
    } else {
      oledMenuPage = MENU_TOP;
      oledMenuIndex = 5;
    }
  } else if (oledMenuPage == MENU_WIFI) {
    if (oledMenuIndex == 0) {
      setMenuMessage("LAN IP", WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("Not connected"));
    } else if (oledMenuIndex == 1) {
      setMenuMessage("Network", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String("Setup hotspot"));
    } else {
      prefs.remove("ssid");
      prefs.remove("pass");
      WiFi.disconnect(false, true);
      setMenuMessage("WiFi reset", "Use hotspot setup");
    }
  } else if (oledMenuPage == MENU_LEDS) {
    if (oledMenuIndex == 3) {
      oledMenuPage = MENU_LED_KEY_SELECT;
      oledMenuIndex = 0;
    }
  } else if (oledMenuPage == MENU_LED_KEY_SELECT) {
    oledMenuLedKey = oledMenuIndex;
    oledMenuPage = MENU_LED_KEY_EFFECT;
    oledMenuIndex = choiceIndex(EFFECTS, sizeof(EFFECTS) / sizeof(EFFECTS[0]), settings[oledMenuLedKey].ledEffect);
  } else if (oledMenuPage == MENU_SCREEN) {
    if (oledMenuIndex == 0) {
      screenEnabled = !screenEnabled;
      prefs.putBool("screenon", screenEnabled);
      if (screenEnabled) applyOledDisplaySettings(true);
    } else if (oledMenuIndex == 1) {
      addressScreenEnabled = !addressScreenEnabled;
      prefs.putBool("addrOn", addressScreenEnabled);
    } else if (oledMenuIndex == 2) {
      clockScreenEnabled = !clockScreenEnabled;
      prefs.putBool("clockOn", clockScreenEnabled);
    } else if (oledMenuIndex == 3) {
      eyeScreenEnabled = !eyeScreenEnabled;
      prefs.putBool("eyeOn", eyeScreenEnabled);
    } else if (oledMenuIndex == 4) {
      eyesEnabled = !eyesEnabled;
      prefs.putBool("eyeson", eyesEnabled);
    } else if (oledMenuIndex == 6) {
      showPressedOnOled = !showPressedOnOled;
      prefs.putBool("showpress", showPressedOnOled);
    } else if (oledMenuIndex == 7) {
      oledMenuPage = MENU_CLOCK_SETTINGS;
      oledMenuIndex = 0;
    }
  } else if (oledMenuPage == MENU_CLOCK_SETTINGS) {
    if (oledMenuIndex == 0) {
      oledMenuPage = MENU_CLOCK_FONT;
      oledMenuIndex = clockFontIndex();
    } else if (oledMenuIndex == 1) {
      clockUse24Hour = !clockUse24Hour;
      prefs.putBool("clock24", clockUse24Hour);
    } else if (oledMenuIndex == 2) {
      clockShowAmPm = !clockShowAmPm;
      prefs.putBool("clockampm", clockShowAmPm);
    } else if (oledMenuIndex == 3) {
      clockAmPmAfter = !clockAmPmAfter;
      prefs.putBool("clockampmafter", clockAmPmAfter);
    } else if (oledMenuIndex == 4) {
      oledMenuPage = MENU_CLOCK_TIMEZONE;
      oledMenuIndex = timeZoneIndex();
    } else if (oledMenuIndex == 5) {
      clockShowTimeZone = !clockShowTimeZone;
      prefs.putBool("clockshowtz", clockShowTimeZone);
    } else if (oledMenuIndex == 6) {
      clockShowDate = !clockShowDate;
      prefs.putBool("clockshowdate", clockShowDate);
    } else if (oledMenuIndex == 7) {
      oledMenuPage = MENU_CLOCK_DATE_FORMAT;
      oledMenuIndex = dateFormatIndex();
    }
  } else if (oledMenuPage == MENU_CLOCK_FONT) {
    clockFont = CLOCK_FONTS[oledMenuIndex].value;
    prefs.putString("clockfont", clockFont);
    setMenuMessage("Clock font", clockFontLabel(clockFont));
  } else if (oledMenuPage == MENU_CLOCK_TIMEZONE) {
    clockTimeZone = TIME_ZONES[oledMenuIndex].value;
    prefs.putString("clocktz", clockTimeZone);
    configureClockTime();
    setMenuMessage("Time zone", timeZoneLabel(clockTimeZone));
  } else if (oledMenuPage == MENU_CLOCK_DATE_FORMAT) {
    clockDateFormat = DATE_FORMATS[oledMenuIndex].value;
    prefs.putString("clockdatefmt", clockDateFormat);
    setMenuMessage("Date format", dateFormatLabel(clockDateFormat));
  }
  drawOledMenu();
}

static void backOledMenu() {
  if (oledMenuPage == MENU_TOP) {
    closeOledMenu();
  } else if (oledMenuPage == MENU_PROFILES) {
    oledMenuPage = MENU_TOP;
    oledMenuIndex = 1;
  } else if (oledMenuPage == MENU_LED_KEY_EFFECT) {
    oledMenuPage = MENU_LED_KEY_SELECT;
    oledMenuIndex = oledMenuLedKey;
  } else if (oledMenuPage == MENU_LED_KEY_SELECT) {
    oledMenuPage = MENU_LEDS;
    oledMenuIndex = 3;
  } else if (oledMenuPage == MENU_CLOCK_SETTINGS) {
    oledMenuPage = MENU_SCREEN;
    oledMenuIndex = 7;
  } else if (oledMenuPage == MENU_CLOCK_FONT) {
    oledMenuPage = MENU_CLOCK_SETTINGS;
    oledMenuIndex = 0;
  } else if (oledMenuPage == MENU_CLOCK_TIMEZONE) {
    oledMenuPage = MENU_CLOCK_SETTINGS;
    oledMenuIndex = 4;
  } else if (oledMenuPage == MENU_CLOCK_DATE_FORMAT) {
    oledMenuPage = MENU_CLOCK_SETTINGS;
    oledMenuIndex = 7;
  } else if (oledMenuPage == MENU_UPDATE_CONFIRM) {
    oledMenuPage = MENU_TOP;
    oledMenuIndex = 5;
  } else if (oledMenuPage == MENU_RESET_CONFIRM) {
    oledMenuPage = MENU_TOP;
    oledMenuIndex = 4;
  } else if (oledMenuPage == MENU_UPDATE_NOW) {
    return;
  } else {
    oledMenuPage = MENU_TOP;
    oledMenuIndex = 0;
  }
  drawOledMenu();
}

static void handleOledMenuKey(uint8_t keyIndex) {
  if (keyIndex == 9) key10HoldHandled = true;
  uint8_t maxIndex = 0;
  if (oledMenuPage == MENU_TOP) maxIndex = 5;
  else if (oledMenuPage == MENU_LEDS) maxIndex = 3;
  else if (oledMenuPage == MENU_SCREEN) maxIndex = 7;
  else if (oledMenuPage == MENU_CLOCK_SETTINGS) maxIndex = 7;
  else if (oledMenuPage == MENU_WIFI) maxIndex = 2;
  else if (oledMenuPage == MENU_PROFILES) maxIndex = profileCount ? profileCount - 1 : 0;
  else if (oledMenuPage == MENU_LED_KEY_SELECT) maxIndex = KEY_COUNT - 1;
  else if (oledMenuPage == MENU_LED_KEY_EFFECT) maxIndex = (sizeof(EFFECTS) / sizeof(EFFECTS[0])) - 1;
  else if (oledMenuPage == MENU_CLOCK_FONT) maxIndex = (sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0])) - 1;
  else if (oledMenuPage == MENU_CLOCK_TIMEZONE) maxIndex = (sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0])) - 1;
  else if (oledMenuPage == MENU_CLOCK_DATE_FORMAT) maxIndex = (sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0])) - 1;
  else if (oledMenuPage == MENU_RESET_CONFIRM) maxIndex = 1;
  else if (oledMenuPage == MENU_UPDATE_CONFIRM) maxIndex = 1;
  else if (oledMenuPage == MENU_UPDATE_NOW) return;

  if (keyIndex == 1) {
    oledMenuIndex = oledMenuIndex == 0 ? maxIndex : oledMenuIndex - 1;
    drawOledMenu();
  } else if (keyIndex == 7) {
    oledMenuIndex = oledMenuIndex >= maxIndex ? 0 : oledMenuIndex + 1;
    drawOledMenu();
  } else if (keyIndex == 3) {
    if (oledMenuPage == MENU_LEDS || oledMenuPage == MENU_LED_KEY_EFFECT || oledMenuPage == MENU_SCREEN || oledMenuPage == MENU_CLOCK_SETTINGS || oledMenuPage == MENU_CLOCK_FONT || oledMenuPage == MENU_CLOCK_TIMEZONE || oledMenuPage == MENU_CLOCK_DATE_FORMAT) adjustOledMenu(-1);
    else backOledMenu();
  } else if (keyIndex == 5) {
    if (oledMenuPage == MENU_LEDS || oledMenuPage == MENU_LED_KEY_EFFECT || oledMenuPage == MENU_SCREEN || oledMenuPage == MENU_CLOCK_SETTINGS || oledMenuPage == MENU_CLOCK_FONT || oledMenuPage == MENU_CLOCK_TIMEZONE || oledMenuPage == MENU_CLOCK_DATE_FORMAT) adjustOledMenu(1);
    else selectOledMenu();
  } else if (keyIndex == 4) {
    selectOledMenu();
  } else if (keyIndex == 9) {
    backOledMenu();
  }
}

static void runAction(uint8_t i, const String &requestedAction = "") {
  String action = requestedAction.length() ? requestedAction : settings[i].action;
  action.trim();
  if (!action.length()) return;

  Serial.printf("Pressed key %u action=%s\n", i + 1, action.c_str());
  if (action == "PROFILE:NEXT") {
    nextProfile();
    return;
  }
  if (action == "PROFILE:PREV") {
    previousProfile();
    return;
  }
  if (action.startsWith("PROFILE:")) {
    switchProfile((uint8_t)action.substring(8).toInt());
    return;
  }
  if (settings[i].useEyes && eyesEnabled && screenEnabled) {
    startKeyEyes(i);
  } else if (showPressedOnOled && screenEnabled) {
    oled("Key " + String(i + 1), action);
    oledReturnAt = millis() + oledReturnMs;
  }
  playKeyEffect(i);

  if (action.startsWith("MEDIA:")) {
    sendMedia(action.substring(6));
    return;
  }

  if (action.startsWith("TEXT:")) {
    keyboard.print(action.substring(5));
    return;
  }

  if (action.startsWith("RUN:")) {
    runWindowsCommand(action.substring(4));
    return;
  }

  if (action.startsWith("KEY:")) action = action.substring(4);
  action.toUpperCase();

  int start = 0;
  bool pressed = false;
  while (start < action.length()) {
    int plus = action.indexOf('+', start);
    String token = plus < 0 ? action.substring(start) : action.substring(start, plus);
    token.trim();
    if (token == "CTRL" || token == "CONTROL") {
      keyboard.press(KEY_LEFT_CTRL);
      pressed = true;
    } else if (token == "SHIFT") {
      keyboard.press(KEY_LEFT_SHIFT);
      pressed = true;
    } else if (token == "ALT") {
      keyboard.press(KEY_LEFT_ALT);
      pressed = true;
    } else if (token == "GUI" || token == "WIN" || token == "WINDOWS" || token == "CMD" || token == "META") {
      keyboard.press(KEY_LEFT_GUI);
      pressed = true;
    } else if (pressOne(token)) {
      pressed = true;
    }
    if (plus < 0) break;
    start = plus + 1;
  }

  if (pressed) {
    delay(20);
    keyboard.releaseAll();
  }
}

static void scanKeys() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    bool reading = rawDown(i);
    if (reading != lastRawDown[i]) {
      lastRawDown[i] = reading;
      changedAt[i] = now;
    }
    if (now - changedAt[i] >= DEBOUNCE_MS && reading != stableDown[i]) {
      stableDown[i] = reading;
      if (stableDown[i]) {
        keyPressedAt[i] = now;
        actionSentOnPress[i] = false;
        longActionFired[i] = false;
        if (oledMenuOpen) {
          handleOledMenuKey(i);
        } else if (i != 9) {
          if (!settings[i].longAction.length()) {
            runAction(i);
            actionSentOnPress[i] = true;
          }
        }
      } else if (!oledMenuOpen) {
        if (!actionSentOnPress[i] && !key10HoldHandled) {
          runAction(i);
        }
        keyPressedAt[i] = 0;
        actionSentOnPress[i] = false;
        longActionFired[i] = false;
      }
    }
    if (i != 9 && stableDown[i] && !oledMenuOpen && settings[i].longAction.length() && keyPressedAt[i] && !longActionFired[i]) {
      if (now - keyPressedAt[i] >= settings[i].longPressMs) {
        runAction(i, settings[i].longAction);
        longActionFired[i] = true;
        actionSentOnPress[i] = true;
      }
    }
    if (i == 9) {
      if (stableDown[i] && !oledMenuOpen) {
        if (!key10HoldStartedAt) key10HoldStartedAt = now;
        if (!key10HoldHandled && now - key10HoldStartedAt >= 5000) {
          key10HoldHandled = true;
          openOledMenu();
        }
      } else {
        key10HoldStartedAt = 0;
        if (!stableDown[i]) key10HoldHandled = false;
      }
    }
  }
}

static String formatUptime() {
  unsigned long seconds = millis() / 1000;
  unsigned long days = seconds / 86400;
  seconds %= 86400;
  uint8_t hours = seconds / 3600;
  seconds %= 3600;
  uint8_t minutes = seconds / 60;
  seconds %= 60;
  String out;
  if (days) out += String(days) + "d ";
  out += String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
  return out;
}

static bool clockHasSynced() {
  time_t now = time(nullptr);
  return now >= 1700000000;
}

static const char *resetReasonText() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON: return "Power on";
    case ESP_RST_EXT: return "External reset";
    case ESP_RST_SW: return "Software restart";
    case ESP_RST_PANIC: return "Panic";
    case ESP_RST_INT_WDT: return "Interrupt watchdog";
    case ESP_RST_TASK_WDT: return "Task watchdog";
    case ESP_RST_WDT: return "Other watchdog";
    case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
    case ESP_RST_BROWNOUT: return "Brownout";
    case ESP_RST_SDIO: return "SDIO reset";
    default: return "Unknown";
  }
}

static String header(const String &title) {
  String h = F("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>");
  h += F("<title>");
  h += htmlEscape(title);
  h += F("</title><style>");
  h += F("body{margin:0;background:#11161b;color:#edf2f7;font-family:system-ui,Arial}main{max-width:1160px;margin:auto;padding:18px}");
  h += F(".brand{display:flex;align-items:center;justify-content:space-between;gap:14px}.brandLeft{display:flex;align-items:center;gap:14px}.profilePick{display:flex;align-items:end;gap:8px}.profilePick select{min-width:170px}.eyeBox{width:104px;height:54px;border:1px solid #303b46;border-radius:8px;background:#05080b;display:flex;align-items:center;justify-content:center;gap:8px;overflow:hidden;box-shadow:inset 0 0 20px rgba(47,117,200,.2)}.roboEye{width:28px;height:27px;border-radius:7px;background:#edf2f7;position:relative;animation:roboBlink 4.1s steps(1,end) infinite,roboLook 4.8s cubic-bezier(.3,0,.2,1) infinite;transform-origin:center}.roboEye:before,.roboEye:after{content:'';position:absolute;left:-3px;right:-3px;height:13px;background:#05080b;opacity:0}.roboEye:before{top:-8px}.roboEye:after{bottom:-8px}.eyes-HAPPY .roboEye:after{opacity:1;border-radius:50% 50% 0 0}.eyes-ANGRY .roboEye:before{opacity:1;transform:rotate(12deg);transform-origin:left bottom}.eyes-TIRED .roboEye{height:21px}.eyes-TIRED .roboEye:before{opacity:1}.eyes-CYCLOPS{gap:0}.eyes-CYCLOPS .roboEye:first-child{width:48px;height:32px;border-radius:9px}.eyes-CYCLOPS .roboEye:last-child{display:none}@keyframes roboBlink{0%,88%,93%,100%{scale:1 1}90%,91%{scale:1 .08}}@keyframes roboLook{0%,18%,100%{translate:0 0}25%,38%{translate:9px 0}45%,58%{translate:-8px 0}65%,76%{translate:0 -5px}83%,92%{translate:0 5px}}");
  h += F("section{border:1px solid #303b46;border-radius:8px;background:#171e25;padding:14px;margin:14px 0}");
  h += F(".panels{display:grid;grid-template-columns:1fr;gap:14px}.col{display:flex;flex-direction:column;gap:14px}.panels section{margin:0}.full{grid-column:1/-1}.sectionHead{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:10px}.sectionHead h2{margin:0}");
  h += F(".key{display:grid;grid-template-columns:minmax(190px,.7fr) minmax(220px,1.2fr) minmax(150px,.55fr) minmax(110px,.4fr);gap:12px;align-items:end;border-top:1px solid #26313b;padding:12px 0}.key:first-of-type{border-top:0}");
  h += F(".name{display:grid;grid-template-columns:16px 1fr;column-gap:8px;row-gap:2px;align-items:center;padding-bottom:4px}.name strong{line-height:1.1}.keyMeta{grid-column:2;color:#aeb8c2;font-size:12px;line-height:1.3}.dot{width:16px;height:16px;border-radius:50%;background:#3a4652;border:1px solid #596675}");
  h += F(".raw .dot{background:#ffcf33;box-shadow:0 0 10px #ffcf33}.down .dot{background:#38e879;box-shadow:0 0 12px #38e879}");
  h += F("label{display:block;font-size:13px;color:#aeb8c2;margin-bottom:4px}select,input,button{box-sizing:border-box;width:100%;font:inherit;border-radius:6px;border:1px solid #43505c;background:#0d1115;color:#edf2f7;padding:8px}select:focus,input:focus{outline:2px solid var(--accent);outline-offset:1px}");
  h += F("input[type=checkbox]{width:auto;accent-color:var(--accent)}input[type=range]{accent-color:var(--accent);padding-left:0;padding-right:0}input[type=range]::-webkit-slider-thumb{background:var(--accent)}input[type=range]::-moz-range-thumb{background:var(--accent);border-color:var(--accent)}.check{display:flex;gap:8px;align-items:center;padding-bottom:9px}.check label{margin:0}");
  h += F(".eyeGrid{display:grid;grid-template-columns:repeat(3,minmax(120px,1fr));gap:8px}.eyeGrid input{position:absolute;opacity:0;pointer-events:none}.eyeGrid label{display:flex;align-items:center;justify-content:center;min-height:48px;margin:0;border:1px solid #43505c;border-radius:6px;background:#0d1115;color:#aeb8c2;font-size:14px;cursor:pointer;text-align:center}.eyeGrid input:checked+label{border-color:var(--accent);background:var(--accent);color:#edf2f7}.btn.disabled{border-color:#43505c;background:#27313a;color:#87919b;pointer-events:none}");
  h += F("input[type=color]{height:42px;padding:3px;cursor:pointer}input[type=file]{padding:7px}.field{max-width:340px;margin-top:14px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}.displayRows{display:flex;flex-direction:column;gap:10px}.displayLine{display:grid;grid-template-columns:minmax(180px,1fr) 120px minmax(210px,1.3fr);gap:10px;align-items:center}.displayLine label{margin:0}.displayLine input[type=number]{text-align:right}.displayLine .check{padding:0}.displayLine .btn{justify-self:start}.displayGrid{margin-top:14px}");
  h += F("button,.btn{display:inline-block;box-sizing:border-box;width:auto;font:inherit;border-radius:6px;border:1px solid var(--accent);background:var(--accent-dark);color:#edf2f7;padding:8px 12px;text-decoration:none;cursor:pointer}.key .btn{width:100%;text-align:center}.row{display:flex;gap:10px;flex-wrap:wrap}.row>*{width:auto}.eyeField{max-width:520px;margin-top:16px}.menuHelp{border:1px solid #303b46;border-radius:8px;background:#11161b;padding:12px;margin:16px 0}.menuHelp h3{margin:0 0 8px}.detailsPanel{margin-top:16px;border:1px solid #303b46;border-radius:8px;background:#11161b;padding:10px 12px}.detailsPanel summary{cursor:pointer;color:#edf2f7;font-weight:700}.diagGrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:8px 14px;margin-top:12px}.diagItem{color:#aeb8c2}.diagItem strong{display:block;color:#edf2f7}.hardwareList{line-height:1.45;margin-bottom:0}.hardwareList li{margin:0 0 12px}.hardwareList li:last-child{margin-bottom:0}.aboutBox{margin-top:16px;border:1px solid #303b46;border-radius:8px;background:#11161b;padding:10px 12px}.aboutBox h3{margin:0 0 10px}.aboutBox p{margin:8px 0}.warn{color:#ffd47a}.ok{color:#82f7b1}.muted{color:#aeb8c2}a{color:#86bdff}ul{margin-top:8px;padding-left:22px}");
  h += F("@media(min-width:900px){.panels{grid-template-columns:1fr 1fr;align-items:start}}@media(max-width:680px){.key{grid-template-columns:1fr}.displayLine{grid-template-columns:1fr}.displayLine .btn{width:100%;text-align:center}.row>*{width:100%}button,.btn{width:100%;text-align:center}.brand{align-items:flex-start;flex-direction:column}.profilePick{width:100%;align-items:stretch;flex-direction:column}.profilePick select{min-width:0}.eyeBox{width:82px;height:44px}.roboEye{width:22px;height:21px}}</style></head><body style='--accent:");
  h += colorHex(ambientColor);
  h += F(";--accent-dark:");
  h += colorHex(scaleColor(ambientColor, 128));
  h += F("'><main>");
  h += F("<div class='brand'><div class='brandLeft'><div class='eyeBox eyes-");
  if (eyePersonality == "HAPPY" || eyePersonality == "CURIOUS" || eyePersonality == "DREAMY") h += F("HAPPY");
  else if (eyePersonality == "ANGRY" || eyePersonality == "DRAMATIC" || eyePersonality == "JUDGY") h += F("ANGRY");
  else if (eyePersonality == "SLEEPY" || eyePersonality == "CHILL" || eyePersonality == "NERVOUS") h += F("TIRED");
  else if (eyePersonality == "CYCLOPS" || eyePersonality == "CYCLOPS_SCAN") h += F("CYCLOPS");
  else h += F("DEFAULT");
  h += F("' aria-hidden='true'><span class='roboEye'></span><span class='roboEye'></span></div><h1>");
  h += htmlEscape(title);
  h += F("</h1></div>");
  if (title == "MacroLad") {
    h += F("<form class='profilePick' method='GET' action='/profile'><div><label>Profile</label><select name='p' onchange='this.form.submit()'>");
    for (uint8_t i = 0; i < profileCount; i++) {
      h += F("<option value='");
      h += String(i);
      h += F("'");
      if (i == activeProfile) h += F(" selected");
      h += F(">");
      h += htmlEscape(profileName(i));
      h += F("</option>");
    }
    h += F("</select></div><a class='btn' href='/profiles'>Manage profiles</a></form>");
  }
  h += F("</div>");
  return h;
}

static void appendChoices(String &html, const String &selected) {
  bool found = false;
  for (size_t i = 0; i < sizeof(ACTIONS) / sizeof(ACTIONS[0]); i++) {
    html += F("<option value='");
    html += ACTIONS[i].value;
    html += F("'");
    if (selected == ACTIONS[i].value) {
      html += F(" selected");
      found = true;
    }
    html += F(">");
    html += ACTIONS[i].label;
    html += F("</option>");
  }
  for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
    if (!customCombos[i].length()) continue;
    html += F("<option value='");
    html += htmlEscape(customCombos[i]);
    html += F("'");
    if (selected == customCombos[i]) {
      html += F(" selected");
      found = true;
    }
    html += F(">Custom: ");
    html += htmlEscape(comboLabel(customCombos[i]));
    html += F("</option>");
  }
  html += F("<option value='PROFILE:NEXT'");
  if (selected == "PROFILE:NEXT") {
    html += F(" selected");
    found = true;
  }
  html += F(">Profile: load next</option>");
  html += F("<option value='PROFILE:PREV'");
  if (selected == "PROFILE:PREV") {
    html += F(" selected");
    found = true;
  }
  html += F(">Profile: load previous</option>");
  for (uint8_t i = 0; i < profileCount; i++) {
    String value = "PROFILE:" + String(i);
    html += F("<option value='");
    html += value;
    html += F("'");
    if (selected == value) {
      html += F(" selected");
      found = true;
    }
    html += F(">Profile: ");
    html += htmlEscape(profileName(i));
    html += F("</option>");
  }
  if (!found && selected.length()) {
    html += F("<option value='");
    html += htmlEscape(selected);
    html += F("' selected>Saved action</option>");
  }
}

static const char *physicalKeyPosition(uint8_t keyIndex) {
  static const char *positions[KEY_COUNT] = {
    "Top left",
    "Top middle",
    "Top right",
    "Centre left",
    "Centre",
    "Centre right",
    "Bottom left",
    "Bottom middle",
    "Bottom right",
    "Below screen"
  };
  return keyIndex < KEY_COUNT ? positions[keyIndex] : "";
}

static void appendComboPartChoices(String &html) {
  for (size_t i = 0; i < sizeof(COMBO_PARTS) / sizeof(COMBO_PARTS[0]); i++) {
    html += F("<option value='");
    html += COMBO_PARTS[i].value;
    html += F("'>");
    html += COMBO_PARTS[i].label;
    html += F("</option>");
  }
}

static void appendEffectChoices(String &html, const String &selected) {
  for (size_t i = 0; i < sizeof(EFFECTS) / sizeof(EFFECTS[0]); i++) {
    html += F("<option value='");
    html += EFFECTS[i].value;
    html += F("'");
    if (selected == EFFECTS[i].value) html += F(" selected");
    html += F(">");
    html += EFFECTS[i].label;
    html += F("</option>");
  }
}

static void appendLightingChoices(String &html, const String &selected) {
  for (size_t i = 0; i < sizeof(LIGHTING_EFFECTS) / sizeof(LIGHTING_EFFECTS[0]); i++) {
    html += F("<option value='");
    html += LIGHTING_EFFECTS[i].value;
    html += F("'");
    if (selected == LIGHTING_EFFECTS[i].value) html += F(" selected");
    html += F(">");
    html += LIGHTING_EFFECTS[i].label;
    html += F("</option>");
  }
}

static void appendDisplayChoices(String &html, const DisplayChoice *choices, size_t count, const String &selected) {
  for (size_t i = 0; i < count; i++) {
    html += F("<option value='");
    html += choices[i].value;
    html += F("'");
    if (selected == choices[i].value) html += F(" selected");
    html += F(">");
    html += choices[i].label;
    html += F("</option>");
  }
}

static void appendClockFontChoices(String &html, const String &selected) {
  for (size_t i = 0; i < sizeof(CLOCK_FONTS) / sizeof(CLOCK_FONTS[0]); i++) {
    html += F("<option value='");
    html += CLOCK_FONTS[i].value;
    html += F("'");
    if (selected == CLOCK_FONTS[i].value) html += F(" selected");
    html += F(">");
    html += CLOCK_FONTS[i].label;
    html += F("</option>");
  }
}

static void appendEyePositionGrid(String &html, uint8_t keyIndex, const String &selected) {
  static const DisplayChoice grid[] = {
    {"NW", "Top left"}, {"N", "Top"}, {"NE", "Top right"},
    {"W", "Left"}, {"DEFAULT", "Center"}, {"E", "Right"},
    {"SW", "Bottom left"}, {"S", "Bottom"}, {"SE", "Bottom right"}
  };
  String current = validEyePosition(selected) ? selected : "DEFAULT";
  html += F("<label>Eye position</label><div class='eyeGrid'>");
  for (size_t i = 0; i < sizeof(grid) / sizeof(grid[0]); i++) {
    String id = "ep" + String(keyIndex) + "_" + String(i);
    html += F("<input type='radio' id='");
    html += id;
    html += F("' name='ep");
    html += String(keyIndex);
    html += F("' value='");
    html += grid[i].value;
    html += F("'");
    if (current == grid[i].value) html += F(" checked");
    html += F("><label for='");
    html += id;
    html += F("'>");
    html += grid[i].label;
    html += F("</label>");
  }
  html += F("</div>");
}

static void rootPage() {
  scanKeys();
  String html = header("MacroLad");
#if MACROPAD_USB_HID
#else
  html += F("<section><strong class='warn'>USB HID build: disabled.</strong><p class='muted'>Your selected board options include USBMode=hwcdc. That can let the web UI run, but it will not create a USB keyboard. Use USB Mode: USB-OTG (TinyUSB) for real key output.</p></section>");
#endif

  html += F("<section class='full'><div class='sectionHead'><h2>Keys</h2><a class='btn' href='/combos'>Manage Key Combinations</a></div><form method='GET' action='/save'>");
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    html += F("<div class='key' id='k");
    html += String(i);
    html += F("'><div class='name'><span class='dot'></span><strong>Key ");
    html += String(i + 1);
    html += F("</strong><span class='keyMeta'>");
    html += physicalKeyPosition(i);
    html += F(" &middot; GPIO ");
    html += String(KEY_PINS[i]);
    html += F("</span></div><div><label>Action</label><select name='a");
    html += String(i);
  html += F("'>");
  appendChoices(html, settings[i].action);
    html += F("</select></div><div>");
    if (i == 9) {
      html += F("<span class='btn disabled'>Long press action reserved for menu</span>");
    } else {
      html += F("<a class='btn' href='/long?k=");
      html += String(i);
      html += F("'>Long press action</a>");
    }
    html += F("</div><div><a class='btn' href='/key?k=");
    html += String(i);
    html += F("'>Effects</a></div></div>");
    if (i == 9) {
      html += F("<div class='menuHelp'><p class='muted'>Hold key 10 for 5 seconds to open the on-device OLED menu.</p><p class='muted'>Menu controls: key 2 moves up, key 8 moves down, key 4 goes left or back, key 6 goes right or changes a value, key 5 selects, and key 10 exits.</p></div>");
    }
  }
  html += F("<p><button type='submit'>Save key settings</button></p></form></section>");
  html += F("<div class='panels'><div class='col'>");

  String ambientHex = colorHex(ambientColor);
  html += F("<section><h2>Lighting</h2><form method='GET' action='/save'><div class='grid'><div><label>Backlight color</label><input type='color' name='lc' value='");
  html += ambientHex;
  html += F("' style='background:");
  html += ambientHex;
  html += F("'></div><div><label>Ambient animation</label><select name='le'>");
  appendLightingChoices(html, lightingEffect);
  html += F("</select></div><div><label>Animation speed</label><input type='range' name='ls' min='1' max='10' value='");
  html += String(lightingSpeed);
  html += F("'></div><div><label>LED brightness</label><input type='range' name='lb' min='0' max='255' value='");
  html += String(ledBrightness);
  html += F("'></div><div><label>Default keypress flash</label><select name='e'>");
  appendEffectChoices(html, pressEffect);
  html += F("</select></div></div><p><button type='submit'>Save lighting settings</button></p></form>");
  html += F("</section>");

  html += F("<section><h2>System, Firmware, and About</h2>");
  html += F("<div class='row'><form method='GET' action='/reset'><button type='submit'>Factory reset</button></form>");
  html += F("<form method='POST' action='/usb-upload-mode'><button type='submit'>Restart into USB upload mode</button></form></div>");
  html += F("<details class='detailsPanel'><summary>Diagnostics</summary><p id='live' class='muted'>Waiting for live state...</p><p class='muted'>Yellow = raw GPIO low. Green = debounced press.</p><div class='diagGrid'>");
  html += F("<div class='diagItem'><strong>Free heap</strong>");
  html += String(ESP.getFreeHeap());
  html += F(" bytes</div><div class='diagItem'><strong>Minimum free heap</strong>");
  html += String(ESP.getMinFreeHeap());
  html += F(" bytes</div><div class='diagItem'><strong>Sketch size</strong>");
  html += String(ESP.getSketchSize());
  html += F(" bytes</div><div class='diagItem'><strong>Free sketch space</strong>");
  html += String(ESP.getFreeSketchSpace());
  html += F(" bytes</div><div class='diagItem'><strong>Uptime</strong><span id='uptime'>");
  html += formatUptime();
  html += F("</span></div><div class='diagItem'><strong>Reset reason</strong>");
  html += resetReasonText();
  html += F("</div><div class='diagItem'><strong>Active profile</strong>");
  html += htmlEscape(profileName(activeProfile));
  html += F("</div><div class='diagItem'><strong>LAN IP</strong>");
  html += WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("Not connected");
  html += F("</div><div class='diagItem'><strong>Setup hotspot</strong>");
  html += String(AP_SSID) + " " + WiFi.softAPIP().toString();
  html += F("</div><div class='diagItem'><strong>Clock sync</strong><span id='clockSync'>");
  html += clockHasSynced() ? F("Synced") : F("Waiting");
  html += F("</span></div></div></details>");
  html += F("<details class='detailsPanel'><summary>Hardware</summary><ul class='hardwareList'>");
  html += F("<li><strong>ESP32-S3 Supermini - Model ESP32S3FH4R2:</strong> 32-bit RISC-V single-core CPU @ 160 MHz; 400KB SRAM, 384KB ROM; WiFi 802.11b/g/n 2.4 GHz; Bluetooth 5.0</li>");
  html += F("<li><strong>Display:</strong> 0.96 inch IIC Serial 4pin White OLED Display Module (new version); Resolution 128x64; Driver IC SSD1315</li>");
  html += F("<li><strong>Key Switches:</strong> Gateron Jupiter Banana; Operating force 59 +/-10 gf; Pre-travel 2+/-0.6 mm; Travel distance 3.4 mm max</li>");
  html += F("<li><strong>LED:</strong> 5V WS2812B-compatible 5050 addressable RGB LED strip</li>");
  html += F("</ul></details>");
  html += F("<div class='aboutBox'><h3>About</h3><p><strong>Firmware version:</strong> ");
  html += FIRMWARE_VERSION;
  html += F("</p><p><strong>License:</strong> GNU General Public License v3.0 (GPL-3.0).</p><p>You may share, redistribute, and modify this project under the GPL-3.0 license.</p><p>Firmware by Casket Pizza.</p><p><a href='https://github.com/CasketPizza/MacroLad'>MacroLad source code</a></p><p>Uses <a href='https://github.com/FluxGarage/RoboEyes'>FluxGarage RoboEyes</a>, licensed under GPL-3.0.</p></div></section></div><div class='col'>");

  html += F("<section><h2>Display</h2><form method='POST' action='/wifi'><div class='displayRows'>");
  html += F("<div class='displayLine'><label for='t'>Button/action OLED display time</label><input id='t' name='t' type='number' min='250' max='30000' step='50' value='");
  html += String(oledReturnMs);
  html += F("'><div class='check'><input type='checkbox' id='showpress' name='showpress'");
  if (showPressedOnOled) html += F(" checked");
  html += F("><label for='showpress'>Show button/action when a key is pushed</label></div></div>");
  html += F("<div class='displayLine'><label for='at'>IP address idle screen time</label><input id='at' name='at' type='number' min='1000' max='60000' step='100' value='");
  html += String(addressScreenMs);
  html += F("'><div class='check'><input type='checkbox' id='addrOn' name='addrOn'");
  if (addressScreenEnabled) html += F(" checked");
  html += F("><label for='addrOn'>Show IP address in idle cycle</label></div></div>");
  html += F("<div class='displayLine'><label for='ct'>Clock idle screen time</label><input id='ct' name='ct' type='number' min='1000' max='60000' step='100' value='");
  html += String(clockScreenMs);
  html += F("'><div class='check'><input type='checkbox' id='clockOn' name='clockOn'");
  if (clockScreenEnabled) html += F(" checked");
  html += F("><label for='clockOn'>Show clock in idle cycle</label></div></div>");
  html += F("<div class='displayLine'><span></span><span></span><a class='btn' href='/clock'>Clock setup</a></div>");
  html += F("<div class='displayLine'><label for='et'>RoboEyes idle screen time</label><input id='et' name='et' type='number' min='1000' max='60000' step='100' value='");
  html += String(eyeScreenMs);
  html += F("'><div class='check'><input type='checkbox' id='eyeOn' name='eyeOn'");
  if (eyeScreenEnabled) html += F(" checked");
  html += F("><label for='eyeOn'>Show RoboEyes in idle cycle</label></div></div></div>");
  html += F("<div class='grid displayGrid'><div><label>RoboEyes personality</label><select name='r'>");
  appendDisplayChoices(html, EYE_PERSONALITIES, sizeof(EYE_PERSONALITIES) / sizeof(EYE_PERSONALITIES[0]), eyePersonality);
  html += F("</select></div><div><label>Screen brightness</label><input name='ob' type='range' min='1' max='255' value='");
  html += String(oledBrightness);
  html += F("'></div><div class='check'><input type='checkbox' id='screenon' name='screenon'");
  if (screenEnabled) html += F(" checked");
  html += F("><label for='screenon'>Screen on</label></div><div class='check'><input type='checkbox' id='eyeson' name='eyeson'");
  if (eyesEnabled) html += F(" checked");
  html += F("><label for='eyeson'>Eyes on. When off, idle OLED shows the IP screen.</label></div></div>");
  html += F("<p><button type='submit'>Save display settings</button></p></form></section>");

  html += F("<section><h2>WiFi</h2><form method='POST' action='/wifi'>");
  html += F("<label>Local WiFi name</label><input name='s' value='");
  html += htmlEscape(prefs.getString("ssid", ""));
  html += F("' placeholder='Your WiFi SSID'>");
  html += F("<label>Local WiFi password</label><input name='p' type='password' placeholder='Leave blank to keep saved password'>");
  html += F("<p><button type='submit'>Save WiFi settings</button></p></form>");
  html += F("<p class='muted'>The setup hotspot is open, with no password. It stays active for recovery access even while LAN WiFi is connected.</p>");
  html += F("<div class='row'><a href='/scan'>Scan networks</a><a href='/forgetwifi'>Forget local WiFi</a></div></section></div>");

  html += F("</div>");
  html += F("<script>");
  html += F("function eachNode(list,fn){for(var i=0;i<list.length;i++)fn(list[i],i);}");
  html += F("function shade(hex,scale){hex=hex.replace('#','');var n=parseInt(hex,16);var r=((n>>16)&255)*scale>>8,g=((n>>8)&255)*scale>>8,b=(n&255)*scale>>8;return '#'+(0x1000000+(r<<16)+(g<<8)+b).toString(16).slice(1).toUpperCase();}");
  html += F("eachNode(document.querySelectorAll('input[type=color]'),function(x){x.style.background=x.value;x.addEventListener('input',function(){x.style.background=x.value;document.body.style.setProperty('--accent',x.value);document.body.style.setProperty('--accent-dark',shade(x.value,128));});});");
  html += F("function boolList(a){var out=[];for(var i=0;i<a.length;i++)out.push(a[i]?1:0);return out.join(' ');}");
  html += F("function setClass(e,n,on){if(!e)return;if(e.classList){e.classList.toggle(n,on);}else{var s=' '+e.className+' ';var has=s.indexOf(' '+n+' ')>=0;if(on&&!has)e.className+=(e.className?' ':'')+n;if(!on&&has)e.className=s.replace(' '+n+' ',' ').replace(/^\\s+|\\s+$/g,'');}}");
  html += F("function poll(){var x=new XMLHttpRequest();x.onreadystatechange=function(){if(x.readyState!==4)return;var live=document.getElementById('live');if(x.status!==200){live.textContent='No response from /state';return;}try{var s=JSON.parse(x.responseText);for(var i=0;i<s.keys.length;i++){var e=document.getElementById('k'+i);setClass(e,'raw',s.raw[i]);setClass(e,'down',s.keys[i]);}live.textContent='Raw '+boolList(s.raw)+' | Pressed '+boolList(s.keys)+' | Lighting '+s.lighting+' speed '+s.speed+' | Press flash '+s.effect+' | Display '+s.display+' '+s.eyes+' | OLED '+s.oled+' | '+s.status;var up=document.getElementById('uptime');if(up)up.textContent=s.uptime;var cs=document.getElementById('clockSync');if(cs)cs.textContent=s.clockSynced?'Synced':'Waiting';}catch(e){live.textContent='No response from /state';}};x.open('GET','/state?t='+Date.now(),true);x.send();}setInterval(poll,1000);poll();");
  html += F("</script></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void statePage() {
  scanKeys();
  String json = F("{\"keys\":[");
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (i) json += ',';
    json += stableDown[i] ? F("true") : F("false");
  }
  json += F("],\"raw\":[");
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (i) json += ',';
    json += rawDown(i) ? F("true") : F("false");
  }
  json += F("],\"actions\":[");
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    if (i) json += ',';
    json += '"';
    json += jsonEscape(settings[i].action);
    json += '"';
  }
  json += F("],\"hid\":");
  json += MACROPAD_USB_HID ? F("true") : F("false");
  json += F(",\"effect\":\"");
  json += jsonEscape(pressEffect);
  json += F("\"");
  json += F(",\"lighting\":\"");
  json += jsonEscape(lightingEffect);
  json += F("\",\"speed\":");
  json += String(lightingSpeed);
  json += F(",\"addressOn\":");
  json += addressScreenEnabled ? F("true") : F("false");
  json += F(",\"clockOn\":");
  json += clockScreenEnabled ? F("true") : F("false");
  json += F(",\"eyeOn\":");
  json += eyeScreenEnabled ? F("true") : F("false");
  json += F(",\"eyes\":\"");
  json += jsonEscape(eyePersonality);
  json += F("\"");
  json += F(",\"oled\":\"");
  if (oledOk) {
    json += F("0x");
    json += String(oledAddr, HEX);
  } else {
    json += F("not found");
  }
  json += F("\",\"status\":\"");
  json += jsonEscape(statusLine);
  json += F("\",\"uptime\":\"");
  json += jsonEscape(formatUptime());
  json += F("\",\"freeHeap\":");
  json += String(ESP.getFreeHeap());
  json += F(",\"minFreeHeap\":");
  json += String(ESP.getMinFreeHeap());
  json += F(",\"clockSynced\":");
  json += clockHasSynced() ? F("true") : F("false");
  json += F("}");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

static void keyConfigPage() {
  int keyIndex = server.hasArg("k") ? server.arg("k").toInt() : -1;
  if (keyIndex < 0 || keyIndex >= KEY_COUNT) {
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }

  String html = header("Key Effects");
  html += F("<section><h2>Key ");
  html += String(keyIndex + 1);
  html += F(" Effects</h2>");
  html += F("<form method='GET' action='/savekey'><input type='hidden' name='k' value='");
  html += String(keyIndex);
  html += F("'><div class='grid'><div><label>Action</label><select name='a'>");
  appendChoices(html, settings[keyIndex].action);
  html += F("</select></div><div><label>Keypress LED</label><select name='ke'>");
  appendEffectChoices(html, settings[keyIndex].ledEffect);
  html += F("</select></div></div><div class='check'><input type='checkbox' id='useEyesKey' name='u'");
  if (settings[keyIndex].useEyes) html += F(" checked");
  html += F("><label for='useEyesKey'>Use eyes for this key</label></div><div class='field'><label>Eye animation</label><select name='ea'>");
  appendDisplayChoices(html, KEY_EYE_ANIMS, sizeof(KEY_EYE_ANIMS) / sizeof(KEY_EYE_ANIMS[0]), settings[keyIndex].eyeAnim);
  html += F("</select></div><div class='eyeField'>");
  appendEyePositionGrid(html, keyIndex, settings[keyIndex].eyePos);
  html += F("</div>");
  html += F("<p class='row'><button type='submit'>Save effects</button><a class='btn' href='/#k");
  html += String(keyIndex);
  html += F("'>Cancel</a></p></form></section></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void longPressPage() {
  int keyIndex = server.hasArg("k") ? server.arg("k").toInt() : -1;
  if (keyIndex < 0 || keyIndex >= KEY_COUNT) {
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }

  String html = header("Long Press Action");
  html += F("<section><h2>Key ");
  html += String(keyIndex + 1);
  html += F(" Long Press Action</h2>");
  if (keyIndex == 9) {
    html += F("<p class='muted'>Key 10 long press action is reserved for the OLED menu.</p>");
    html += F("<p><a class='btn' href='/#k9'>Cancel</a></p></section></main></body></html>");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html", html);
    return;
  }
  html += F("<form method='GET' action='/savelong'><input type='hidden' name='k' value='");
  html += String(keyIndex);
  html += F("'><div class='grid'><div><label>Long press action</label><select name='la'>");
  appendChoices(html, settings[keyIndex].longAction);
  html += F("</select></div><div><label>Long press time, milliseconds</label><input name='lm' type='number' min='100' max='10000' step='50' value='");
  html += String(settings[keyIndex].longPressMs);
  html += F("'></div></div><p class='muted'>Disabled means this key keeps firing its normal action immediately on press.</p>");
  html += F("<p class='row'><button type='submit'>Save long press action</button><a class='btn' href='/#k");
  html += String(keyIndex);
  html += F("'>Cancel</a></p></form></section></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void saveKeyPage() {
  int keyIndex = server.hasArg("k") ? server.arg("k").toInt() : -1;
  if (keyIndex >= 0 && keyIndex < KEY_COUNT) {
    if (server.hasArg("a")) {
      settings[keyIndex].action = server.arg("a");
      settings[keyIndex].action.trim();
      if (!validAction(settings[keyIndex].action)) settings[keyIndex].action = defaultActionFor(keyIndex);
    }
    settings[keyIndex].useEyes = server.hasArg("u");
    if (server.hasArg("ea")) {
      settings[keyIndex].eyeAnim = server.arg("ea");
      settings[keyIndex].eyeAnim.trim();
      if (!validKeyEyeAnim(settings[keyIndex].eyeAnim)) settings[keyIndex].eyeAnim = "BLINK";
    }
    String posArg = "ep" + String(keyIndex);
    if (server.hasArg(posArg)) {
      settings[keyIndex].eyePos = server.arg(posArg);
      settings[keyIndex].eyePos.trim();
      if (!validEyePosition(settings[keyIndex].eyePos)) settings[keyIndex].eyePos = "DEFAULT";
    }
    if (server.hasArg("ke")) {
      settings[keyIndex].ledEffect = server.arg("ke");
      settings[keyIndex].ledEffect.trim();
      if (!validPressEffect(settings[keyIndex].ledEffect)) settings[keyIndex].ledEffect = pressEffect;
    }
    saveSetting(keyIndex);
    Serial.printf("Saved key %u action=%s longAction=%s longMs=%u useEyes=%u eyeAnim=%s eyePos=%s ledEffect=%s\n", keyIndex + 1, settings[keyIndex].action.c_str(), settings[keyIndex].longAction.c_str(), settings[keyIndex].longPressMs, settings[keyIndex].useEyes ? 1 : 0, settings[keyIndex].eyeAnim.c_str(), settings[keyIndex].eyePos.c_str(), settings[keyIndex].ledEffect.c_str());
    oled("Saved key settings", "Key " + String(keyIndex + 1));
    oledReturnAt = millis() + oledReturnMs;
  }
  server.sendHeader("Location", "/#k" + String(keyIndex >= 0 && keyIndex < KEY_COUNT ? keyIndex : 0));
  server.send(303);
}

static void saveLongPressPage() {
  int keyIndex = server.hasArg("k") ? server.arg("k").toInt() : -1;
  if (keyIndex >= 0 && keyIndex < KEY_COUNT && keyIndex != 9) {
    if (server.hasArg("la")) {
      settings[keyIndex].longAction = server.arg("la");
      settings[keyIndex].longAction.trim();
      if (!validAction(settings[keyIndex].longAction)) settings[keyIndex].longAction = "";
    }
    if (server.hasArg("lm")) {
      int longMs = server.arg("lm").toInt();
      if (longMs < 100) longMs = 100;
      if (longMs > 10000) longMs = 10000;
      settings[keyIndex].longPressMs = (uint16_t)longMs;
    }
    saveSetting(keyIndex);
    Serial.printf("Saved key %u longAction=%s longMs=%u\n", keyIndex + 1, settings[keyIndex].longAction.c_str(), settings[keyIndex].longPressMs);
    oled("Saved long press", "Key " + String(keyIndex + 1));
    oledReturnAt = millis() + oledReturnMs;
  }
  server.sendHeader("Location", "/#k" + String(keyIndex >= 0 && keyIndex < KEY_COUNT ? keyIndex : 0));
  server.send(303);
}

static void clockPage() {
  String html = header("Clock Setup");
  html += F("<section><h2>Clock Setup</h2><form method='POST' action='/clock'>");
  html += F("<div class='grid'><div><label>Time zone</label><select name='tz'>");
  appendDisplayChoices(html, TIME_ZONES, sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]), clockTimeZone);
  html += F("</select></div><div><label>Clock format</label><select name='fmt'><option value='24'");
  if (clockUse24Hour) html += F(" selected");
  html += F(">24 hour</option><option value='12'");
  if (!clockUse24Hour) html += F(" selected");
  html += F(">12 hour</option></select></div><div><label>AM/PM position</label><select name='ampmpos'>");
  appendDisplayChoices(html, AMPM_POSITIONS, sizeof(AMPM_POSITIONS) / sizeof(AMPM_POSITIONS[0]), clockAmPmAfter ? "AFTER" : "BEFORE");
  html += F("</select></div><div><label>Date format</label><select name='datefmt'>");
  appendDisplayChoices(html, DATE_FORMATS, sizeof(DATE_FORMATS) / sizeof(DATE_FORMATS[0]), clockDateFormat);
  html += F("</select></div><div><label>Clock font</label><select name='font'>");
  appendClockFontChoices(html, clockFont);
  html += F("</select></div></div>");
  html += F("<div class='grid'><div class='check'><input type='checkbox' id='showampm' name='showampm'");
  if (clockShowAmPm) html += F(" checked");
  html += F("><label for='showampm'>Show AM/PM on 12-hour clock</label></div><div class='check'><input type='checkbox' id='showtz' name='showtz'");
  if (clockShowTimeZone) html += F(" checked");
  html += F("><label for='showtz'>Show time zone</label></div><div class='check'><input type='checkbox' id='showdate' name='showdate'");
  if (clockShowDate) html += F(" checked");
  html += F("><label for='showdate'>Show date</label></div></div>");
  html += F("<p class='muted'>Clock time syncs from NTP over LAN/WiFi. The selected time zone handles daylight saving automatically where the rule supports it.</p>");
  html += F("<p class='row'><button type='submit'>Save clock setup</button><a class='btn' href='/'>Cancel</a></p></form></section></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void saveClockPage() {
  if (server.hasArg("tz")) {
    clockTimeZone = server.arg("tz");
    clockTimeZone.trim();
    if (!validTimeZone(clockTimeZone)) clockTimeZone = "UTC0";
    prefs.putString("clocktz", clockTimeZone);
  }
  if (server.hasArg("fmt")) {
    clockUse24Hour = server.arg("fmt") != "12";
    prefs.putBool("clock24", clockUse24Hour);
  }
  clockShowAmPm = server.hasArg("showampm");
  prefs.putBool("clockampm", clockShowAmPm);
  if (server.hasArg("ampmpos")) {
    clockAmPmAfter = server.arg("ampmpos") != "BEFORE";
    prefs.putBool("clockampmafter", clockAmPmAfter);
  }
  clockShowTimeZone = server.hasArg("showtz");
  prefs.putBool("clockshowtz", clockShowTimeZone);
  clockShowDate = server.hasArg("showdate");
  prefs.putBool("clockshowdate", clockShowDate);
  if (server.hasArg("datefmt")) {
    clockDateFormat = server.arg("datefmt");
    clockDateFormat.trim();
    if (!validDateFormat(clockDateFormat)) clockDateFormat = "DOW_D_MON";
    prefs.putString("clockdatefmt", clockDateFormat);
  }
  if (server.hasArg("font")) {
    clockFont = server.arg("font");
    clockFont.trim();
    if (!validClockFont(clockFont)) clockFont = "DEFAULT";
    prefs.putString("clockfont", clockFont);
  }
  configureClockTime();
  oled("Clock saved", timeZoneLabel(clockTimeZone));
  oledReturnAt = millis() + oledReturnMs;
  server.sendHeader("Location", "/");
  server.send(303);
}

static void savePage() {
  Serial.printf("Save request with %u args\n", server.args());
  if (server.hasArg("e")) {
    pressEffect = server.arg("e");
    pressEffect.trim();
    if (!validPressEffect(pressEffect)) {
      pressEffect = "AMBIENT_PULSE";
    }
    Serial.printf("Saved backlight keypress effect=%s\n", pressEffect.c_str());
  }
  if (server.hasArg("lc")) {
    ambientColor = parseColor(server.arg("lc"), ambientColor);
    Serial.printf("Saved backlight color=%s\n", colorHex(ambientColor).c_str());
  }
  if (server.hasArg("le")) {
    lightingEffect = server.arg("le");
    lightingEffect.trim();
    if (lightingEffect != "SOLID" && lightingEffect != "BREATH" && lightingEffect != "RAINBOW" && lightingEffect != "CHASE" && lightingEffect != "SPARKLE") {
      lightingEffect = "SOLID";
    }
    Serial.printf("Saved ambient lighting effect=%s\n", lightingEffect.c_str());
  }
  if (server.hasArg("ls")) {
    int speed = server.arg("ls").toInt();
    if (speed < 1) speed = 1;
    if (speed > 10) speed = 10;
    lightingSpeed = (uint8_t)speed;
    Serial.printf("Saved ambient lighting speed=%u\n", lightingSpeed);
  }
  if (server.hasArg("lb")) {
    int brightness = server.arg("lb").toInt();
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    ledBrightness = (uint8_t)brightness;
    Serial.printf("Saved LED brightness=%u\n", ledBrightness);
  }
  if (server.hasArg("e") || server.hasArg("lc") || server.hasArg("le") || server.hasArg("ls") || server.hasArg("lb")) {
    saveLightingSettings();
  }
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    String a = "a" + String(i);
    if (server.hasArg(a)) {
      settings[i].action = server.arg(a);
      settings[i].action.trim();
      if (!validAction(settings[i].action)) {
        settings[i].action = defaultActionFor(i);
      }
      String longActionArg = "la" + String(i);
      if (server.hasArg(longActionArg)) {
        settings[i].longAction = server.arg(longActionArg);
        settings[i].longAction.trim();
        if (!validAction(settings[i].longAction)) settings[i].longAction = "";
      }
      String animArg = "ea" + String(i);
      if (server.hasArg(animArg)) {
        settings[i].eyeAnim = server.arg(animArg);
        settings[i].eyeAnim.trim();
        if (!validKeyEyeAnim(settings[i].eyeAnim)) settings[i].eyeAnim = "BLINK";
      }
      String posArg = "ep" + String(i);
      if (server.hasArg(posArg)) {
        settings[i].eyePos = server.arg(posArg);
        settings[i].eyePos.trim();
        if (!validEyePosition(settings[i].eyePos)) settings[i].eyePos = "DEFAULT";
      }
      String keyEffectArg = "ke" + String(i);
      if (server.hasArg(keyEffectArg)) {
        settings[i].ledEffect = server.arg(keyEffectArg);
        settings[i].ledEffect.trim();
        if (!validPressEffect(settings[i].ledEffect)) settings[i].ledEffect = pressEffect;
      }
      saveSetting(i);
      Serial.printf("Saved key %u action=%s longAction=%s longMs=%u useEyes=%u eyeAnim=%s eyePos=%s ledEffect=%s\n", i + 1, settings[i].action.c_str(), settings[i].longAction.c_str(), settings[i].longPressMs, settings[i].useEyes ? 1 : 0, settings[i].eyeAnim.c_str(), settings[i].eyePos.c_str(), settings[i].ledEffect.c_str());
    }
  }
  applyLeds();
  oled("Saved settings", wifiText());
  oledReturnAt = millis() + oledReturnMs;
  String html = header("Saved");
  html += F("<section><p class='ok'>Settings saved and applied immediately.</p><p class='muted'>Returning to the macropad page...</p><p><a href='/'>Back to macropad</a></p></section><script>setTimeout(function(){location.href='/';},1200);</script></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void addComboPage() {
  String combo = "";
  bool bad = false;
  for (uint8_t i = 0; i < 4; i++) {
    String arg = "cp" + String(i);
    String part = server.hasArg(arg) ? server.arg(arg) : "";
    part.trim();
    part.toUpperCase();
    if (!part.length()) break;
    if (!validComboPart(part)) {
      bad = true;
      break;
    }
    if (combo.length()) combo += "+";
    combo += part;
  }

  if (!bad && combo.length()) {
    String action = "KEY:" + combo;
    bool exists = false;
    int8_t emptySlot = -1;
    for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
      if (customCombos[i] == action) exists = true;
      if (!customCombos[i].length() && emptySlot < 0) emptySlot = i;
    }
    if (!exists && emptySlot >= 0) {
      customCombos[emptySlot] = action;
      String key = "combo" + String(emptySlot);
      prefs.putString(key.c_str(), action);
      Serial.printf("Added custom combo %s\n", action.c_str());
      oled("Added shortcut", comboLabel(action));
      oledReturnAt = millis() + oledReturnMs;
    }
  }

  server.sendHeader("Location", "/combos");
  server.send(303);
}

static void addRunCommandPage() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    command.trim();
    if (command.length() > 160) command = command.substring(0, 160);
    if (command.length()) {
      String action = "RUN:" + command;
      bool exists = false;
      int8_t emptySlot = -1;
      for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
        if (customCombos[i] == action) exists = true;
        if (!customCombos[i].length() && emptySlot < 0) emptySlot = i;
      }
      if (!exists && emptySlot >= 0) {
        customCombos[emptySlot] = action;
        String key = "combo" + String(emptySlot);
        prefs.putString(key.c_str(), action);
        Serial.printf("Added Windows run command %s\n", command.c_str());
        oled("Added run command", command.substring(0, 18));
        oledReturnAt = millis() + oledReturnMs;
      }
    }
  }
  server.sendHeader("Location", "/combos");
  server.send(303);
}

static void combosPage() {
  String html = header("Manage Key Combinations");
  html += F("<section><h2>Add shortcut</h2><form method='GET' action='/addcombo'><div class='grid'>");
  for (uint8_t i = 0; i < 4; i++) {
    html += F("<div><label>Shortcut part ");
    html += String(i + 1);
    html += F("</label><select name='cp");
    html += String(i);
    html += F("'>");
    appendComboPartChoices(html);
    html += F("</select></div>");
  }
  html += F("</div><p><button type='submit'>Add to action list</button></p></form><p class='muted'>Example: Ctrl, Alt, Delete, None adds Ctrl + Alt + Delete to the normal key action dropdowns.</p></section>");

  html += F("<section><h2>Add Windows run command</h2><form method='POST' action='/addrun'><label>Program, file, shortcut, script, or Run command</label><input name='cmd' maxlength='160' placeholder='C:\\Program Files\\App\\app.exe'><p><button type='submit'>Add to action list</button></p></form>");
  html += F("<p class='muted'>This sends Win+R, types the saved command, then presses Enter. For paths with spaces, include quotes. Example: &quot;C:\\Program Files\\App\\app.exe&quot;</p>");
  html += F("<p class='muted'>Browser security prevents a real browse button from giving MacroLad the full local path, so paste the path or command here.</p></section>");

  html += F("<section><h2>Saved actions</h2>");
  bool hasCustom = false;
  for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
    if (!customCombos[i].length()) continue;
    hasCustom = true;
    html += F("<form class='row' method='GET' action='/removecombo'><input type='hidden' name='i' value='");
    html += String(i);
    html += F("'><span>");
    html += htmlEscape(comboLabel(customCombos[i]));
    html += F("</span><button type='submit'>Remove</button></form>");
  }
  if (!hasCustom) html += F("<p class='muted'>No custom key combinations saved yet.</p>");
  html += F("<p><a class='btn' href='/'>Back to MacroLad</a></p></section></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void removeComboPage() {
  if (server.hasArg("i")) {
    int idx = server.arg("i").toInt();
    if (idx >= 0 && idx < CUSTOM_COMBO_COUNT && customCombos[idx].length()) {
      String removed = customCombos[idx];
      customCombos[idx] = "";
      prefs.remove(("combo" + String(idx)).c_str());
      for (uint8_t i = 0; i < KEY_COUNT; i++) {
        if (settings[i].action == removed) {
          settings[i].action = defaultActionFor(i);
          saveSetting(i);
        }
        if (settings[i].longAction == removed) {
          settings[i].longAction = "";
          saveSetting(i);
        }
      }
      Serial.printf("Removed custom combo %s\n", removed.c_str());
      oled("Removed shortcut", comboLabel(removed));
      oledReturnAt = millis() + oledReturnMs;
    }
  }
  server.sendHeader("Location", "/combos");
  server.send(303);
}

static void copyProfilePrefs(uint8_t from, uint8_t to) {
  String src = profilePrefix(from);
  String dst = profilePrefix(to);
  prefs.putString((dst + "effect").c_str(), prefs.getString((src + "effect").c_str(), "AMBIENT_PULSE"));
  prefs.putUInt((dst + "ambient").c_str(), prefs.getUInt((src + "ambient").c_str(), 0x2040FF));
  prefs.putString((dst + "lightfx").c_str(), prefs.getString((src + "lightfx").c_str(), "SOLID"));
  prefs.putUChar((dst + "lightspeed").c_str(), prefs.getUChar((src + "lightspeed").c_str(), 5));
  prefs.putUChar((dst + "ledbright").c_str(), prefs.getUChar((src + "ledbright").c_str(), 80));
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    prefs.putString((dst + "a" + String(i)).c_str(), prefs.getString((src + "a" + String(i)).c_str(), defaultActionFor(i)));
    prefs.putString((dst + "la" + String(i)).c_str(), prefs.getString((src + "la" + String(i)).c_str(), ""));
    prefs.putUShort((dst + "lm" + String(i)).c_str(), prefs.getUShort((src + "lm" + String(i)).c_str(), 800));
    prefs.putBool((dst + "ue" + String(i)).c_str(), prefs.getBool((src + "ue" + String(i)).c_str(), false));
    prefs.putString((dst + "ea" + String(i)).c_str(), prefs.getString((src + "ea" + String(i)).c_str(), "BLINK"));
    prefs.putString((dst + "ep" + String(i)).c_str(), prefs.getString((src + "ep" + String(i)).c_str(), "DEFAULT"));
    prefs.putString((dst + "ke" + String(i)).c_str(), prefs.getString((src + "ke" + String(i)).c_str(), pressEffect));
  }
}

static void removeProfilePrefs(uint8_t index) {
  String pfx = profilePrefix(index);
  prefs.remove((pfx + "effect").c_str());
  prefs.remove((pfx + "ambient").c_str());
  prefs.remove((pfx + "lightfx").c_str());
  prefs.remove((pfx + "lightspeed").c_str());
  prefs.remove((pfx + "ledbright").c_str());
  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    prefs.remove((pfx + "a" + String(i)).c_str());
    prefs.remove((pfx + "la" + String(i)).c_str());
    prefs.remove((pfx + "lm" + String(i)).c_str());
    prefs.remove((pfx + "ue" + String(i)).c_str());
    prefs.remove((pfx + "ea" + String(i)).c_str());
    prefs.remove((pfx + "ep" + String(i)).c_str());
    prefs.remove((pfx + "ke" + String(i)).c_str());
  }
}

static String exportProfilesText() {
  String out = F("MacroLadProfilesV1\n");
  out += "count=" + String(profileCount) + "\n";
  out += "active=" + String(activeProfile) + "\n";
  for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
    if (customCombos[i].length()) out += "combo" + String(i) + "=" + customCombos[i] + "\n";
  }
  for (uint8_t p = 0; p < profileCount; p++) {
    String pfx = profilePrefix(p);
    out += "name" + String(p) + "=" + profileName(p) + "\n";
    out += "effect" + String(p) + "=" + prefs.getString((pfx + "effect").c_str(), pressEffect) + "\n";
    out += "ambient" + String(p) + "=" + colorHex(prefs.getUInt((pfx + "ambient").c_str(), ambientColor)) + "\n";
    out += "lightfx" + String(p) + "=" + prefs.getString((pfx + "lightfx").c_str(), lightingEffect) + "\n";
    out += "lightspeed" + String(p) + "=" + String(prefs.getUChar((pfx + "lightspeed").c_str(), lightingSpeed)) + "\n";
    out += "ledbright" + String(p) + "=" + String(prefs.getUChar((pfx + "ledbright").c_str(), ledBrightness)) + "\n";
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
      out += "p" + String(p) + "a" + String(i) + "=" + prefs.getString((pfx + "a" + String(i)).c_str(), defaultActionFor(i)) + "\n";
      out += "p" + String(p) + "la" + String(i) + "=" + prefs.getString((pfx + "la" + String(i)).c_str(), "") + "\n";
      out += "p" + String(p) + "lm" + String(i) + "=" + String(prefs.getUShort((pfx + "lm" + String(i)).c_str(), 800)) + "\n";
      out += "p" + String(p) + "ue" + String(i) + "=" + String(prefs.getBool((pfx + "ue" + String(i)).c_str(), false) ? 1 : 0) + "\n";
      out += "p" + String(p) + "ea" + String(i) + "=" + prefs.getString((pfx + "ea" + String(i)).c_str(), "BLINK") + "\n";
      out += "p" + String(p) + "ep" + String(i) + "=" + prefs.getString((pfx + "ep" + String(i)).c_str(), "DEFAULT") + "\n";
      out += "p" + String(p) + "ke" + String(i) + "=" + prefs.getString((pfx + "ke" + String(i)).c_str(), pressEffect) + "\n";
    }
  }
  return out;
}

static void profileSelectPage() {
  if (server.hasArg("p")) switchProfile((uint8_t)server.arg("p").toInt(), true);
  server.sendHeader("Location", "/");
  server.send(303);
}

static void profilesPage() {
  String html = header("Manage Profiles");
  html += F("<section><h2>Profiles</h2>");
  for (uint8_t i = 0; i < profileCount; i++) {
    html += F("<form class='row' method='POST' action='/profiles/rename'><input type='hidden' name='p' value='");
    html += String(i);
    html += F("'><input name='n' value='");
    html += htmlEscape(profileName(i));
    html += F("'><button type='submit'>Rename</button></form>");
  }
  html += F("<div class='row'><form method='POST' action='/profiles/add'><button type='submit'");
  if (profileCount >= MAX_PROFILES) html += F(" disabled");
  html += F(">Add profile</button></form><form method='POST' action='/profiles/delete'><select name='p'>");
  for (uint8_t i = 0; i < profileCount; i++) {
    html += F("<option value='");
    html += String(i);
    html += F("'>");
    html += htmlEscape(profileName(i));
    html += F("</option>");
  }
  html += F("</select><button type='submit'");
  if (profileCount <= 1) html += F(" disabled");
  html += F(">Delete selected</button></form><form method='POST' action='/profiles/copy'><select name='p'>");
  for (uint8_t i = 0; i < profileCount; i++) {
    html += F("<option value='");
    html += String(i);
    html += F("'");
    if (i == activeProfile) html += F(" selected");
    html += F(">");
    html += htmlEscape(profileName(i));
    html += F("</option>");
  }
  html += F("</select><button type='submit'");
  if (profileCount >= MAX_PROFILES) html += F(" disabled");
  html += F(">Copy selected</button></form></div><p class='muted'>At least one profile is always kept. Copy creates a new profile with the selected profile's keys, long press actions, effects, LED colour, and brightness.</p></section>");
  html += F("<section><h2>Backup</h2><p class='row'><a class='btn' href='/profiles/export'>Export profiles</a></p>");
  html += F("<form method='POST' action='/profiles/import'><label>Import profile backup</label><textarea name='data' rows='10' style='box-sizing:border-box;width:100%;font:inherit;border-radius:6px;border:1px solid #43505c;background:#0d1115;color:#edf2f7;padding:8px'></textarea><p><button type='submit'>Import profiles</button></p></form>");
  html += F("<p><a class='btn' href='/'>Back to MacroLad</a></p></section></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void addProfilePage() {
  if (profileCount < MAX_PROFILES) {
    uint8_t newIndex = profileCount++;
    profileNames[newIndex] = "Profile " + String(newIndex + 1);
    copyProfilePrefs(activeProfile, newIndex);
    saveProfileMeta();
  }
  server.sendHeader("Location", "/profiles");
  server.send(303);
}

static void copyProfilePage() {
  if (profileCount < MAX_PROFILES && server.hasArg("p")) {
    uint8_t src = (uint8_t)server.arg("p").toInt();
    if (src < profileCount) {
      uint8_t newIndex = profileCount++;
      profileNames[newIndex] = profileName(src) + " copy";
      if (profileNames[newIndex].length() > 24) profileNames[newIndex] = profileNames[newIndex].substring(0, 24);
      copyProfilePrefs(src, newIndex);
      activeProfile = newIndex;
      saveProfileMeta();
      switchProfile(activeProfile, false);
    }
  }
  server.sendHeader("Location", "/profiles");
  server.send(303);
}

static void renameProfilePage() {
  if (server.hasArg("p") && server.hasArg("n")) {
    uint8_t idx = (uint8_t)server.arg("p").toInt();
    String name = server.arg("n");
    name.trim();
    if (idx < profileCount && name.length()) {
      if (name.length() > 24) name = name.substring(0, 24);
      profileNames[idx] = name;
      saveProfileMeta();
    }
  }
  server.sendHeader("Location", "/profiles");
  server.send(303);
}

static void deleteProfilePage() {
  if (profileCount > 1 && server.hasArg("p")) {
    uint8_t idx = (uint8_t)server.arg("p").toInt();
    if (idx < profileCount) {
      for (uint8_t i = idx; i + 1 < profileCount; i++) {
        profileNames[i] = profileNames[i + 1];
        copyProfilePrefs(i + 1, i);
      }
      profileCount--;
      removeProfilePrefs(profileCount);
      if (activeProfile >= profileCount) activeProfile = profileCount - 1;
      saveProfileMeta();
      switchProfile(activeProfile, false);
    }
  }
  server.sendHeader("Location", "/profiles");
  server.send(303);
}

static void exportProfilesPage() {
  server.sendHeader("Content-Disposition", "attachment; filename=macrolad_profiles.txt");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/plain", exportProfilesText());
}

static void importProfilesPage() {
  if (server.hasArg("data")) {
    String data = server.arg("data");
    uint8_t newCount = profileCount;
    for (uint8_t i = 0; i < CUSTOM_COMBO_COUNT; i++) {
      customCombos[i] = "";
      prefs.remove(("combo" + String(i)).c_str());
    }
    int start = 0;
    while (start < data.length()) {
      int end = data.indexOf('\n', start);
      if (end < 0) end = data.length();
      String line = data.substring(start, end);
      line.trim();
      int eq = line.indexOf('=');
      if (eq > 0) {
        String key = line.substring(0, eq);
        String value = line.substring(eq + 1);
        value.trim();
        if (key == "count") {
          int c = value.toInt();
          if (c >= 1 && c <= MAX_PROFILES) newCount = (uint8_t)c;
        } else if (key == "active") {
          activeProfile = (uint8_t)value.toInt();
        } else if (key.startsWith("name")) {
          uint8_t p = (uint8_t)key.substring(4).toInt();
          if (p < MAX_PROFILES && value.length()) profileNames[p] = value.substring(0, min((int)value.length(), 24));
        } else if (key.startsWith("combo")) {
          uint8_t comboIndex = (uint8_t)key.substring(5).toInt();
          value.trim();
          if (value.startsWith("KEY:")) value.toUpperCase();
          if (comboIndex < CUSTOM_COMBO_COUNT && ((value.startsWith("KEY:") && value.length() > 4) || (value.startsWith("RUN:") && value.length() > 4 && value.length() <= 180))) {
            customCombos[comboIndex] = value;
            prefs.putString(("combo" + String(comboIndex)).c_str(), value);
          }
        }
      }
      start = end + 1;
    }
    profileCount = newCount < 1 ? 1 : newCount;
    if (profileCount > MAX_PROFILES) profileCount = MAX_PROFILES;
    if (activeProfile >= profileCount) activeProfile = 0;
    saveProfileMeta();
    // Import full profile key/value data after metadata is valid.
    start = 0;
    while (start < data.length()) {
      int end = data.indexOf('\n', start);
      if (end < 0) end = data.length();
      String line = data.substring(start, end);
      line.trim();
      int eq = line.indexOf('=');
      if (eq > 0) {
        String key = line.substring(0, eq);
        String value = line.substring(eq + 1);
        for (uint8_t p = 0; p < profileCount; p++) {
          String pfx = profilePrefix(p);
          String ps = String(p);
          if (key == "effect" + ps && validPressEffect(value)) prefs.putString((pfx + "effect").c_str(), value);
          else if (key == "ambient" + ps) prefs.putUInt((pfx + "ambient").c_str(), parseColor(value, 0x2040FF));
          else if (key == "lightfx" + ps && validLightingEffect(value)) prefs.putString((pfx + "lightfx").c_str(), value);
          else if (key == "lightspeed" + ps) prefs.putUChar((pfx + "lightspeed").c_str(), constrain(value.toInt(), 1, 10));
          else if (key == "ledbright" + ps) prefs.putUChar((pfx + "ledbright").c_str(), constrain(value.toInt(), 0, 255));
          for (uint8_t i = 0; i < KEY_COUNT; i++) {
            String base = "p" + ps;
            if (key == base + "a" + String(i) && validAction(value)) prefs.putString((pfx + "a" + String(i)).c_str(), value);
            else if (key == base + "la" + String(i) && validAction(value)) prefs.putString((pfx + "la" + String(i)).c_str(), value);
            else if (key == base + "lm" + String(i)) prefs.putUShort((pfx + "lm" + String(i)).c_str(), constrain(value.toInt(), 100, 10000));
            else if (key == base + "ue" + String(i)) prefs.putBool((pfx + "ue" + String(i)).c_str(), value.toInt() != 0);
            else if (key == base + "ea" + String(i) && validKeyEyeAnim(value)) prefs.putString((pfx + "ea" + String(i)).c_str(), value);
            else if (key == base + "ep" + String(i) && validEyePosition(value)) prefs.putString((pfx + "ep" + String(i)).c_str(), value);
            else if (key == base + "ke" + String(i) && validPressEffect(value)) prefs.putString((pfx + "ke" + String(i)).c_str(), value);
          }
        }
      }
      start = end + 1;
    }
    switchProfile(activeProfile, false);
  }
  server.sendHeader("Location", "/profiles");
  server.send(303);
}

static void resetPage() {
  if (!server.hasArg("confirm")) {
    String html = header("Factory Reset");
    html += F("<section><h2>Factory reset MacroLad?</h2><p class='warn'>This clears profiles, key settings, custom combinations, lighting, display, and saved WiFi.</p><p class='row'><a class='btn' href='/reset?confirm=1'>Factory reset</a><a class='btn' href='/'>Cancel</a></p></section></main></body></html>");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html", html);
    return;
  }
  prefs.clear();
  loadSettings();
  configureClockTime();
  applyOledDisplaySettings(true);
  applyLeds();
  oled("Factory reset");
  oledReturnAt = millis() + oledReturnMs;
  server.sendHeader("Location", "/");
  server.send(303);
}

static void scanPage() {
  oled("Scanning WiFi");
  oledReturnAt = millis() + oledReturnMs;
  int n = WiFi.scanNetworks();
  String html = header("WiFi");
  html += F("<section><form method='POST' action='/wifi'><label>Network</label><select name='s'>");
  for (int i = 0; i < n; i++) {
    html += F("<option value='");
    html += htmlEscape(WiFi.SSID(i));
    html += F("'>");
    html += htmlEscape(WiFi.SSID(i));
    html += F("</option>");
  }
  html += F("</select><label>Password</label><input name='p' type='password' placeholder='Leave blank to keep saved password'>");
  html += F("<label>Button OLED display time, milliseconds</label><input name='t' type='number' min='250' max='30000' step='50' value='");
  html += String(oledReturnMs);
  html += F("'><p><button>Save and connect</button></p></form><p><a href='/'>Back</a></p></section></main></body></html>");
  server.send(200, "text/html", html);
}

static void wifiPage() {
  String ssid = server.arg("s");
  String pass = server.arg("p");
  ssid.trim();
  if (server.hasArg("t")) {
    uint32_t ms = (uint32_t)server.arg("t").toInt();
    if (ms < 250) ms = 250;
    if (ms > 30000) ms = 30000;
    oledReturnMs = (uint16_t)ms;
    prefs.putUShort("oledms", oledReturnMs);
  }
  if (server.hasArg("r")) {
    eyePersonality = server.arg("r");
    eyePersonality.trim();
    if (eyePersonality == "SHAKY") eyePersonality = "PLAYFUL";
    if (!validEyePersonality(eyePersonality)) eyePersonality = "PLAYFUL";
    prefs.putString("eyes", eyePersonality);
  }
  if (server.hasArg("at")) {
    uint32_t ms = (uint32_t)server.arg("at").toInt();
    if (ms < 1000) ms = 1000;
    if (ms > 60000) ms = 60000;
    addressScreenMs = (uint16_t)ms;
    prefs.putUShort("addrms", addressScreenMs);
  }
  if (server.hasArg("ct")) {
    uint32_t ms = (uint32_t)server.arg("ct").toInt();
    if (ms < 1000) ms = 1000;
    if (ms > 60000) ms = 60000;
    clockScreenMs = (uint16_t)ms;
    prefs.putUShort("clockms", clockScreenMs);
  }
  if (server.hasArg("et")) {
    uint32_t ms = (uint32_t)server.arg("et").toInt();
    if (ms < 1000) ms = 1000;
    if (ms > 60000) ms = 60000;
    eyeScreenMs = (uint16_t)ms;
    prefs.putUShort("eyems", eyeScreenMs);
  }
  if (server.hasArg("ob")) {
    int brightness = server.arg("ob").toInt();
    if (brightness < 1) brightness = 1;
    if (brightness > 255) brightness = 255;
    oledBrightness = (uint8_t)brightness;
    prefs.putUChar("oledbright", oledBrightness);
  }
  if (server.hasArg("r") || server.hasArg("at") || server.hasArg("ct") || server.hasArg("et") || server.hasArg("ob") || server.hasArg("screenon") || server.hasArg("addrOn") || server.hasArg("clockOn") || server.hasArg("eyeOn") || server.hasArg("eyeson") || server.hasArg("showpress")) {
    screenEnabled = server.hasArg("screenon");
    addressScreenEnabled = server.hasArg("addrOn");
    clockScreenEnabled = server.hasArg("clockOn");
    eyeScreenEnabled = server.hasArg("eyeOn");
    eyesEnabled = server.hasArg("eyeson");
    showPressedOnOled = server.hasArg("showpress");
    prefs.putBool("screenon", screenEnabled);
    prefs.putBool("addrOn", addressScreenEnabled);
    prefs.putBool("clockOn", clockScreenEnabled);
    prefs.putBool("eyeOn", eyeScreenEnabled);
    prefs.putBool("eyeson", eyesEnabled);
    prefs.putBool("showpress", showPressedOnOled);
    applyOledDisplaySettings();
  }
  if (ssid.length()) {
    prefs.putString("ssid", ssid);
    if (pass.length()) prefs.putString("pass", pass);
    oled("Connecting WiFi", ssid);
    WiFi.disconnect(false, false);
    WiFi.begin(ssid.c_str(), prefs.getString("pass", "").c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
      delay(100);
    }
    configureClockTime();
  }
  showIdleDisplay();
  String html = header("WiFi Saved");
  html += F("<section><p class='ok'>Settings saved.</p><p>LAN address: <strong>");
  html += (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("not connected"));
  html += F("</strong></p><p>Setup hotspot: <strong>");
  html += String(AP_SSID) + " " + WiFi.softAPIP().toString() + " (open)";
  html += F("</strong></p><p class='muted'>Returning to the macropad page...</p><p><a href='/'>Back to macropad</a></p></section><script>setTimeout(function(){location.href='/';},1200);</script></main></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void forgetWifiPage() {
  prefs.remove("ssid");
  prefs.remove("pass");
  WiFi.disconnect(false, true);
  startSetupHotspot();
  showAddressScreen();
  server.sendHeader("Location", "/");
  server.send(303);
}

static void usbUploadModePage() {
  usbUploadModePending = true;
  restartAt = millis() + 900;
  oled("USB upload mode");

  String html = header("USB Upload Mode");
  html += F("<section><p class='ok'>Restarting into USB upload mode.</p>");
  html += F("<p class='muted'>When the USB port comes back, upload from Arduino IDE as normal. You do not need to press BOOT.</p></section>");
  html += F("</main></body></html>");
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", html);
}

static void startWifi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  startSetupHotspot();
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  if (ssid.length()) {
    oled("Connecting WiFi", ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 6000) delay(100);
  }
  lastLanConnected = WiFi.status() == WL_CONNECTED;
  configureClockTime();
}

static void startServer() {
  server.on("/", HTTP_GET, rootPage);
  server.on("/state", HTTP_GET, statePage);
  server.on("/key", HTTP_GET, keyConfigPage);
  server.on("/savekey", HTTP_GET, saveKeyPage);
  server.on("/long", HTTP_GET, longPressPage);
  server.on("/savelong", HTTP_GET, saveLongPressPage);
  server.on("/clock", HTTP_GET, clockPage);
  server.on("/clock", HTTP_POST, saveClockPage);
  server.on("/profile", HTTP_GET, profileSelectPage);
  server.on("/profiles", HTTP_GET, profilesPage);
  server.on("/profiles/add", HTTP_POST, addProfilePage);
  server.on("/profiles/copy", HTTP_POST, copyProfilePage);
  server.on("/profiles/rename", HTTP_POST, renameProfilePage);
  server.on("/profiles/delete", HTTP_POST, deleteProfilePage);
  server.on("/profiles/export", HTTP_GET, exportProfilesPage);
  server.on("/profiles/import", HTTP_POST, importProfilesPage);
  server.on("/save", HTTP_GET, savePage);
  server.on("/combos", HTTP_GET, combosPage);
  server.on("/addcombo", HTTP_GET, addComboPage);
  server.on("/addrun", HTTP_POST, addRunCommandPage);
  server.on("/removecombo", HTTP_GET, removeComboPage);
  server.on("/reset", HTTP_GET, resetPage);
  server.on("/scan", HTTP_GET, scanPage);
  server.on("/wifi", HTTP_GET, wifiPage);
  server.on("/wifi", HTTP_POST, wifiPage);
  server.on("/forgetwifi", HTTP_GET, forgetWifiPage);
  server.on("/usb-upload-mode", HTTP_POST, usbUploadModePage);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("Clean ESP32-S3 macropad firmware starting");

  loadSettings();

  for (uint8_t i = 0; i < KEY_COUNT; i++) {
    pinMode(KEY_PINS[i], INPUT_PULLUP);
    stableDown[i] = rawDown(i);
    lastRawDown[i] = stableDown[i];
    actionSentOnPress[i] = false;
    changedAt[i] = millis();
    keyPressedAt[i] = 0;
    Serial.printf("GPIO %u initial %s\n", KEY_PINS[i], stableDown[i] ? "LOW" : "HIGH");
  }

  pixels.begin();
  pixels.setBrightness(ledBrightness);
  randomSeed(micros());
  applyLeds();

  Wire.begin(OLED_SDA, OLED_SCL);
  oledOk = startOled();
  if (oledOk) {
    applyOledDisplaySettings();
    roboEyes.begin(OLED_WIDTH, OLED_HEIGHT, 30);
    configureEyes();
  }
  oled("MacroLad booting");

  keyboard.begin();
  consumer.begin();
#if MACROPAD_USB_HID
  USB.begin();
#endif

  startWifi();
  startServer();
  showIdleDisplay();
}

void loop() {
  scanKeys();
  server.handleClient();
  maintainWifiAccess();
  if (restartAt && millis() >= restartAt) {
    delay(100);
    if (usbUploadModePending) {
      REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    }
    ESP.restart();
  }
  updateLighting();
  if (oledReturnAt && eyesActive) {
    updateKeyEyeAnimation();
    roboEyes.update();
  }
  if (oledReturnAt && millis() >= oledReturnAt) {
    oledReturnAt = 0;
    showIdleDisplay();
  } else {
    updateIdleDisplay();
  }
}
