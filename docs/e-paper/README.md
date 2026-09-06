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
  snow-status-card.ino        # e-paper 상태 카드 스케치. 내부 기기 별칭은 Snow
  secrets.example.h           # Git에 포함하는 예시 설정 파일
  src/snow_telemetry.*        # Serial JSON Lines telemetry
  src/waveshare_epaper_1in54g -> vendor/waveshare_epaper_1in54g

vendor/waveshare_epaper_1in54g/
  DEV_Config.*
  EPD_1in54g.*
  GUI_Paint.*
  fonts.h
  font16.cpp
  font20.cpp
  font24.cpp
```

기능 영역은 `e-paper`로 관리하고, 실제 기기 별칭 `Snow`는 코드 내부와 문서 본문에서 사용합니다.

## Bluetooth 테스트 기준

Snow는 BLE peripheral로 `Snow` 이름을 광고합니다. 폰에서 발견/연결되는지 확인하고, 연결/해제 결과는 Serial telemetry로 기록합니다.

| 항목 | 값 |
| --- | --- |
| BLE device name | `Snow` |
| 확인 이벤트 | `bluetooth_advertising_started`, `bluetooth_client_connected`, `bluetooth_client_disconnected` |
| 범위 | 발견/연결 확인. 데이터 송수신은 다음 BLE 통신 항목에서 분리 검증 |

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
