// Snow status-card firmware
// Expected Arduino IDE sketch path: e-paper/ESP32S3/snow-status-card/snow-status-card.ino
// Includes Serial JSON telemetry, ADC battery voltage checks, and I2C scanner diagnostics.

#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include "secrets.h"
#include "src/snow_telemetry.h"
#include "src/waveshare_epaper_1in54g/EPD_1in54g.h"
#include "src/waveshare_epaper_1in54g/GUI_Paint.h"
#include "src/waveshare_epaper_1in54g/fonts.h"

#ifndef ARDUINO_USB_CDC_ON_BOOT
#define ARDUINO_USB_CDC_ON_BOOT 0
#endif

#if !ARDUINO_USB_CDC_ON_BOOT
#error "Snow needs Tools > USB CDC On Boot > Enabled to show Serial Monitor logs. Enable it, then compile/upload again."
#endif

#define SNOW_FIRMWARE_VERSION "0.0.2"
#define SNOW_I2C_SDA_PIN 47
#define SNOW_I2C_SCL_PIN 48
#define SNOW_BATTERY_ADC_UNIT ADC_UNIT_1
#define SNOW_BATTERY_ADC_CHANNEL ADC_CHANNEL_3
#define SNOW_BATTERY_ADC_ATTEN ADC_ATTEN_DB_12
#define SNOW_BATTERY_ADC_BITWIDTH ADC_BITWIDTH_12
#define SNOW_BATTERY_DIVIDER_RATIO 2.0f
#define SNOW_BATTERY_MIN_MV 3000
#define SNOW_BATTERY_FULL_MV 4120

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

static adc_oneshot_unit_handle_t batteryAdcHandle = NULL;
static adc_cali_handle_t batteryAdcCaliHandle = NULL;
static bool batteryAdcInitialized = false;
static bool batteryAdcCalibrated = false;

int estimateBatteryPercent(int voltageMv) {
  if (voltageMv <= SNOW_BATTERY_MIN_MV) {
    return 0;
  }
  if (voltageMv >= SNOW_BATTERY_FULL_MV) {
    return 100;
  }
  return (int)(((long)(voltageMv - SNOW_BATTERY_MIN_MV) * 100L) / (SNOW_BATTERY_FULL_MV - SNOW_BATTERY_MIN_MV));
}

bool initBatteryAdc() {
  if (batteryAdcInitialized) {
    return true;
  }

  telemetryLog(TELEMETRY_INFO, BATTERY_ADC_INIT, "Battery ADC init started", "{\"adc_unit\":1,\"adc_channel\":3,\"attenuation\":\"ADC_ATTEN_DB_12\",\"bitwidth\":12}");

  adc_oneshot_unit_init_cfg_t unitConfig = {};
  unitConfig.unit_id = SNOW_BATTERY_ADC_UNIT;
  esp_err_t err = adc_oneshot_new_unit(&unitConfig, &batteryAdcHandle);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"new_unit\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC init failed", details);
    return false;
  }

  adc_oneshot_chan_cfg_t channelConfig = {};
  channelConfig.atten = SNOW_BATTERY_ADC_ATTEN;
  channelConfig.bitwidth = SNOW_BATTERY_ADC_BITWIDTH;
  err = adc_oneshot_config_channel(batteryAdcHandle, SNOW_BATTERY_ADC_CHANNEL, &channelConfig);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"config_channel\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC init failed", details);
    return false;
  }

  adc_cali_curve_fitting_config_t caliConfig = {};
  caliConfig.unit_id = SNOW_BATTERY_ADC_UNIT;
  caliConfig.atten = SNOW_BATTERY_ADC_ATTEN;
  caliConfig.bitwidth = SNOW_BATTERY_ADC_BITWIDTH;
  err = adc_cali_create_scheme_curve_fitting(&caliConfig, &batteryAdcCaliHandle);
  batteryAdcCalibrated = (err == ESP_OK);

  batteryAdcInitialized = true;
  char details[80];
  snprintf(details, sizeof(details), "{\"adc_unit\":1,\"adc_channel\":3,\"calibrated\":%s}", batteryAdcCalibrated ? "true" : "false");
  telemetryLog(TELEMETRY_INFO, BATTERY_ADC_INIT, "Battery ADC init completed", details);
  return true;
}

void logBatteryVoltage() {
  if (!initBatteryAdc()) {
    return;
  }

  int adcRaw = 0;
  esp_err_t err = adc_oneshot_read(batteryAdcHandle, SNOW_BATTERY_ADC_CHANNEL, &adcRaw);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"read\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC read failed", details);
    return;
  }

  int adcMv = 0;
  if (batteryAdcCalibrated) {
    err = adc_cali_raw_to_voltage(batteryAdcCaliHandle, adcRaw, &adcMv);
    if (err != ESP_OK) {
      char details[80];
      snprintf(details, sizeof(details), "{\"step\":\"calibrate\",\"adc_raw\":%d,\"esp_err\":%d}", adcRaw, (int)err);
      telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC calibration failed", details);
      return;
    }
  } else {
    adcMv = (int)(((long)adcRaw * 3300L) / 4096L);
  }

  int voltageMv = (int)(adcMv * SNOW_BATTERY_DIVIDER_RATIO);
  int estimatedPercent = estimateBatteryPercent(voltageMv);
  bool validVoltage = voltageMv >= SNOW_BATTERY_MIN_MV && voltageMv <= 4300;

  char details[192];
  snprintf(details, sizeof(details),
           "{\"adc_unit\":1,\"adc_channel\":3,\"adc_raw\":%d,\"adc_mv\":%d,\"voltage_mv\":%d,\"voltage_v\":%.2f,\"estimated_percent\":%d,\"calibrated\":%s}",
           adcRaw,
           adcMv,
           voltageMv,
           voltageMv / 1000.0f,
           estimatedPercent,
           batteryAdcCalibrated ? "true" : "false");

  telemetryLog(validVoltage ? TELEMETRY_INFO : TELEMETRY_WARNING, BATTERY_VOLTAGE_READ, "Battery voltage read", details);
}

bool connectWiFi() {
  telemetryLog(TELEMETRY_INFO, WIFI_CONNECTING, "Wi-Fi connection started", "{\"retry_limit\":24,\"retry_delay_ms\":500}");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

bool updateTodayDate() {
  telemetryLog(TELEMETRY_INFO, TIME_SYNC_START, "NTP time sync started", "{\"timezone\":\"KST\",\"retry_limit\":20,\"retry_delay_ms\":500}");

  // KST = UTC+9. No daylight saving time.
  configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 20) {
    delay(500);
    retry++;
  }

  if (retry >= 20) {
    snprintf(dateLine, sizeof(dateLine), "NO DATE");
    char details[48];
    snprintf(details, sizeof(details), "{\"retry_count\":%d}", retry);
    telemetryLog(TELEMETRY_WARNING, TIME_SYNC_FAILED, "NTP time sync failed", details);
    return false;
  }

  snprintf(dateLine, sizeof(dateLine), "%04d-%02d-%02d",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday);

  char details[80];
  snprintf(details, sizeof(details), "{\"date\":\"%s\",\"retry_count\":%d}", dateLine, retry);
  telemetryLog(TELEMETRY_INFO, TIME_SYNC_SUCCESS, "NTP time sync completed", details);
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
  telemetryLog(TELEMETRY_INFO, SYSTEM_START, "Snow status-card firmware started", "{\"baudrate\":115200}");
  char versionDetails[192];
  snprintf(versionDetails, sizeof(versionDetails), "{\"version\":\"%s\",\"sketch\":\"snow-status-card\",\"features\":\"json_telemetry,battery_adc,i2c_scanner\"}", SNOW_FIRMWARE_VERSION);
  telemetryLog(TELEMETRY_INFO, FIRMWARE_VERSION, "Snow firmware version", versionDetails);

  Wire.begin(SNOW_I2C_SDA_PIN, SNOW_I2C_SCL_PIN);
  Wire.setClock(400000UL);
  scanI2CBus();
  logBatteryVoltage();

  telemetryLog(TELEMETRY_INFO, DISPLAY_INIT_START, "Display module init started");
  if (DEV_Module_Init() != 0) {
    telemetryLog(TELEMETRY_ERROR, MODULE_INIT_FAILED, "Display module init failed");
    DEV_Module_Exit();
    while (1) delay(1000);
  }

  EPD_1IN54G_Init();
  EPD_1IN54G_Clear(EPD_1IN54G_WHITE);
  DEV_Delay_ms(500);

  imageSize = ((EPD_1IN54G_WIDTH % 4 == 0) ? (EPD_1IN54G_WIDTH / 4) : (EPD_1IN54G_WIDTH / 4 + 1)) * EPD_1IN54G_HEIGHT;
  image = (UBYTE *)malloc(imageSize);
  if (image == NULL) {
    char details[48];
    snprintf(details, sizeof(details), "{\"image_size\":%u}", imageSize);
    telemetryLog(TELEMETRY_ERROR, MEMORY_ALLOC_FAILED, "Display image buffer allocation failed", details);
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
  telemetryLog(TELEMETRY_INFO, DISPLAY_REFRESH_START, "Display refresh started", "{\"mode\":\"full_refresh\"}");
  showOpenFace();

  telemetryLog(TELEMETRY_INFO, DISPLAY_REFRESH_DONE, "Display refresh completed", "{\"mode\":\"full_refresh\"}");
}

void loop() {
  static unsigned long n = 0;
  static unsigned long lastBatteryReadMs = 0;
  static unsigned long lastI2CScanMs = 0;

  char details[48];
  snprintf(details, sizeof(details), "{\"sequence\":%lu}", n++);
  telemetryLog(TELEMETRY_INFO, SYSTEM_HEARTBEAT, "Main loop heartbeat", details);

  unsigned long now = millis();
  if (lastBatteryReadMs == 0 || now - lastBatteryReadMs >= 30000UL) {
    lastBatteryReadMs = now;
    logBatteryVoltage();
  }

  if (lastI2CScanMs == 0 || now - lastI2CScanMs >= 60000UL) {
    lastI2CScanMs = now;
    scanI2CBus();
  }

  delay(5000);
}
