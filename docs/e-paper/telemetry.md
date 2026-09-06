# Snow Serial telemetry

## 목적

Snow firmware의 Serial Monitor 로그를 사람이 읽는 임시 문자열이 아니라, 기계적으로 파싱 가능한 JSON Lines 형식으로 기록합니다.

화면 UI는 e-paper 특성상 저빈도 요약 상태만 표시하고, 상세 상태는 Serial telemetry로 분리합니다.

---

## 출력 형식

각 로그는 한 줄에 하나의 JSON object로 출력합니다.

```json
{"level":"info","event":"wifi_connected","message":"Wi-Fi connected","uptime_ms":1234,"details":{"retry_count":3}}
```

필드 기준:

| 필드 | 타입 | 설명 |
| --- | --- | --- |
| `level` | string | `info`, `warning`, `error` 중 하나 |
| `event` | string | 이벤트 이름. snake_case 사용 |
| `message` | string | 사람이 읽을 수 있는 짧은 설명 |
| `uptime_ms` | number | 부팅 이후 경과 시간. `millis()` 기준 |
| `details` | object | 이벤트별 상세 값 |

---

## 레벨 기준

| 레벨 | 기준 | 예시 |
| --- | --- | --- |
| `info` | 정상 진행 상태 | firmware start, Wi-Fi 연결 시작/성공, display refresh 완료 |
| `warning` | 기능은 계속 진행 가능하지만 기대 상태가 아닌 경우 | Wi-Fi 연결 실패, NTP 동기화 실패 |
| `error` | 주요 기능을 계속할 수 없는 경우 | display module init 실패, image buffer allocation 실패 |

---

## 이벤트 기준

| 이벤트 | 레벨 | 설명 |
| --- | --- | --- |
| `system_start` | `info` | firmware 시작 |
| `firmware_version` | `info` | 업로드된 firmware 버전과 포함 기능 확인 |
| `system_heartbeat` | `info` | loop 동작 확인 |
| `memory_alloc_failed` | `error` | e-paper image buffer 할당 실패 |
| `module_init_failed` | `error` | Waveshare display module 초기화 실패 |
| `wifi_connecting` | `info` | Wi-Fi 연결 시도 시작 |
| `wifi_connected` | `info` | Wi-Fi 연결 성공 |
| `wifi_failed` | `warning` | Wi-Fi 연결 실패 |
| `time_sync_start` | `info` | NTP 시간 동기화 시작 |
| `time_sync_success` | `info` | NTP 시간 동기화 성공 |
| `time_sync_failed` | `warning` | NTP 시간 동기화 실패 |
| `display_init_start` | `info` | e-paper module 초기화 시작 |
| `display_refresh_start` | `info` | e-paper full refresh 시작 |
| `display_refresh_done` | `info` | e-paper full refresh 완료 |
| `battery_adc_init` | `info` | ADC 기반 배터리 전압 측정 초기화 시작 |
| `battery_voltage_read` | `info` 또는 `warning` | ADC 기반 배터리 전압 읽기 완료. 유효 전압 범위면 `info`, 범위 밖이면 `warning` |
| `battery_voltage_read_failed` | `warning` | ADC 배터리 전압 읽기 실패 |
| `bluetooth_init_start` | `info` | BLE advertising 초기화 시작 |
| `bluetooth_advertising_started` | `info` | BLE peripheral advertising 시작 또는 재시작 |
| `bluetooth_client_connected` | `info` | BLE client 연결 확인 |
| `bluetooth_client_disconnected` | `info` | BLE client 연결 해제 확인 |
| `i2c_scan_start` | `info` | I2C address scan 시작 |
| `i2c_scan_device_found` | `info` | I2C 응답 주소 발견 |
| `i2c_scan_done` | `info` 또는 `warning` | I2C scan 완료. 발견 주소가 없으면 `warning` |

---

## 배터리 전압 측정 기준

Waveshare ESP32-S3-ePaper-1.54G 문서와 공식 예제 기준으로 배터리 전압은 I2C fuel gauge가 아니라 ADC로 측정합니다.

근거 파일:

- Waveshare README: `Lithium battery connector, charging circuit, battery voltage measurement`
- Arduino 예제: `Example/Arduino_3.2.0/examples/01_ADC_Test/adc_bsp.cpp`
- ESP-IDF 예제: `Example/ESP-IDF_5.5.1/01_ADC_Test/components/adc_bsp/adc_bsp.c`

측정 기준:

| 항목 | 값 |
| --- | --- |
| 측정 방식 | ADC battery measurement |
| ADC unit | `ADC_UNIT_1` |
| ADC channel | `ADC_CHANNEL_3` |
| attenuation | `ADC_ATTEN_DB_12` |
| bit width | `ADC_BITWIDTH_12` |
| 전압 계산 | `battery_voltage_v = calibrated_adc_mv * 0.001 * 2` |

공식 예제는 ADC 보정값을 mV로 변환한 뒤, 배터리 전압 분압 회로를 보정하기 위해 `* 2`를 적용합니다.

`battery_voltage_read` 이벤트의 `details`에서 다음 값을 확인합니다.

```json
{"adc_unit":1,"adc_channel":3,"adc_raw":2048,"voltage_v":3.92,"voltage_mv":3920,"estimated_percent":82}
```

판정 기준:

| 값 | 의미 |
| --- | --- |
| `voltage_mv` | 배터리 전압. 일반적인 1셀 리튬 배터리는 대략 3000~4200mV 범위 |
| `voltage_v` | 사람이 읽기 쉬운 V 단위 전압 |
| `adc_raw` | 원본 ADC 값. 노이즈 확인과 디버깅에 사용 |
| `estimated_percent` | 전압 기반 단순 추정치. 정확한 잔량이 아니라 참고값 |

배터리 측정은 `voltage_mv`가 유효 범위에 있을 때 확인된 것으로 봅니다. 단, 전압 기반 `estimated_percent`는 fuel gauge 잔량이 아니라 단순 추정값입니다.

실제 Snow 확인값:

| 항목 | 값 |
| --- | --- |
| firmware feature marker | `battery_adc` |
| 확인 이벤트 | `battery_voltage_read` |
| `adc_raw` | `2402` ~ `2407` |
| `adc_mv` | `2060` ~ `2063` |
| 계산 전압 | 약 `4120` ~ `4126mV` |
| 판정 | 배터리 연결 및 ADC 전압 측정 확인 |

---

## 충전 확인 기준

Waveshare 문서에는 충전 회로가 있다고 명시되어 있지만, 현재 확인한 공식 ADC 예제는 충전 상태 레지스터나 별도 charge status 핀을 제공하지 않습니다.

따라서 현재 firmware 기준 충전 확인은 다음 중 하나로 판정합니다.

| 방법 | 판정 기준 | 비고 |
| --- | --- | --- |
| 전압 추이 | USB 전원 연결 후 `battery_voltage_read.voltage_mv`가 시간에 따라 증가 | 배터리가 이미 4.12V 근처면 증가가 거의 없을 수 있음 |
| 충전 표시 | 보드의 충전 LED 또는 표시가 충전/완료 상태를 나타냄 | 표시 위치와 색상은 보드 실물/문서 추가 확인 필요 |
| 장시간 관찰 | 낮은 전압 상태에서 USB 연결 후 전압이 회복됨 | 가장 확실한 firmware 기반 확인 방법 |

`voltage_mv`가 4120mV 전후이면 1셀 리튬 배터리 기준 만충에 가까우므로, 충전 회로가 동작해도 전압 상승이 작거나 충전이 종료되어 보일 수 있습니다.

실제 Snow 충전 확인 관찰값:

| 항목 | 값 |
| --- | --- |
| 관찰 조건 | USB 연결 상태에서 Serial telemetry 직접 수집 |
| 관찰 시간 | 약 95초 |
| 확인 이벤트 수 | `battery_voltage_read` 3회 |
| 측정 전압 | `4124mV`, `4124mV`, `4124mV` |
| 전압 변화 | `0mV` |
| 판정 | 전압은 만충에 가까운 범위에서 안정적. 현재 상태만으로 충전 진행 여부는 단정하지 않음 |

현재 배터리 전압이 이미 만충에 가까우므로, 충전 기능 자체를 확인하려면 배터리 전압을 낮춘 뒤 USB 연결 후 `voltage_mv`가 회복되는지 다시 관찰합니다.

---

## Bluetooth 확인 기준

ESP32-S3는 BLE peripheral advertising 기준으로 Snow 발견/연결을 확인합니다. 이번 항목은 폰에서 기기를 발견하고 연결/해제하는 것까지를 범위로 두며, 데이터 송수신은 다음 `BLE 통신` 항목에서 분리해 검증합니다.

펌웨어 기준:

| 항목 | 값 |
| --- | --- |
| firmware version | `0.0.4` |
| feature marker | `ble_advertising` |
| BLE device name | `Snow` |
| BLE service UUID | `7d8c0f2a-6f8a-4d4c-9d4a-0a2c0f8b1540` |

확인 이벤트:

| 이벤트 | 판정 기준 |
| --- | --- |
| `bluetooth_advertising_started` | BLE advertising 시작 확인 |
| `bluetooth_client_connected` | 폰 앱에서 Snow 연결 확인 |
| `bluetooth_client_disconnected` | 폰 앱 연결 해제 확인. 이후 advertising 재시작 확인 |

테스트 절차:

1. Arduino IDE에서 `snow-status-card.ino`를 업로드합니다.
2. Serial Monitor `115200`에서 `firmware_version`의 `version`이 `0.0.4`이고 `features`에 `ble_advertising`이 있는지 확인합니다.
3. iPhone 또는 Android의 BLE scanner 앱에서 `Snow`를 검색합니다.
4. `Snow`에 연결한 뒤 Serial Monitor에서 `bluetooth_client_connected`를 확인합니다.
5. 연결을 해제한 뒤 `bluetooth_client_disconnected`와 `bluetooth_advertising_started` 재시작 로그를 확인합니다.

---

## I2C scan 확인 기준

`i2c_scan_done` 이벤트의 `details.addresses`에서 실제 응답한 I2C 주소를 확인합니다.

예:

```json
{"found_count":2,"addresses":"0x51,0x70"}
```

판정 기준:

| 주소 | 확인된/예상 장치 | 비고 |
| --- | --- | --- |
| `0x18` | ES8311 audio codec | 공식 audio 예제의 ES8311 주소 |
| `0x51` | PCF85063 RTC | 공식 RTC 예제 대상 |
| `0x70` | SHTC3 온습도 센서 | 공식 온습도 예제 대상 |
| `0x55` | BQ27220 fuel gauge | 현재 Waveshare 문서/예제 기준에서는 배터리 측정 경로로 사용하지 않음 |

Snow에서 확인된 I2C 주소가 `0x18,0x51,0x70`이면 audio codec, RTC, 온습도 센서가 I2C bus에 응답하는 상태로 봅니다.

`0x55`가 없어도 배터리가 빠졌다고 단정하지 않습니다. Waveshare ESP32-S3-ePaper-1.54G의 배터리 전압 측정은 공식 예제 기준 ADC 방식입니다.

---

## 개인정보/환경정보 제한

Serial telemetry에는 다음 값을 출력하지 않습니다.

- Wi-Fi SSID
- Wi-Fi password
- IP address
- MAC address
- token/API key
- private hostname

연결 여부와 재시도 횟수만 기록합니다.

---

## 적용 기준

새 기능을 추가할 때는 다음 기준을 따릅니다.

1. 화면에 필요한 값인지, Serial telemetry에만 필요한 값인지 먼저 구분합니다.
2. Serial 로그는 `telemetryLog()`를 사용합니다.
3. 임시 디버깅용 `Serial.print()`는 PR 전에 제거합니다.
4. `details`는 JSON object 문자열로 전달합니다.
5. 민감하거나 환경을 식별할 수 있는 값은 `details`에도 넣지 않습니다.
