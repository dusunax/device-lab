#ifndef SNOW_RTC_H
#define SNOW_RTC_H

struct RtcReading {
  bool readOk;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

RtcReading readRtcTime();
bool writeRtcTime(int year, int month, int day, int hour, int minute, int second);

#endif // SNOW_RTC_H
