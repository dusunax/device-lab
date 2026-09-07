#ifndef SNOW_BATTERY_H
#define SNOW_BATTERY_H

#include <Arduino.h>

struct BatteryReading {
  bool readOk;    // false if ADC init/read/calibration failed; other fields are unset
  bool inRange;   // voltageMv within [minMv, maxMv]; only meaningful when readOk
  int voltageMv;
  int percent;
};

int estimateBatteryPercent(int voltageMv, int minMv, int fullMv);
BatteryReading readBatteryVoltage(float dividerRatio, int minMv, int maxMv, int fullMv);

#endif // SNOW_BATTERY_H
