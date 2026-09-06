# e-paper 문제 해결

## 1. Serial Monitor 로그가 보이지 않을 때

확인 순서:

1. Serial Monitor baud가 `115200`인지 확인합니다.
2. Arduino IDE 설정에서 `Tools > USB CDC On Boot > Enabled`인지 확인합니다.
3. 업로드 후 Serial Monitor를 닫았다가 다시 엽니다.
4. USB를 뽑고 몇 초 뒤 다시 연결합니다.
5. 선택한 포트가 현재 잡힌 `/dev/cu.usbmodem...` 포트인지 확인합니다.

기대 로그:

```json
{"level":"info","event":"system_start","message":"Snow status-card firmware started","uptime_ms":2000,"details":{"baudrate":115200}}
```

`system_heartbeat`만 계속 보이고 새로 추가한 이벤트가 보이지 않으면, 새 스케치가 보드에 올라가지 않았거나 Serial Monitor가 이전 실행 로그를 보고 있을 가능성이 있습니다.

## 2. macOS에서 보드가 보이지 않을 때

확인 순서:

1. `ls /dev/cu.*`를 실행합니다.
2. 케이블이 데이터 전송을 지원하는지 확인합니다.
3. 보드 전원 버튼 또는 부팅 상태를 확인합니다.
4. USB를 다시 연결합니다.
5. USB 장치 자체가 전혀 보이지 않으면 드라이버보다 케이블, 커넥터, 전원 상태를 먼저 확인합니다.

Snow가 정상적으로 잡히면 보통 다음 형태의 포트가 보입니다.

```text
/dev/cu.usbmodem...
/dev/tty.usbmodem...
```

## 3. 업로드 성공 기준

Arduino IDE에서 다음 로그가 보이면 보드 기록 검증까지 완료된 것으로 봅니다.

```text
Hash of data verified.
```

단, 업로드 성공 로그만으로 실제 실행 중인 firmware가 새 버전이라고 단정하지 않습니다. 업로드 후에는 Serial Monitor에서 부팅 시 출력되는 `firmware_version` 이벤트를 확인합니다.

예시:

```json
{"level":"info","event":"firmware_version","message":"Snow firmware version","uptime_ms":2017,"details":{"version":"0.0.3","sketch":"snow-status-card","features":"json_telemetry,battery_adc,i2c_scanner"}}
```

판정 기준:

1. `event`가 `firmware_version`인지 확인합니다.
2. `details.version`이 업로드한 버전과 일치하는지 확인합니다.
3. `details.features`에 이번에 확인하려는 기능 marker가 포함되어 있는지 확인합니다.
4. 버전이 기대값과 다르면 Arduino IDE에서 다른 스케치를 열었거나, 업로드가 실패했거나, 보드가 재부팅되지 않았을 가능성을 먼저 확인합니다.

현재 `main`에 반영된 상태 카드/배터리 UI 기준 버전은 `0.0.3`입니다. 이후 기능 테스트 브랜치에서는 새 기능을 구분하기 위해 버전이 올라갈 수 있습니다.

## 4. `secrets.h` 파일이 없다고 나올 때

오류 예시:

```text
fatal error: secrets.h: No such file or directory
```

해결 방법:

```bash
cp e-paper/ESP32S3/snow-status-card/secrets.example.h \
  e-paper/ESP32S3/snow-status-card/secrets.h
```

그 다음 `secrets.h`에 실제 Wi-Fi 값을 로컬에서만 입력합니다. `secrets.h`는 Git에 커밋하지 않습니다.

## 5. `firmware_version` 로그가 보이지 않을 때

`firmware_version`은 부팅 직후 1회만 출력됩니다. Serial Monitor를 늦게 열면 이미 지나간 로그를 놓칠 수 있습니다.

확인 순서:

1. Serial Monitor를 닫습니다.
2. USB를 뽑았다가 다시 연결합니다.
3. Serial Monitor를 바로 엽니다.
4. baud가 `115200`인지 확인합니다.
5. `firmware_version` 이벤트가 보이는지 확인합니다.

그래도 보이지 않으면 Arduino IDE에서 `e-paper/ESP32S3/snow-status-card/snow-status-card.ino` 파일을 열고 있는지 확인합니다.

## 6. 새 이벤트가 보이지 않고 `system_heartbeat`만 반복될 때

증상 예시:

```json
{"level":"info","event":"system_heartbeat","message":"Main loop heartbeat","uptime_ms":694662,"details":{"sequence":129}}
{"level":"info","event":"system_heartbeat","message":"Main loop heartbeat","uptime_ms":699663,"details":{"sequence":130}}
```

가능한 원인:

1. 새 firmware가 업로드되지 않았습니다.
2. Arduino IDE에서 다른 `.ino` 파일을 열고 업로드했습니다.
3. 업로드 후 보드가 재부팅되지 않았습니다.
4. Serial Monitor를 늦게 열어 부팅 로그를 놓쳤습니다.
5. 이전 firmware도 heartbeat를 출력하므로, heartbeat만으로는 최신 firmware 실행을 확인할 수 없습니다.

해결 기준:

- 최신 firmware 여부는 `system_heartbeat`가 아니라 `firmware_version` 이벤트로 확인합니다.
- 기능별 확인은 해당 이벤트가 실제로 나오는지 봅니다.
  - 배터리 ADC: `battery_adc_init`, `battery_voltage_read`
  - I2C scan: `i2c_scan_start`, `i2c_scan_done`
  - Bluetooth advertising: `bluetooth_advertising_started`

## 7. 포트가 사용 중이라고 나올 때

증상 예시:

```text
Resource busy
```

가능한 원인:

- Arduino IDE Serial Monitor가 `/dev/cu.usbmodem...` 포트를 이미 잡고 있습니다.
- 다른 터미널 프로세스가 같은 포트를 읽고 있습니다.

해결 방법:

1. Arduino IDE Serial Monitor를 닫습니다.
2. 다시 포트를 읽습니다.
3. 그래도 막히면 포트를 사용하는 프로세스를 확인한 뒤 종료합니다.

## 8. e-paper 화면은 바뀌었는데 Serial 로그 버전이 다를 때

e-paper는 전원이 꺼져도 마지막 화면을 유지할 수 있습니다. 따라서 화면에 보이는 내용만으로 현재 실행 중인 firmware를 판단하지 않습니다.

판정 기준:

1. 실제 실행 firmware는 Serial Monitor의 `firmware_version`으로 확인합니다.
2. 화면 확인이 필요한 기능은 사진으로 확인합니다.
3. 두 조건이 모두 맞을 때 해당 UI 변경을 검증 완료로 봅니다.
