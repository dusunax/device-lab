# e-paper 설정

## 대상 기기

| 항목 | 값 |
| --- | --- |
| 별칭 | `Snow` |
| 보드 | Waveshare ESP32-S3-ePaper-1.54G |
| 스케치 | `e-paper/ESP32S3/snow-status-card` |

## Arduino IDE 설정

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

## macOS 포트 확인

```bash
ls /dev/cu.*
```

기기 포트 후보:

```text
/dev/cu.usbmodem...
/dev/cu.usbserial...
/dev/cu.SLAB_USBtoUART
/dev/cu.wchusbserial...
```

## 로컬 secrets 설정

이 단계는 Arduino IDE 컴파일 전에 반드시 필요합니다.

예시 파일을 복사해서 로컬에서 수정합니다.

```bash
cp e-paper/ESP32S3/snow-status-card/secrets.example.h \
  e-paper/ESP32S3/snow-status-card/secrets.h
```

`secrets.h`는 Git 추적 대상이 아닙니다.

파일이 없으면 Arduino IDE에서 다음 오류가 발생합니다.

```text
fatal error: secrets.h: No such file or directory
```

## 코드 위치

Snow 상태 카드 코드는 다음 파일에 둡니다.

```text
e-paper/ESP32S3/snow-status-card/snow-status-card.ino
```

Waveshare 기반 드라이버 파일은 다음 위치에 둡니다.

```text
vendor/waveshare_epaper_1in54g/
```
