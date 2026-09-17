# e-paper

## 개요

이 문서는 e-paper 기기와 관련된 설정, 기능 검증, 문제 해결 정보를 관리합니다.

현재 e-paper 기기 별칭은 `Snow`입니다.

## 하드웨어

| 항목 | 값 |
| --- | --- |
| 별칭 | `Snow` |
| 기기 유형 | e-paper |
| 보드 | Waveshare ESP32-S3-ePaper-1.54G |
| 프레임워크 | Arduino / ESP32-S3 |
| 스케치 | `e-paper/ESP32S3/snow-status-card` |

## 현재 펌웨어 목표

부팅 후 e-paper 화면에 상태 카드를 1회 표시합니다.

```text
HELLO SUN-A
YYYY-MM-DD
WIFI OK 또는 WIFI FAIL
BATTERY OK 또는 BATTERY CHECK
SNOW READY
• — •
```

## 코드 구조

```text
e-paper/ESP32S3/snow-status-card/
  snow-status-card.ino        # e-paper 상태 카드 스케치. 전역 상태와 BLE 콜백. 내부 기기 별칭은 Snow
  secrets.example.h           # Git에 포함하는 예시 설정 파일
  src/snow_telemetry.*        # Serial JSON Lines telemetry
  src/snow_display.*          # 전역 상태 없는 순수 함수 (draw, I2C scan, Wi-Fi 연결)
  src/snow_battery.*          # 배터리 ADC 초기화/측정
  src/snow_speaker.*          # ES8311 코덱 초기화, 합성 멜로디 재생
  src/snow_melody.*           # 멜로디 데이터 (음 목록/길이/간격)
  src/snow_pitches.h          # 표준 음이름(NOTE_C4 등) 매핑 유틸
  src/snow_mic_recorder.*     # 마이크 레벨/녹음, WiFi 로컬 웹서버(wav 다운로드)
  src/snow_climate.*          # SHTC3 온습도 읽기(Wire 직접 구현, CRC8 검증)
  src/snow_rtc.*               # PCF85063 RTC 읽기/쓰기(Wire 직접 구현, BCD 레지스터)
  src/waveshare_epaper_1in54g -> vendor/waveshare_epaper_1in54g
  src/es8311 -> vendor/es8311

vendor/waveshare_epaper_1in54g/
  DEV_Config.*
  EPD_1in54g.*
  GUI_Paint.*
  fonts.h
  font16.cpp
  font20.cpp
  font24.cpp

vendor/es8311/
  es8311.*                    # Waveshare 공식 예제(07_Audio_out) 벤더링
  es8311_reg.h
```

기능 영역은 `e-paper`로 관리하고, 실제 기기 별칭 `Snow`는 코드 내부와 문서 본문에서 사용합니다.

## Bluetooth 테스트 기준

Snow는 BLE peripheral로 `Snow` 이름을 광고합니다. 폰에서 발견/연결되는지 확인하고, 연결/해제 결과는 Serial telemetry로 기록합니다.

| 항목 | 값 |
| --- | --- |
| BLE device name | `Snow` |
| 확인 이벤트 | `bluetooth_advertising_started`, `bluetooth_client_connected`, `bluetooth_client_disconnected` |
| 범위 | 발견/연결 확인. 데이터 송수신과 보안은 BLE 통신 항목에서 검증 |

## BLE 통신 기준

status(읽기)/command(쓰기) characteristic으로 폰 ↔ Snow 데이터를 주고받습니다. 두 characteristic
모두 패스키 페어링(display-only IO, bonding+MITM+secure connections)이 필요합니다.

| 항목 | 값 |
| --- | --- |
| status characteristic | `cdd36062-d8f1-43ba-9858-bd405c6152f8`, 읽기 전용, `battery {mV}mV {percent}%` |
| command characteristic | `008b1a7b-7e83-4333-b5f5-b8913e93b937`, 쓰기 전용, 값은 로그만 남기고 실행하지 않음 |
| 확인 이벤트 | `bluetooth_passkey_display`, `bluetooth_auth_complete`, `bluetooth_data_received` |

## Speaker 테스트 기준

ES8311 오디오 코덱(I2C 제어 + I2S 출력) + NS4150B 앰프를 통해 스피커로 소리를 출력합니다.
음원은 별도 샘플 파일 없이 합성한 짧은 톤(사인파)입니다.

| 항목 | 값 |
| --- | --- |
| 코덱 | ES8311 (I2C 주소 `0x18`) |
| I2C 핀 | SDA `47`, SCL `48` (기존 I2C 버스 공유) |
| I2S 핀 | MCLK `14`, BCLK `15`, LRCK `38`, DOUT `45`, DIN `16` |
| 앰프 제어 핀 | PA_EN `42`, PA_CTRL `46` |
| 확인 이벤트 | `speaker_init_start`(시작/완료 겸용), `speaker_init_failed`, `speaker_tone_played` |
| 범위 | e-paper 카드가 표시된 직후(`showOpenFace()` 이후) 부팅 멜로디 1회 재생. 실기기에서 청취로 확인 |
| 멜로디 데이터 | `src/snow_melody.h/.cpp`(음 목록), `src/snow_pitches.h`(표준 음이름 매핑 유틸) |

## Microphone 테스트 기준

Speaker와 동일한 ES8311 코덱의 analog mic 입력을 사용합니다. I2S는 DOUT/DIN 핀을 모두 설정하면
자동으로 풀 듀플렉스(TX+RX)로 동작하므로 별도 I2S 버스 초기화가 필요 없습니다.

| 항목 | 값 |
| --- | --- |
| 코덱 | ES8311 (Speaker와 동일 핸들 공유) |
| 마이크 게인 | `ES8311_MIC_GAIN_24DB` |
| 확인 이벤트 | `mic_level_read`(5초 주기 peak 레벨), `mic_recorder_started`, `mic_clip_recorded` |
| 청취 확인 | WiFi 연결 직후 3초 자동 녹음 → 로컬 웹서버(`http://<Snow IP>/`)에서 재생, `/mic.wav`에서 다운로드 |
| 비고 | IP는 화면/로그에 남기지 않는 기존 원칙에 따라 `mDNS`(`snow.local`)를 우선 안내. 환경에 따라 mDNS 해석이 안 될 수 있어, 필요 시 라우터에서 IP를 직접 확인 (troubleshooting #15) |

## Climate(온습도) 테스트 기준

SHTC3 센서(I2C `0x70`)를 Wire 기반으로 직접 구현했습니다(16비트 커맨드 전송 → 6바이트 응답,
CRC8 검증).

| 항목 | 값 |
| --- | --- |
| 센서 | SHTC3 (I2C 주소 `0x70`) |
| 측정 주기 | 30초 (loop 기준) |
| 확인 이벤트 | `climate_read`, `climate_read_failed` |
| 화면 표시 | 날짜 바로 아래 줄에 "OO.OC OO%" 형식으로 표시 |
| 비고 | RTC/SHTC3/ES8311은 `SNOW_PERIPHERAL_PWR_PIN`(GPIO42) 전원 레일을 공유함. `Wire.begin()` 전에 이 핀을 켜야 부팅 직후부터 안정적으로 응답함 (troubleshooting #17 참고) |

## RTC 테스트 기준

PCF85063 RTC(I2C `0x51`)를 Wire 기반으로 직접 구현했습니다(BCD 레지스터 0x04~0x0A 직접
읽기/쓰기). 부팅 시 1회 읽고, NTP 동기화에 성공하면 그 시각으로 RTC를 맞춥니다.

| 항목 | 값 |
| --- | --- |
| 센서 | PCF85063 (I2C 주소 `0x51`) |
| 확인 이벤트 | `rtc_read`, `rtc_read_failed`, `rtc_write`, `rtc_write_failed` |
| 시간 설정 | NTP 동기화 성공 직후 `writeRtcTime()`으로 RTC에 반영, 곧바로 재읽기로 확인 |
| 비고 | Climate와 동일하게 `SNOW_PERIPHERAL_PWR_PIN` 전원 레일 공유. `oscillator_stopped` 플래그로 RTC 배터리/전원 유지 여부 확인 가능(다음 "RTC 유지" 항목에서 활용) |

## 배터리 측정 기준

Waveshare ESP32-S3-ePaper-1.54G 문서와 공식 예제 기준으로 배터리 전압은 ADC로 측정합니다.

| 항목 | 값 |
| --- | --- |
| 측정 방식 | ADC battery measurement |
| ADC unit | `ADC_UNIT_1` |
| ADC channel | `ADC_CHANNEL_3` |
| 계산식 | `battery_voltage_v = calibrated_adc_mv * 0.001 * 2` |

I2C scan에서 확인되는 `0x18`, `0x51`, `0x70`은 각각 audio codec, RTC, 온습도 센서로 봅니다. `0x55` BQ27220 fuel gauge는 현재 문서 기준 배터리 측정 경로로 사용하지 않습니다.

## 보안 제약

- e-paper 화면에 IP 주소를 표시하지 않습니다.
- Serial Monitor에 IP 주소를 출력하지 않습니다.
- `secrets.h`를 커밋하지 않습니다.
- Wi-Fi 자격정보는 로컬의 추적 제외 파일에만 둡니다.

## 컴파일 전 필수 준비

Arduino IDE에서 `e-paper/ESP32S3/snow-status-card` 스케치를 컴파일하기 전에 로컬 `secrets.h` 파일이 필요합니다.

```bash
cp e-paper/ESP32S3/snow-status-card/secrets.example.h \
  e-paper/ESP32S3/snow-status-card/secrets.h
```

생성한 `secrets.h`에는 실제 Wi-Fi 값을 로컬에서만 입력합니다. 이 파일은 Git에 커밋하지 않습니다.

`secrets.h`가 없으면 다음 오류가 발생합니다.

```text
fatal error: secrets.h: No such file or directory
```

## Arduino IDE 기준 설정

```text
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
USB Mode: Hardware CDC and JTAG
Upload Mode: UART0 / Hardware CDC
Flash Size: 8MB
PSRAM: OPI PSRAM
Partition Scheme: 8M with spiffs
Serial Monitor baud: 115200
```

## 관련 문서

- `docs/e-paper/setup.md`
- `docs/e-paper/troubleshooting.md`
- `docs/e-paper/feature-checklist.md`
- `docs/e-paper/telemetry.md`
- `docs/shared/waveshare-epaper-driver.md`
