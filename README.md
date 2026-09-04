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
  {alias}/
    README.md
    arduino/
    docs/

experiments/
  epaper/
  amoled/
  wifi/
  audio/

shared/
  arduino/
  docs/

references/
```

## 문서 관리 원칙

- 특정 기기 문서는 `devices/{alias}/docs/`에 둡니다.
- 최상위 `docs/`에는 특정 기기 문서를 복제하지 않습니다.
- 여러 기기에서 공통으로 쓰는 문서만 `shared/docs/`로 분리합니다.
- 체크리스트 상태는 의도나 계획이 아니라 실제 확인 결과 기준으로 표시합니다.

## 보안 원칙

Wi-Fi 자격정보, 토큰, 로컬 IP 주소, MAC 주소 등 환경 식별 정보는 커밋하지 않습니다.

Git에는 `secrets.example.h`만 포함하고, 실제 로컬 설정인 `secrets.h`는 추적하지 않습니다.
