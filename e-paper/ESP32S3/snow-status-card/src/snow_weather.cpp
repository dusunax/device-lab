#include "snow_weather.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "snow_telemetry.h"

WeatherReading fetchWeather(float latitude, float longitude) {
  WeatherReading result = { false, 0.0f, -1 };

  if (WiFi.status() != WL_CONNECTED) {
    telemetryLog(TELEMETRY_WARNING, WEATHER_FETCH_FAILED, "Weather fetch skipped, WiFi not connected");
    return result;
  }

  char url[160];
  snprintf(url, sizeof(url), "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current_weather=true", latitude, longitude);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode != 200) {
    char details[48];
    snprintf(details, sizeof(details), "{\"http_code\":%d}", httpCode);
    telemetryLog(TELEMETRY_WARNING, WEATHER_FETCH_FAILED, "Weather HTTP request failed", details);
    http.end();
    return result;
  }

  String payload = http.getString();
  http.end();

  // Open-Meteo puts a "current_weather_units" object (with a "temperature":"°C"
  // label) before "current_weather". Search from the data key, not the whole
  // payload, or the units label gets parsed as the value.
  int weatherIndex = payload.indexOf("\"current_weather\":");
  if (weatherIndex < 0) {
    telemetryLog(TELEMETRY_WARNING, WEATHER_FETCH_FAILED, "Weather response missing current_weather");
    return result;
  }
  String weatherBlock = payload.substring(weatherIndex);

  int tempIndex = weatherBlock.indexOf("\"temperature\":");
  int codeIndex = weatherBlock.indexOf("\"weathercode\":");
  if (tempIndex < 0 || codeIndex < 0) {
    telemetryLog(TELEMETRY_WARNING, WEATHER_FETCH_FAILED, "Weather response parse failed");
    return result;
  }

  result.temperatureC = weatherBlock.substring(tempIndex + 14).toFloat();
  result.weatherCode = weatherBlock.substring(codeIndex + 14).toInt();
  result.readOk = true;

  char details[80];
  snprintf(details, sizeof(details), "{\"temperature_c\":%.1f,\"weather_code\":%d}", result.temperatureC, result.weatherCode);
  telemetryLog(TELEMETRY_INFO, WEATHER_FETCH, "Weather fetched", details);

  return result;
}

const char* weatherCodeToText(int weatherCode) {
  if (weatherCode == 0) return "CLEAR";
  if (weatherCode <= 3) return "CLOUDY";
  if (weatherCode == 45 || weatherCode == 48) return "FOG";
  if (weatherCode <= 67) return "RAIN";
  if (weatherCode <= 77) return "SNOW";
  if (weatherCode <= 82) return "RAIN";
  if (weatherCode <= 86) return "SNOW";
  if (weatherCode <= 99) return "STORM";
  return "UNKNOWN";
}
