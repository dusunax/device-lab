# e-paper 문제 해결

## 1. Serial Monitor 로그가 보이지 않을 때

확인 순서:

1. Serial Monitor baud가 `115200`인지 확인합니다.
2. Arduino IDE 설정에서 `Tools > USB CDC On Boot > Enabled`인지 확인합니다.
3. 업로드 후 Serial Monitor를 닫았다가 다시 엽니다.
4. USB를 뽑고 몇 초 뒤 다시 연결합니다.
5. 선택한 포트가 현재 잡힌 `/dev/cu.usbmodem...` 포트인지 확인합니다.

## 2. macOS에서 보드가 보이지 않을 때

확인 순서:

1. `ls /dev/cu.*`를 실행합니다.
2. 케이블이 데이터 전송을 지원하는지 확인합니다.
3. 보드 전원 버튼 또는 부팅 상태를 확인합니다.
4. USB를 다시 연결합니다.
5. USB 장치 자체가 전혀 보이지 않으면 드라이버보다 케이블, 커넥터, 전원 상태를 먼저 확인합니다.

## 3. 업로드 성공 기준

Arduino IDE에서 다음 로그가 보이면 보드 기록 검증까지 완료된 것으로 봅니다.

```text
Hash of data verified.
```

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
