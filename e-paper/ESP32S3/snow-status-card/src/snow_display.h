#ifndef SNOW_DISPLAY_H
#define SNOW_DISPLAY_H

#include <Arduino.h>
#include "waveshare_epaper_1in54g/GUI_Paint.h"
#include "waveshare_epaper_1in54g/fonts.h"

// Pure helpers with no dependency on the sketch's global state.
void drawCentered(const char* text, int y, sFONT* font, UWORD fg, UWORD bg);
void drawEyesOpen();
void appendAddress(char* buffer, size_t bufferSize, const char* addressText, bool needsSeparator);
void scanI2CBus();
int estimateBatteryPercent(int voltageMv, int minMv, int fullMv);
bool connectWiFi(const char* ssid, const char* password);

#endif // SNOW_DISPLAY_H
