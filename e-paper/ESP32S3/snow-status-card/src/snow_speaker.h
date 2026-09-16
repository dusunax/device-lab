#ifndef SNOW_SPEAKER_H
#define SNOW_SPEAKER_H

#include <Arduino.h>

#define SNOW_AUDIO_SAMPLE_RATE 24000

bool initSpeaker();
void playBootChime();
int readMicLevel();
size_t recordMicAudio(int16_t* buffer, size_t maxSamples);

#endif // SNOW_SPEAKER_H
