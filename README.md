BTC Ticker für LILYGO T-Display-S3 (ESP32-S3)

Dieser BTC-Ticker zeigt den aktuellen Bitcoin-Kurs (EUR/USD) in Echtzeit auf dem LILYGO T-Display-S3 an.
Das Gerät verbindet sich per WLAN, ruft regelmäßig Kursdaten von einer API ab und stellt sie farbcodiert auf dem integrierten 1,9" ST7789-Display dar.

🧰 Unterstützte Hardware

LILYGO T-Display-S3 (ESP32-S3 mit ST7789 1,9" TFT, 170×320)

USB-C-Kabel zum Flashen

WLAN

Optional:

3D-gedrucktes Gehäuse

Powerbank / Dauerstromversorgung

🚀 Funktionen

Anzeige des BTC-Kurses in EUR und/oder USD

Farbige Markierung bei Kursänderungen

Zyklische Aktualisierung (intervallgesteuert)

Automatischer WLAN-Reconnect

Fehlertolerantes API-Handling

Statusanzeige auf dem Display

📥 Repository klonen
git clone https://github.com/eXplOteTeD/BTC-Ticker.git
cd BTC-Ticker

🛠️ Installation mit PlatformIO (empfohlen)

Voraussetzungen:

Visual Studio Code

PlatformIO-Extension

USB-Treiber für ESP32-S3

Flashen:

Projektordner in VS Code öffnen

PlatformIO → Build

T-Display-S3 per USB verbinden

Port auswählen (z. B. COM3)

PlatformIO → Upload

🧠 Installation mit Arduino IDE (Alternative)

In den Einstellungen unter „Zusätzliche Boardverwalter-URLs“ hinzufügen:
https://dl.espressif.com/dl/package_esp32_index.json

ESP32-Paket installieren

Board auswählen: ESP32S3 Dev Module

Libraries installieren:

TFT_eSPI

ArduinoJson

HTTPClient

WiFi

Sketch öffnen → Port wählen → Hochladen

Display-Konfiguration

In TFT_eSPI/User_Setup_Select.h wählen:

#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>


Ohne korrekte Display-Konfiguration bleibt der Bildschirm schwarz.

🔌 WLAN & API konfigurieren

In include/wifi_config.h eintragen:

const char* WIFI_SSID     = "DEIN_WLAN";
const char* WIFI_PASSWORD = "DEIN_PASSWORT";


Die API-URL ist im Code anpassbar.

📟 Nutzung

Nach dem Start:

ESP32 verbindet sich mit WLAN

BTC-Preis wird abgerufen

Anzeige wird periodisch aktualisiert

Farbcodes:

Farbe	Bedeutung
Grün	Kurs gestiegen
Rot	Kurs gefallen
Weiß	Anfangswert / keine Änderung
🗂️ Projektstruktur
/src/main.cpp           → Hauptprogramm
/include/wifi_config.h  → WLAN-Daten
/platformio.ini         → PlatformIO-Konfiguration
/lib/                   → Libraries
/test/                  → Tests

❗ Fehlerbehebung

Display schwarz

Falsche Boardauswahl

TFT_eSPI falsch konfiguriert

Keine WLAN-Verbindung

Falsches Passwort

Netzwerk blockiert Gerät

Keine Kursdaten

Falsche API-URL

API nicht erreichbar

🧩 Erweiterungen

Mini-Chart

Mehrere APIs (Fallback)

Tastenmenü

Auto-Dimmen

📝 Lizenz

Dieses Projekt ist unter der MIT-Lizenz veröffentlicht und vollständig frei nutzbar.
