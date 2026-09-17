#include "snow_climate.h"
#include <Wire.h>
#include "snow_telemetry.h"

#define SNOW_SHTC3_I2C_ADDR 0x70
#define SNOW_SHTC3_CMD_WAKEUP 0x3517
#define SNOW_SHTC3_CMD_SLEEP 0xB098
#define SNOW_SHTC3_CMD_MEASURE 0x7866
#define SNOW_SHTC3_CRC_POLYNOMIAL 0x31
#define SNOW_SHTC3_TEMP_OFFSET_C 4.0f

static bool writeCommand(uint16_t command) {
  Wire.beginTransmission(SNOW_SHTC3_I2C_ADDR);
  Wire.write((uint8_t)(command >> 8));
  Wire.write((uint8_t)(command & 0xFF));
  return Wire.endTransmission() == 0;
}

static uint8_t crc8(const uint8_t* data, int len) {
  uint8_t crc = 0xFF;
  for (int i = 0; i < len; i++) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ SNOW_SHTC3_CRC_POLYNOMIAL) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static ClimateReading readClimateOnce() {
  ClimateReading result = { false, 0.0f, 0.0f };

  if (!writeCommand(SNOW_SHTC3_CMD_WAKEUP)) {
    telemetryLog(TELEMETRY_WARNING, CLIMATE_READ_FAILED, "SHTC3 wakeup failed");
    return result;
  }
  delay(50);

  if (!writeCommand(SNOW_SHTC3_CMD_MEASURE)) {
    telemetryLog(TELEMETRY_WARNING, CLIMATE_READ_FAILED, "SHTC3 measure command failed");
    writeCommand(SNOW_SHTC3_CMD_SLEEP);
    return result;
  }
  delay(20);

  uint8_t bytes[6];
  int received = Wire.requestFrom(SNOW_SHTC3_I2C_ADDR, 6);
  if (received != 6) {
    telemetryLog(TELEMETRY_WARNING, CLIMATE_READ_FAILED, "SHTC3 read failed");
    writeCommand(SNOW_SHTC3_CMD_SLEEP);
    return result;
  }
  for (int i = 0; i < 6; i++) {
    bytes[i] = Wire.read();
  }

  writeCommand(SNOW_SHTC3_CMD_SLEEP);

  if (crc8(bytes, 2) != bytes[2] || crc8(bytes + 3, 2) != bytes[5]) {
    telemetryLog(TELEMETRY_WARNING, CLIMATE_READ_FAILED, "SHTC3 CRC mismatch");
    return result;
  }

  uint16_t rawTemp = ((uint16_t)bytes[0] << 8) | bytes[1];
  uint16_t rawHumi = ((uint16_t)bytes[3] << 8) | bytes[4];

  result.readOk = true;
  result.temperatureC = 175.0f * rawTemp / 65536.0f - 45.0f - SNOW_SHTC3_TEMP_OFFSET_C;
  result.humidityPercent = 100.0f * rawHumi / 65536.0f;

  char details[96];
  snprintf(details, sizeof(details), "{\"temperature_c\":%.2f,\"humidity_percent\":%.2f}", result.temperatureC, result.humidityPercent);
  telemetryLog(TELEMETRY_INFO, CLIMATE_READ, "SHTC3 climate read", details);

  return result;
}

ClimateReading readClimate() {
  // Right after boot (soon after Wire.begin()), the SHTC3 read can fail once
  // before the I2C bus settles. Retry a couple of times before giving up.
  for (int attempt = 0; attempt < 3; attempt++) {
    ClimateReading result = readClimateOnce();
    if (result.readOk) {
      return result;
    }
    delay(50);
  }
  return { false, 0.0f, 0.0f };
}
