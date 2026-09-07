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

## 9. BLE 연결/페어링 도중 보드 전체가 멈출 때

증상: BLE 연결 직후 heartbeat를 포함해 Serial 로그가 완전히 멈추고, 페어링도 타임아웃으로 실패한다.

원인: `showOpenFace()`(e-paper 풀 리프레시, 15~20초 소요, 재진입 불가)를 BLE 콜백(`onConnect`,
`onPassKeyNotify`, `onAuthenticationComplete`)에서 직접 호출했을 때, 두 콜백이 리프레시 시간
간격보다 가깝게 연달아 발생하면(예: 연결 직후 바로 페어링이 시작되는 경우) 같은 NimBLE 호스트
태스크에서 e-paper 드라이버가 두 번째로 재진입되어 멈춘다. BLE 호스트 태스크가 멈추면 이후 BLE
프로토콜 처리(페어링 완료 포함)도 진행되지 않고, 메인 루프까지 함께 정지하는 것으로 관찰됐다.

해결: BLE 콜백은 `redrawRequested` 플래그만 세우고, 실제 `showOpenFace()` 호출은 `loop()`(메인
태스크)에서만 수행하도록 분리했다. BLE 콜백은 즉시 반환하므로 호스트 태스크를 블로킹하지 않는다.

확인 방법: 보드가 멈추면 소프트 리셋(예: `esptool --after hard_reset chip_id`) 후 재시도.
정상이라면 페어링 중에도 heartbeat가 5초 간격으로 계속 찍힌다.

## 10. 페어링 패스키가 매번 같은 값처럼 보일 때

한 세션에서 리셋 없이 연속 두 번 페어링을 시도했을 때 같은 패스키(`100924`)가 나온 적이 있었다.
이후 정상 리셋을 거친 다음 시도에서는 다른 값(`656029`)이 나와, 매번 고정되는 문제는 아닌 것으로
보인다. 다만 `randomSeed()`를 호출하지 않고 있어 시드가 예측 가능한 상태일 수 있다는 점은 남아있는
확인 필요 사항이다.

## 11. `cat`/`screen`으로 Serial을 열었는데 로그가 비어있거나 앞부분이 깨질 때

증상: 리셋 직후 바로 `cat`이나 `screen`으로 포트를 열면, 첫 한두 줄의 JSON이 중간부터 잘려
나오거나(`{"level":"info"i2c_scan_start"...`처럼 필드 하나가 통째로 사라짐), 아예 아무 로그도
안 찍힌다.

원인 추정: 리셋 펄스 직후 포트를 여는 타이밍이 겹치면 발생하는 것으로 보인다. 재현 조건을 완전히
특정하지는 못했다.

해결: 리셋 후 2~3초 정도 대기했다가 Serial 리더를 여는 것으로 재현 빈도가 크게 줄었다. 그래도
비거나 깨지면 Arduino IDE Serial Monitor로 다시 확인한다 — 지금까지는 항상 정상으로 나왔다.
