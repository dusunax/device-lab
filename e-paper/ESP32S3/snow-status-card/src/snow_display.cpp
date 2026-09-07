#include "snow_display.h"
#include <Wire.h>
#include <WiFi.h>
#include "snow_telemetry.h"
#include "waveshare_epaper_1in54g/EPD_1in54g.h"

void drawCentered(const char* text, int y, sFONT* font, UWORD fg, UWORD bg) {
  int len = 0;
  while (text[len] != '\0') len++;
  int w = len * font->Width;
  int x = (EPD_1IN54G_WIDTH - w) / 2;
  if (x < 0) x = 0;
  Paint_DrawString_EN(x, y, text, font, fg, bg);
}

void drawEyesOpen() {
  // Small dot eyes: simple, not glossy.
  Paint_DrawCircle(75, 181, 3, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawCircle(125, 181, 3, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawLine(88, 188, 112, 188, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

void appendAddress(char* buffer, size_t bufferSize, const char* addressText, bool needsSeparator) {
  size_t used = strlen(buffer);
  if (needsSeparator && used + 1 < bufferSize) {
    buffer[used++] = ',';
    buffer[used] = '\0';
  }

  for (size_t i = 0; addressText[i] != '\0' && used + 1 < bufferSize; i++) {
    buffer[used++] = addressText[i];
  }
  buffer[used] = '\0';
}

void scanI2CBus() {
  telemetryLog(TELEMETRY_INFO, I2C_SCAN_START, "I2C scan started", "{\"i2c_sda\":47,\"i2c_scl\":48,\"address_start\":1,\"address_end\":126}");

  int foundCount = 0;
  char foundAddresses[160];
  foundAddresses[0] = '\0';

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      char addressText[8];
      snprintf(addressText, sizeof(addressText), "0x%02X", address);

      appendAddress(foundAddresses, sizeof(foundAddresses), addressText, foundCount > 0);
      foundCount++;

      char details[64];
      snprintf(details, sizeof(details), "{\"address\":\"%s\"}", addressText);
      telemetryLog(TELEMETRY_INFO, I2C_SCAN_DEVICE_FOUND, "I2C device found", details);
    }
  }

  char details[224];
  snprintf(details, sizeof(details), "{\"found_count\":%d,\"addresses\":\"%s\"}", foundCount, foundAddresses);
  telemetryLog(foundCount > 0 ? TELEMETRY_INFO : TELEMETRY_WARNING, I2C_SCAN_DONE, "I2C scan completed", details);
}

int estimateBatteryPercent(int voltageMv, int minMv, int fullMv) {
  if (voltageMv <= minMv) {
    return 0;
  }
  if (voltageMv >= fullMv) {
    return 100;
  }
  return (int)(((long)(voltageMv - minMv) * 100L) / (fullMv - minMv));
}

bool connectWiFi(const char* ssid, const char* password) {
  telemetryLog(TELEMETRY_INFO, WIFI_CONNECTING, "Wi-Fi connection started", "{\"retry_limit\":24,\"retry_delay_ms\":500}");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 24) {
    delay(500);
    retry++;
  }

  char details[48];
  snprintf(details, sizeof(details), "{\"retry_count\":%d}", retry);

  if (WiFi.status() == WL_CONNECTED) {
    telemetryLog(TELEMETRY_INFO, WIFI_CONNECTED, "Wi-Fi connected", details);
    return true;
  }

  telemetryLog(TELEMETRY_WARNING, WIFI_FAILED, "Wi-Fi connection failed", details);
  return false;
}
