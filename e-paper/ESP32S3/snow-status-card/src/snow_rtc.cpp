#include "snow_rtc.h"
#include <Wire.h>
#include "snow_telemetry.h"

#define SNOW_RTC_I2C_ADDR 0x51
#define SNOW_RTC_REG_SECONDS 0x04

static uint8_t bcdToDec(uint8_t bcd) {
  return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

static uint8_t decToBcd(int dec) {
  return (uint8_t)(((dec / 10) << 4) | (dec % 10));
}

RtcReading readRtcTime() {
  RtcReading result = { false, 0, 0, 0, 0, 0, 0 };

  Wire.beginTransmission(SNOW_RTC_I2C_ADDR);
  Wire.write(SNOW_RTC_REG_SECONDS);
  if (Wire.endTransmission(false) != 0) {
    telemetryLog(TELEMETRY_WARNING, RTC_READ_FAILED, "PCF85063 register write failed");
    return result;
  }

  uint8_t bytes[7];
  int received = Wire.requestFrom(SNOW_RTC_I2C_ADDR, 7);
  if (received != 7) {
    telemetryLog(TELEMETRY_WARNING, RTC_READ_FAILED, "PCF85063 read failed");
    return result;
  }
  for (int i = 0; i < 7; i++) {
    bytes[i] = Wire.read();
  }

  bool oscillatorStopped = (bytes[0] & 0x80) != 0;

  result.readOk = true;
  result.second = bcdToDec(bytes[0] & 0x7F);
  result.minute = bcdToDec(bytes[1] & 0x7F);
  result.hour = bcdToDec(bytes[2] & 0x3F);
  result.day = bcdToDec(bytes[3] & 0x3F);
  result.month = bcdToDec(bytes[5] & 0x1F);
  result.year = 2000 + bcdToDec(bytes[6]);

  char details[112];
  snprintf(details, sizeof(details), "{\"datetime\":\"%04d-%02d-%02d %02d:%02d:%02d\",\"oscillator_stopped\":%s}",
           result.year, result.month, result.day, result.hour, result.minute, result.second,
           oscillatorStopped ? "true" : "false");
  telemetryLog(TELEMETRY_INFO, RTC_READ, "PCF85063 RTC read", details);

  return result;
}

bool writeRtcTime(int year, int month, int day, int hour, int minute, int second) {
  Wire.beginTransmission(SNOW_RTC_I2C_ADDR);
  Wire.write(SNOW_RTC_REG_SECONDS);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(day));
  Wire.write(0);  // weekday, unused
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year - 2000));
  if (Wire.endTransmission() != 0) {
    telemetryLog(TELEMETRY_WARNING, RTC_WRITE_FAILED, "PCF85063 time write failed");
    return false;
  }

  telemetryLog(TELEMETRY_INFO, RTC_WRITE, "PCF85063 time set from NTP");
  return true;
}
