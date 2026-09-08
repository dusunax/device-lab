#include "snow_speaker.h"
#include <math.h>
#include <Wire.h>
#include "ESP_I2S.h"
#include "es8311/es8311.h"
#include "snow_telemetry.h"

#define SNOW_SPEAKER_I2S_MCLK_PIN 14
#define SNOW_SPEAKER_I2S_BCLK_PIN 15
#define SNOW_SPEAKER_I2S_LRCK_PIN 38
#define SNOW_SPEAKER_I2S_DOUT_PIN 45
#define SNOW_SPEAKER_I2S_DIN_PIN 16
#define SNOW_SPEAKER_PA_CTRL_PIN 46
#define SNOW_SPEAKER_PA_EN_PIN 42
#define SNOW_SPEAKER_SAMPLE_RATE 24000
#define SNOW_SPEAKER_MCLK_MULTIPLE 256
#define SNOW_SPEAKER_VOLUME 70

static I2SClass i2s;
static bool speakerReady = false;

bool initSpeaker() {
  telemetryLog(TELEMETRY_INFO, SPEAKER_INIT_START, "Speaker init started", "{\"codec\":\"ES8311\"}");

  pinMode(SNOW_SPEAKER_PA_EN_PIN, OUTPUT);
  pinMode(SNOW_SPEAKER_PA_CTRL_PIN, OUTPUT);
  digitalWrite(SNOW_SPEAKER_PA_EN_PIN, LOW);
  digitalWrite(SNOW_SPEAKER_PA_CTRL_PIN, HIGH);

  es8311_handle_t codec = es8311_create(I2C_NUM_0, ES8311_ADDRESS_0);
  if (codec == NULL) {
    telemetryLog(TELEMETRY_WARNING, SPEAKER_INIT_FAILED, "ES8311 codec create failed");
    return false;
  }

  es8311_clock_config_t clockConfig = {};
  clockConfig.mclk_inverted = false;
  clockConfig.sclk_inverted = false;
  clockConfig.mclk_from_mclk_pin = true;
  clockConfig.mclk_frequency = SNOW_SPEAKER_SAMPLE_RATE * SNOW_SPEAKER_MCLK_MULTIPLE;
  clockConfig.sample_frequency = SNOW_SPEAKER_SAMPLE_RATE;

  if (es8311_init(codec, &clockConfig, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) != ESP_OK) {
    telemetryLog(TELEMETRY_WARNING, SPEAKER_INIT_FAILED, "ES8311 codec init failed");
    return false;
  }
  es8311_voice_volume_set(codec, SNOW_SPEAKER_VOLUME, NULL);

  i2s.setPins(SNOW_SPEAKER_I2S_BCLK_PIN, SNOW_SPEAKER_I2S_LRCK_PIN, SNOW_SPEAKER_I2S_DOUT_PIN, SNOW_SPEAKER_I2S_DIN_PIN, SNOW_SPEAKER_I2S_MCLK_PIN);
  if (!i2s.begin(I2S_MODE_STD, SNOW_SPEAKER_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    telemetryLog(TELEMETRY_WARNING, SPEAKER_INIT_FAILED, "I2S bus init failed");
    return false;
  }

  speakerReady = true;
  telemetryLog(TELEMETRY_INFO, SPEAKER_INIT_START, "Speaker init completed", "{\"codec\":\"ES8311\"}");
  return true;
}

static void playTone(float frequencyHz, int durationMs) {
  const int chunkSamples = 240;
  int16_t buffer[chunkSamples];
  int totalSamples = (SNOW_SPEAKER_SAMPLE_RATE * durationMs) / 1000;
  int written = 0;
  int sampleIndex = 0;

  while (written < totalSamples) {
    int count = min(chunkSamples, totalSamples - written);
    for (int i = 0; i < count; i++) {
      buffer[i] = (int16_t)(sinf(2.0f * PI * frequencyHz * sampleIndex / SNOW_SPEAKER_SAMPLE_RATE) * 12000);
      sampleIndex++;
    }
    i2s.write((uint8_t*)buffer, count * sizeof(int16_t));
    written += count;
  }
}

void playStartupChime() {
  if (!speakerReady) {
    return;
  }
  playTone(523.25f, 150);  // C5
  playTone(659.25f, 150);  // E5
  playTone(783.99f, 200);  // G5
  telemetryLog(TELEMETRY_INFO, SPEAKER_TONE_PLAYED, "Speaker startup chime played", "{\"notes\":\"C5,E5,G5\"}");
}
