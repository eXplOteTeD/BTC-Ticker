#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include "wifi_config.h"

// ---------------------------------------------------------
// Display-Konfiguration für LilyGO T-Display-S3 (ST7789, 170x320, 8-bit Parallel)
// Basierend auf offizieller LovyanGFX Panel-Definition für dieses Panel.
// ---------------------------------------------------------

class LGFX : public lgfx::LGFX_Device
{
  // Panel-Typ
  lgfx::Panel_ST7789 _panel_instance;
  // 8-Bit Parallel-Bus
  lgfx::Bus_Parallel8 _bus_instance;
  // Backlight per PWM
  lgfx::Light_PWM _light_instance;

public:
  LGFX(void)
  {
    { // Bus-Konfiguration
      auto cfg = _bus_instance.config();

      // Achtung: i2s_port wird in aktuellen LovyanGFX-Versionen nicht mehr genutzt.
      // Also NICHT setzen (sonst Compile-Fehler).

      cfg.freq_write = 20000000;  // max. 20 MHz
      cfg.pin_wr     = 8;         // WR
      cfg.pin_rd     = 9;         // RD
      cfg.pin_rs     = 7;         // D/C

      cfg.pin_d0     = 39;
      cfg.pin_d1     = 40;
      cfg.pin_d2     = 41;
      cfg.pin_d3     = 42;
      cfg.pin_d4     = 45;
      cfg.pin_d5     = 46;
      cfg.pin_d6     = 47;
      cfg.pin_d7     = 48;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Panel-Konfiguration
      auto cfg = _panel_instance.config();

      cfg.pin_cs      = 6;     // CS
      cfg.pin_rst     = 5;     // RST
      cfg.pin_busy    = -1;    // kein BUSY

      cfg.panel_width  = 170;
      cfg.panel_height = 320;
      cfg.offset_x     = 35;   // wie im T-Display-S3-Setup
      cfg.offset_y     = 0;
      cfg.offset_rotation  = 0;

      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;

      cfg.readable     = true;
      cfg.invert       = true;
      cfg.rgb_order    = false;
      cfg.dlen_16bit   = false;
      cfg.bus_shared   = false;

      _panel_instance.config(cfg);
    }

    { // Backlight
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 38;    // Backlight-Pin
      cfg.invert      = false;
      cfg.freq        = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

static LGFX display;

// ---------------------------------------------------------
// BTC-Logik
// ---------------------------------------------------------

// API: einfacher Endpunkt für BTC/EUR
static const char *BTC_API_URL =
    "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=eur";

constexpr uint32_t FETCH_INTERVAL_MS      = 30 * 1000;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15 * 1000;
constexpr uint32_t WIFI_RETRY_DELAY_MS     = 1000;

uint32_t lastFetchMs = 0;
float    lastPrice   = 0.0f;

// Vorwärtsdeklarationen
void showStatus(const char *msg, uint16_t color = TFT_WHITE);
void showError(const char *msg);
void showPrice(float price);
bool  connectWiFi();
bool  fetchBtcPrice(float &outPrice);

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  ::delay(200);

  // Display initialisieren
  display.init();
  display.setRotation(1);  // Querformat (170 x 320 -> 320 breit, 170 hoch)
  display.setBrightness(255);
  display.fillScreen(TFT_BLACK);

  showStatus("BTC Ticker startet...", TFT_CYAN);

  // WiFi verbinden
  if (!connectWiFi()) {
    showError("WLAN fehlgeschlagen");
    return;
  }

  showStatus("Hole BTC/EUR-Preis...", TFT_YELLOW);

  float price = 0.0f;
  if (fetchBtcPrice(price)) {
    lastPrice   = price;
    lastFetchMs = ::millis();
    showPrice(price);
  } else {
    showError("HTTP-Fehler");
  }
}

// ---------------------------------------------------------
// Loop
// ---------------------------------------------------------

void loop()
{
  uint32_t now = ::millis();

  if (now - lastFetchMs >= FETCH_INTERVAL_MS) {
    lastFetchMs = now;

    if (WiFi.status() != WL_CONNECTED) {
      showStatus("Verbinde WLAN...", TFT_ORANGE);
      if (!connectWiFi()) {
        showError("WLAN down");
        return;
      }
    }

    float price = 0.0f;
    if (fetchBtcPrice(price)) {
      lastPrice = price;
      showPrice(price);
    } else {
      showError("API Fehler");
    }
  }

  // kleines Delay, damit die CPU nicht komplett durchdreht
  ::delay(50);
}

// ---------------------------------------------------------
// WiFi & HTTP
// ---------------------------------------------------------

bool connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  showStatus("Verbinde mit WLAN...", TFT_YELLOW);
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  uint32_t start = ::millis();
  while (WiFi.status() != WL_CONNECTED &&
         (::millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
    ::delay(WIFI_RETRY_DELAY_MS);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK, IP: ");
    Serial.println(WiFi.localIP());
    showStatus("WLAN OK", TFT_GREEN);
    ::delay(800);
    return true;
  }

  Serial.println("WiFi connect FAILED");
  return false;
}

bool fetchBtcPrice(float &outPrice)
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("fetchBtcPrice: kein WLAN");
    return false;
  }

  HTTPClient http;
  http.begin(BTC_API_URL);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP Fehler: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // JSON: { "bitcoin": { "eur": 12345.67 } }
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("JSON Fehler: ");
    Serial.println(err.c_str());
    return false;
  }

  if (!doc.containsKey("bitcoin") ||
      !doc["bitcoin"].as<JsonObject>().containsKey("eur")) {
    Serial.println("JSON: Feld bitcoin.eur fehlt");
    return false;
  }

  outPrice = doc["bitcoin"]["eur"].as<float>();
  Serial.print("BTC/EUR: ");
  Serial.println(outPrice);
  return true;
}

// ---------------------------------------------------------
// Anzeige-Funktionen
// ---------------------------------------------------------

void showStatus(const char *msg, uint16_t color)
{
  display.fillScreen(TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);

  display.setTextColor(color, TFT_BLACK);
  display.setFont(&lgfx::v1::fonts::Font2);

  int16_t cx = display.width() / 2;
  int16_t cy = display.height() / 2;

  display.drawString(msg, cx, cy);
}

void showError(const char *msg)
{
  Serial.print("Anzeige-Fehler: ");
  Serial.println(msg);

  display.fillScreen(TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);

  display.setTextColor(TFT_RED, TFT_BLACK);
  display.setFont(&lgfx::v1::fonts::Font4);

  int16_t cx = display.width() / 2;
  int16_t cy = display.height() / 2 - 10;
  display.drawString("FEHLER", cx, cy);

  display.setFont(&lgfx::v1::fonts::Font2);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString(msg, cx, cy + 20);
}

void showPrice(float price)
{
  display.fillScreen(TFT_BLACK);

  display.setTextDatum(textdatum_t::middle_center);

  // Status-Text oben
  display.setFont(&lgfx::v1::fonts::Font2);
  display.setTextColor(TFT_CYAN, TFT_BLACK);
  display.drawString("BTC / EUR", display.width() / 2, 20);

  // Preis groß in der Mitte
  display.setFont(&lgfx::v1::fonts::Font6);
  display.setTextColor(TFT_GREEN, TFT_BLACK);

  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f", price);

  display.drawString(buf, display.width() / 2, display.height() / 2);

  // Kleine Info unten
  display.setFont(&lgfx::v1::fonts::Font2);
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.drawString("Update alle 30s", display.width() / 2, display.height() - 20);
}
