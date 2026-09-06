#include "snow_telemetry.h"

static const char* levelToString(TelemetryLevel level) {
  switch (level) {
    case TELEMETRY_INFO:
      return "info";
    case TELEMETRY_WARNING:
      return "warning";
    case TELEMETRY_ERROR:
      return "error";
    default:
      return "unknown";
  }
}

static const char* eventToString(TelemetryEvent event) {
  switch (event) {
    case SYSTEM_START:
      return "system_start";
    case FIRMWARE_VERSION:
      return "firmware_version";
    case SYSTEM_HEARTBEAT:
      return "system_heartbeat";
    case MEMORY_ALLOC_FAILED:
      return "memory_alloc_failed";
    case MODULE_INIT_FAILED:
      return "module_init_failed";
    case WIFI_CONNECTING:
      return "wifi_connecting";
    case WIFI_CONNECTED:
      return "wifi_connected";
    case WIFI_FAILED:
      return "wifi_failed";
    case TIME_SYNC_START:
      return "time_sync_start";
    case TIME_SYNC_SUCCESS:
      return "time_sync_success";
    case TIME_SYNC_FAILED:
      return "time_sync_failed";
    case DISPLAY_INIT_START:
      return "display_init_start";
    case DISPLAY_REFRESH_START:
      return "display_refresh_start";
    case DISPLAY_REFRESH_DONE:
      return "display_refresh_done";
    case BATTERY_ADC_INIT:
      return "battery_adc_init";
    case BATTERY_VOLTAGE_READ:
      return "battery_voltage_read";
    case BATTERY_VOLTAGE_READ_FAILED:
      return "battery_voltage_read_failed";
    case I2C_SCAN_START:
      return "i2c_scan_start";
    case I2C_SCAN_DEVICE_FOUND:
      return "i2c_scan_device_found";
    case I2C_SCAN_DONE:
      return "i2c_scan_done";
    default:
      return "unknown_event";
  }
}

static void printEscapedJsonString(const char* value) {
  Serial.print('"');
  if (value != NULL) {
    for (const char* p = value; *p != '\0'; ++p) {
      switch (*p) {
        case '\\':
          Serial.print("\\\\");
          break;
        case '"':
          Serial.print("\\\"");
          break;
        case '\n':
          Serial.print("\\n");
          break;
        case '\r':
          Serial.print("\\r");
          break;
        case '\t':
          Serial.print("\\t");
          break;
        default:
          Serial.print(*p);
          break;
      }
    }
  }
  Serial.print('"');
}

void telemetryLog(TelemetryLevel level, TelemetryEvent event, const char* message) {
  telemetryLog(level, event, message, "{}");
}

void telemetryLog(TelemetryLevel level, TelemetryEvent event, const char* message, const char* detailsJson) {
  Serial.print('{');
  Serial.print("\"level\":");
  printEscapedJsonString(levelToString(level));
  Serial.print(",\"event\":");
  printEscapedJsonString(eventToString(event));
  Serial.print(",\"message\":");
  printEscapedJsonString(message);
  Serial.print(",\"uptime_ms\":");
  Serial.print(millis());
  Serial.print(",\"details\":");

  if (detailsJson != NULL && detailsJson[0] == '{') {
    Serial.print(detailsJson);
  } else {
    Serial.print("{}");
  }

  Serial.println('}');
}
