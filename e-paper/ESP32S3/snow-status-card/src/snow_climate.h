#ifndef SNOW_CLIMATE_H
#define SNOW_CLIMATE_H

struct ClimateReading {
  bool readOk;
  float temperatureC;
  float humidityPercent;
};

ClimateReading readClimate();

#endif // SNOW_CLIMATE_H
