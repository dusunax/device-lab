#include "snow_mic_recorder.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include "snow_speaker.h"
#include "snow_telemetry.h"

#define SNOW_MIC_RECORD_SECONDS 30
#define SNOW_MIC_SAMPLE_COUNT (SNOW_AUDIO_SAMPLE_RATE * SNOW_MIC_RECORD_SECONDS)

static WebServer server(80);
static int16_t* audioBuffer = NULL;
static size_t recordedSamples = 0;
static bool serverReady = false;

static void buildWavHeader(uint8_t* header, size_t dataSize) {
  uint32_t chunkSize = 36 + dataSize;
  uint32_t byteRate = SNOW_AUDIO_SAMPLE_RATE * 2;

  memcpy(header, "RIFF", 4);
  memcpy(header + 4, &chunkSize, 4);
  memcpy(header + 8, "WAVE", 4);
  memcpy(header + 12, "fmt ", 4);
  uint32_t fmtSize = 16;
  memcpy(header + 16, &fmtSize, 4);
  uint16_t audioFormat = 1;
  memcpy(header + 20, &audioFormat, 2);
  uint16_t numChannels = 1;
  memcpy(header + 22, &numChannels, 2);
  uint32_t sampleRate = SNOW_AUDIO_SAMPLE_RATE;
  memcpy(header + 24, &sampleRate, 4);
  memcpy(header + 28, &byteRate, 4);
  uint16_t blockAlign = 2;
  memcpy(header + 32, &blockAlign, 2);
  uint16_t bitsPerSample = 16;
  memcpy(header + 34, &bitsPerSample, 2);
  memcpy(header + 36, "data", 4);
  uint32_t dataSize32 = dataSize;
  memcpy(header + 40, &dataSize32, 4);
}

static void handleMicWav() {
  if (audioBuffer == NULL || recordedSamples == 0) {
    server.send(404, "text/plain", "no recording yet");
    return;
  }

  size_t dataSize = recordedSamples * sizeof(int16_t);
  uint8_t header[44];
  buildWavHeader(header, dataSize);

  server.setContentLength(44 + dataSize);
  server.send(200, "audio/wav", "");
  server.sendContent((const char*)header, 44);
  server.sendContent((const char*)audioBuffer, dataSize);
}

bool initMicRecorder() {
  audioBuffer = (int16_t*)ps_malloc(SNOW_MIC_SAMPLE_COUNT * sizeof(int16_t));
  if (audioBuffer == NULL) {
    telemetryLog(TELEMETRY_WARNING, MIC_RECORDER_FAILED, "Mic recorder buffer allocation failed");
    return false;
  }

  if (!MDNS.begin("snow")) {
    telemetryLog(TELEMETRY_WARNING, MIC_RECORDER_FAILED, "mDNS responder start failed");
  }

  server.on("/mic.wav", handleMicWav);
  server.begin();
  serverReady = true;
  telemetryLog(TELEMETRY_INFO, MIC_RECORDER_STARTED, "Mic recorder server started", "{\"url\":\"http://snow.local/mic.wav\"}");
  return true;
}

void recordMicClip() {
  if (audioBuffer == NULL) {
    return;
  }

  recordedSamples = 0;
  while (recordedSamples < SNOW_MIC_SAMPLE_COUNT) {
    size_t got = recordMicAudio(audioBuffer + recordedSamples, SNOW_MIC_SAMPLE_COUNT - recordedSamples);
    if (got == 0) {
      break;
    }
    recordedSamples += got;
  }

  char details[48];
  snprintf(details, sizeof(details), "{\"samples\":%u}", (unsigned)recordedSamples);
  telemetryLog(TELEMETRY_INFO, MIC_CLIP_RECORDED, "Mic clip recorded", details);
}

void handleMicServer() {
  if (serverReady) {
    server.handleClient();
  }
}
