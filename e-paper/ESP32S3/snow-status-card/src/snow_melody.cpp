#include "snow_melody.h"
#include "snow_pitches.h"

const SnowNote SNOW_BOOT_MELODY[] = {
  { NOTE_E5, 90, 20 },
  { NOTE_E5, 90, 20 },
  { NOTE_E5, 160, 20 },
  { NOTE_C5, 160, 20 },
  { NOTE_E5, 160, 20 },
  { NOTE_G5, 300, 180 },
  { NOTE_G4, 400, 20 },
};
const int SNOW_BOOT_MELODY_LENGTH = sizeof(SNOW_BOOT_MELODY) / sizeof(SNOW_BOOT_MELODY[0]);
