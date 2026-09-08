#ifndef SNOW_MELODY_H
#define SNOW_MELODY_H

struct SnowNote {
  int frequencyHz;
  int durationMs;
  int gapMs;
};

extern const SnowNote SNOW_BOOT_MELODY[];
extern const int SNOW_BOOT_MELODY_LENGTH;

#endif // SNOW_MELODY_H
