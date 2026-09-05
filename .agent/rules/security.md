# 보안 규칙

## secrets

다음 파일과 값은 Git에 커밋하지 않습니다.

```text
secrets.h
.env
Wi-Fi SSID
Wi-Fi password
API key
token
local IP address
MAC address
private hostname
```

Git에는 예시 파일만 포함합니다.

```text
secrets.example.h
```

로컬 컴파일이 필요하면 예시 파일을 복사해서 `secrets.h`를 만들고, 실제 값은 로컬에서만 입력합니다.

```bash
cp e-paper/ESP32S3/snow-status-card/secrets.example.h \
  e-paper/ESP32S3/snow-status-card/secrets.h
```

## e-paper 출력 정책

Snow의 e-paper 화면과 Serial Monitor에는 IP 주소를 출력하지 않습니다.
연결 여부는 `WIFI OK`, `WIFI FAIL`처럼 상태로만 표시합니다.

## 커밋 전 확인

커밋 전 다음을 확인합니다.

```bash
git check-ignore -v e-paper/ESP32S3/snow-status-card/secrets.h
```

추적 대상 파일에 자격정보나 IP 주소가 포함되어 있지 않은지 확인합니다.
