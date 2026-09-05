#include <WiFi.h>
#include <time.h>
#include "secrets.h"
#include "src/waveshare_epaper_1in54g/EPD_1in54g.h"
#include "src/waveshare_epaper_1in54g/GUI_Paint.h"
#include "src/waveshare_epaper_1in54g/fonts.h"

#ifndef ARDUINO_USB_CDC_ON_BOOT
#define ARDUINO_USB_CDC_ON_BOOT 0
#endif

#if !ARDUINO_USB_CDC_ON_BOOT
#error "Snow needs Tools > USB CDC On Boot > Enabled to show Serial Monitor logs. Enable it, then compile/upload again."
#endif


UBYTE *image = NULL;
UWORD imageSize = 0;

bool wifiOk = false;
char dateLine[16] = "NO DATE";

void drawCentered(const char* text, int y, sFONT* font, UWORD fg, UWORD bg) {
  int len = 0;
  while (text[len] != '\0') len++;
  int w = len * font->Width;
  int x = (EPD_1IN54G_WIDTH - w) / 2;
  if (x < 0) x = 0;
  Paint_DrawString_EN(x, y, text, font, fg, bg);
}

bool connectWiFi() {
  Serial.println("SNOW_WIFI_CONNECTING");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 24) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("SNOW_WIFI_CONNECTED");
    return true;
  }
  Serial.println("SNOW_WIFI_FAILED");
  return false;
}


bool updateTodayDate() {
  Serial.println("SNOW_TIME_SYNC_START");

  // KST = UTC+9. No daylight saving time.
  configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 20) {
    delay(500);
    Serial.print("t");
    retry++;
  }
  Serial.println();

  if (retry >= 20) {
    snprintf(dateLine, sizeof(dateLine), "NO DATE");
    Serial.println("SNOW_TIME_SYNC_FAILED");
    return false;
  }

  snprintf(dateLine, sizeof(dateLine), "%04d-%02d-%02d",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday);

  Serial.print("SNOW_DATE: ");
  Serial.println(dateLine);
  return true;
}

void drawBaseCard() {
  Paint_Clear(EPD_1IN54G_WHITE);
  Paint_DrawRectangle(2, 2, 197, 197, EPD_1IN54G_BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(8, 8, 191, 42, EPD_1IN54G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  drawCentered("HELLO SUN-A", 14, &Font24, EPD_1IN54G_BLACK, EPD_1IN54G_YELLOW);
  drawCentered(dateLine, 56, &Font20, EPD_1IN54G_RED, EPD_1IN54G_WHITE);

  if (wifiOk) {
    drawCentered("WIFI OK", 86, &Font20, EPD_1IN54G_BLACK, EPD_1IN54G_WHITE);
  } else {
    drawCentered("WIFI FAIL", 86, &Font20, EPD_1IN54G_RED, EPD_1IN54G_WHITE);
  }

  drawCentered("SNOW READY", 146, &Font16, EPD_1IN54G_RED, EPD_1IN54G_WHITE);
}

void drawEyesOpen() {
  // Small dot eyes: simple, not glossy.
  Paint_DrawCircle(75, 181, 3, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawCircle(125, 181, 3, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawLine(88, 188, 112, 188, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

void showOpenFace() {
  drawBaseCard();
  drawEyesOpen();
  EPD_1IN54G_Display(image);
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give Arduino IDE Serial Monitor time to attach after USB reset.
  Serial.println();
  Serial.println("====================");
  Serial.println("SNOW_WIFI_TEST_START");
  Serial.println("Serial baud: 115200");
  Serial.println("====================");

  if (DEV_Module_Init() != 0) {
    Serial.println("DEV_Module_Init failed");
    DEV_Module_Exit();
    while (1) delay(1000);
  }

  EPD_1IN54G_Init();
  EPD_1IN54G_Clear(EPD_1IN54G_WHITE);
  DEV_Delay_ms(500);

  imageSize = ((EPD_1IN54G_WIDTH % 4 == 0) ? (EPD_1IN54G_WIDTH / 4) : (EPD_1IN54G_WIDTH / 4 + 1)) * EPD_1IN54G_HEIGHT;
  image = (UBYTE *)malloc(imageSize);
  if (image == NULL) {
    Serial.println("malloc failed");
    while (1) delay(1000);
  }

  Paint_NewImage(image, EPD_1IN54G_WIDTH, EPD_1IN54G_HEIGHT, 0, EPD_1IN54G_WHITE);
  Paint_SetScale(4);
  Paint_SelectImage(image);

  wifiOk = connectWiFi();
  if (wifiOk) {
    updateTodayDate();
  }

  // Draw once only. e-Paper refresh is slow, so avoid repeated updates.
  Serial.println("SNOW_DISPLAY_ONCE");
  showOpenFace();

  Serial.println("SNOW_DISPLAY_DONE");
}

void loop() {
  static unsigned long n = 0;
  Serial.print("SNOW_HEARTBEAT ");
  Serial.println(n++);
  delay(5000);
}
