#ifndef SNOW_TELEMETRY_H
#define SNOW_TELEMETRY_H

#include <Arduino.h>

// Serial telemetry is emitted as one JSON object per line.
// Do not include Wi-Fi SSID, password, IP address, MAC address, or token values.
enum TelemetryLevel {
  TELEMETRY_INFO,
  TELEMETRY_WARNING,
  TELEMETRY_ERROR
};

enum TelemetryEvent {
  SYSTEM_START,
  FIRMWARE_VERSION,
  SYSTEM_HEARTBEAT,
  MEMORY_ALLOC_FAILED,
  MODULE_INIT_FAILED,

  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_FAILED,

  TIME_SYNC_START,
  TIME_SYNC_SUCCESS,
  TIME_SYNC_FAILED,

  DISPLAY_INIT_START,
  DISPLAY_REFRESH_START,
  DISPLAY_REFRESH_DONE,

  BATTERY_ADC_INIT,
  BATTERY_VOLTAGE_READ,
  BATTERY_VOLTAGE_READ_FAILED,

  BLUETOOTH_INIT_START,
  BLUETOOTH_ADVERTISING_STARTED,
  BLUETOOTH_CLIENT_CONNECTED,
  BLUETOOTH_CLIENT_DISCONNECTED,

  I2C_SCAN_START,
  I2C_SCAN_DEVICE_FOUND,
  I2C_SCAN_DONE
};

void telemetryLog(TelemetryLevel level, TelemetryEvent event, const char* message);
void telemetryLog(TelemetryLevel level, TelemetryEvent event, const char* message, const char* detailsJson);

#endif // SNOW_TELEMETRY_H
