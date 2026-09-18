#ifndef SNOW_WEATHER_H
#define SNOW_WEATHER_H

struct WeatherReading {
  bool readOk;
  float temperatureC;
  int weatherCode;
};

WeatherReading fetchWeather(float latitude, float longitude);
const char* weatherCodeToText(int weatherCode);

#endif // SNOW_WEATHER_H
