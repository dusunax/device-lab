# device-lab

작은 기기와 디스플레이 기능을 검증하는 저장소입니다.

## 범위

- e-paper 기기
- AMOLED 기기
- ESP32 기반 보드
- 기기별 펌웨어 실험
- 하드웨어 설정과 문제 해결 문서

## 기기 이름 규칙

각 물리 기기는 `devices/` 아래에서 짧은 별칭으로 관리합니다.

| 별칭 | 기기 유형 | 하드웨어 |
| --- | --- | --- |
| `snow` | e-paper | Waveshare ESP32-S3-ePaper-1.54G |

## 저장소 구조

```text
devices/
  snow/
    snow_status_card/

vendor/
  waveshare_epaper_1in54g/

docs/
  snow/
  shared/
```

## 구조 원칙

- 기기별 실행 코드는 `devices/{alias}/` 아래에 둡니다.
- 외부 기반 드라이버 코드는 `vendor/` 아래에 둡니다.
- 문서는 모두 `docs/` 아래에서 관리합니다.
- 특정 기기 문서는 `docs/{alias}/`에 둡니다.
- 여러 기기에 공통으로 쓰는 문서는 `docs/shared/`에 둡니다.
- 빈 폴더 유지만을 위한 `.gitkeep`은 두지 않습니다.
- 폴더 깊이와 폴더 개수는 실제 필요가 생길 때만 늘립니다.

## 보안 원칙

Wi-Fi 자격정보, 토큰, 로컬 IP 주소, MAC 주소 등 환경 식별 정보는 커밋하지 않습니다.

Git에는 `secrets.example.h`만 포함하고, 실제 로컬 설정인 `secrets.h`는 추적하지 않습니다.
