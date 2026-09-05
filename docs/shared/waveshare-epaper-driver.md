# Waveshare e-paper 드라이버

## 목적

e-paper 상태 카드에서 사용하는 Waveshare 기반 드라이버와 그리기 지원 파일의 위치와 정리 기준을 설명합니다.

## 위치

```text
vendor/waveshare_epaper_1in54g/
```

## 포함 파일

- `DEV_Config.*`: ESP32-S3 GPIO/SPI 설정 보조 코드
- `EPD_1in54g.*`: 1.54인치 e-paper 디스플레이 드라이버
- `GUI_Paint.*`: 도형/텍스트 그리기 유틸리티
- `fonts.h`, `font16.cpp`, `font20.cpp`, `font24.cpp`: 현재 상태 카드에서 사용하는 폰트

## 정리 기준

현재 구현에서 사용하지 않는 예제 이미지와 미사용 폰트 파일은 포함하지 않습니다.

기능 구현 코드는 `e-paper/ESP32S3/snow-status-card/snow-status-card.ino`에 두고, 외부 기반 코드는 `vendor/` 아래에 분리합니다.
