#include "snow_battery.h"
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include "snow_telemetry.h"

#define SNOW_BATTERY_ADC_UNIT ADC_UNIT_1
#define SNOW_BATTERY_ADC_CHANNEL ADC_CHANNEL_3
#define SNOW_BATTERY_ADC_ATTEN ADC_ATTEN_DB_12
#define SNOW_BATTERY_ADC_BITWIDTH ADC_BITWIDTH_12

static adc_oneshot_unit_handle_t batteryAdcHandle = NULL;
static adc_cali_handle_t batteryAdcCaliHandle = NULL;
static bool batteryAdcInitialized = false;
static bool batteryAdcCalibrated = false;

static bool initBatteryAdc() {
  if (batteryAdcInitialized) {
    return true;
  }

  telemetryLog(TELEMETRY_INFO, BATTERY_ADC_INIT, "Battery ADC init started", "{\"adc_unit\":1,\"adc_channel\":3,\"attenuation\":\"ADC_ATTEN_DB_12\",\"bitwidth\":12}");

  adc_oneshot_unit_init_cfg_t unitConfig = {};
  unitConfig.unit_id = SNOW_BATTERY_ADC_UNIT;
  esp_err_t err = adc_oneshot_new_unit(&unitConfig, &batteryAdcHandle);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"new_unit\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC init failed", details);
    return false;
  }

  adc_oneshot_chan_cfg_t channelConfig = {};
  channelConfig.atten = SNOW_BATTERY_ADC_ATTEN;
  channelConfig.bitwidth = SNOW_BATTERY_ADC_BITWIDTH;
  err = adc_oneshot_config_channel(batteryAdcHandle, SNOW_BATTERY_ADC_CHANNEL, &channelConfig);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"config_channel\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC init failed", details);
    return false;
  }

  adc_cali_curve_fitting_config_t caliConfig = {};
  caliConfig.unit_id = SNOW_BATTERY_ADC_UNIT;
  caliConfig.atten = SNOW_BATTERY_ADC_ATTEN;
  caliConfig.bitwidth = SNOW_BATTERY_ADC_BITWIDTH;
  err = adc_cali_create_scheme_curve_fitting(&caliConfig, &batteryAdcCaliHandle);
  batteryAdcCalibrated = (err == ESP_OK);

  batteryAdcInitialized = true;
  char details[80];
  snprintf(details, sizeof(details), "{\"adc_unit\":1,\"adc_channel\":3,\"calibrated\":%s}", batteryAdcCalibrated ? "true" : "false");
  telemetryLog(TELEMETRY_INFO, BATTERY_ADC_INIT, "Battery ADC init completed", details);
  return true;
}

int estimateBatteryPercent(int voltageMv, int minMv, int fullMv) {
  if (voltageMv <= minMv) {
    return 0;
  }
  if (voltageMv >= fullMv) {
    return 100;
  }
  return (int)(((long)(voltageMv - minMv) * 100L) / (fullMv - minMv));
}

BatteryReading readBatteryVoltage(float dividerRatio, int minMv, int maxMv, int fullMv) {
  BatteryReading result = { false, false, 0, 0 };

  if (!initBatteryAdc()) {
    return result;
  }

  int adcRaw = 0;
  esp_err_t err = adc_oneshot_read(batteryAdcHandle, SNOW_BATTERY_ADC_CHANNEL, &adcRaw);
  if (err != ESP_OK) {
    char details[64];
    snprintf(details, sizeof(details), "{\"step\":\"read\",\"esp_err\":%d}", (int)err);
    telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC read failed", details);
    return result;
  }

  int adcMv = 0;
  if (batteryAdcCalibrated) {
    err = adc_cali_raw_to_voltage(batteryAdcCaliHandle, adcRaw, &adcMv);
    if (err != ESP_OK) {
      char details[80];
      snprintf(details, sizeof(details), "{\"step\":\"calibrate\",\"adc_raw\":%d,\"esp_err\":%d}", adcRaw, (int)err);
      telemetryLog(TELEMETRY_WARNING, BATTERY_VOLTAGE_READ_FAILED, "Battery ADC calibration failed", details);
      return result;
    }
  } else {
    adcMv = (int)(((long)adcRaw * 3300L) / 4096L);
  }

  int voltageMv = (int)(adcMv * dividerRatio);
  int estimatedPercent = estimateBatteryPercent(voltageMv, minMv, fullMv);
  bool validVoltage = voltageMv >= minMv && voltageMv <= maxMv;

  char details[192];
  snprintf(details, sizeof(details),
          "{\"adc_unit\":1,\"adc_channel\":3,\"adc_raw\":%d,\"adc_mv\":%d,\"voltage_mv\":%d,\"voltage_v\":%.2f,\"estimated_percent\":%d,\"calibrated\":%s}",
           adcRaw,
           adcMv,
           voltageMv,
           voltageMv / 1000.0f,
           estimatedPercent,
           batteryAdcCalibrated ? "true" : "false");
  telemetryLog(validVoltage ? TELEMETRY_INFO : TELEMETRY_WARNING, BATTERY_VOLTAGE_READ, "Battery voltage read", details);

  result.readOk = true;
  result.inRange = validVoltage;
  result.voltageMv = voltageMv;
  result.percent = estimatedPercent;
  return result;
}
