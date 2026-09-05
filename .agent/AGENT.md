# Agent Configuration

> 이 저장소에서 작업하는 AI 에이전트는 작업 시작 전에 이 파일과 하위 rules를 읽습니다.

`.agent/`는 device-lab의 에이전트 작업 규칙을 기록하는 위치입니다.
현재는 PR 본문, git, 보안, 문서 구조처럼 반복적으로 필요한 최소 규칙만 둡니다.

---

## 이 저장소

`device-lab`은 작은 하드웨어와 디스플레이 기기 실험을 관리하는 저장소입니다.

e-paper만을 위한 저장소가 아니며, 이후 AMOLED 등 다른 기기 유형도 추가될 수 있습니다.

---

## 현재 구조 기준

```text
e-paper/
  ESP32S3/
    snow-status-card/
      snow-status-card.ino
      secrets.example.h
      src/
        waveshare_epaper_1in54g -> ../../../../vendor/waveshare_epaper_1in54g

vendor/
  waveshare_epaper_1in54g/

docs/
  e-paper/
  shared/
```

구조 원칙:

- 루트에 `device/`를 추가로 감싸지 않습니다.
- 중간 계층은 보드/기기 계열명으로 둡니다. 예: `ESP32S3`.
- 별칭은 스케치명 prefix로 사용합니다. 예: `snow-status-card`.
- 빈 폴더 유지를 위한 `.gitkeep`은 만들지 않습니다.
- `references/`처럼 목적이 불명확한 빈 폴더는 만들지 않습니다.
- 폴더 깊이와 폴더 개수는 실제 필요가 생길 때만 늘립니다.

---

## 필수 Rules 읽기 순서

| 순서 | 파일 | 내용 |
| --- | --- | --- |
| 1 | [rules/security.md](rules/security.md) | secrets, Wi-Fi 정보, IP 주소 보호 |
| 2 | [rules/git.md](rules/git.md) | 커밋/브랜치/PR 본문 규칙 |
| 3 | [rules/docs.md](rules/docs.md) | 문서 위치와 한글 문서 기준 |

규칙 충돌 시 `security` > `git` > `docs` 순서로 적용합니다.

커밋 prefix/type은 영어 conventional commit 형식을 사용하고, 요약은 한국어로 작성합니다.

---

## PR 본문 기본 형식

ux-lab의 일반 PR 형식을 참고하되, device-lab에서는 현재 PR 본문 흐름을 기준으로 아래 순서를 사용합니다.

```markdown
## 개요

- {기기 별칭과 보드 계층을 포함한 변경 목적 — 1~3줄}
- {경로/기기명/별칭 기준처럼 리뷰어가 먼저 알아야 할 결정}

| 기능 | 테스트 목표 | 구현된 사항 | 상태 |
| --- | --- | --- | :-: |
| {기능명} | {검증 목표} | {이번 PR에서 구현된 사항} | ✅ |

## 캡처 자료

| {상태 A 캡처 설명} | {상태 B 캡처 설명} |
| --- | --- |
| {이미지 또는 아직 없음} | {이미지 또는 아직 없음} |

## 변경 내용

- {주요 변경 항목}

---

## 테스트

| 항목 | 값 |
| --- | --- |
| 기기 유형 | {device class} |
| 보드 | {board / hardware model} |

- [x] {실제 수행한 검증}
- [ ] {아직 사람/하드웨어 확인이 필요한 검증}

## 컴파일 결과:

```text
{Arduino compile 결과}
```
```

작성 기준:

- PR 상단에는 기기 별칭과 보드 계층을 함께 적습니다. 예: `Snow(ESP32S3)`.
- PR 상단에는 이번 PR에서 완료/검증한 핵심 기능 테이블을 둡니다.
- 테이블의 `구현된 사항`에는 별도 설명 섹션 대신 완료 항목의 실제 구현 내용을 적습니다.
- 캡처 자료는 `변경 내용`보다 위에 둡니다.
- 캡처 자료는 필요 시 GitHub user-attachments 이미지 태그나 표로 정리합니다.
- `변경 내용` 뒤에는 구분선 `---`을 두고 테스트 영역을 분리할 수 있습니다.
- `테스트`에는 기기 유형/보드 표를 먼저 두고, 실제 수행한 검증과 아직 사람이 확인해야 하는 검증을 분리합니다.
- Arduino 컴파일 결과는 `## 컴파일 결과:` 섹션으로 분리합니다.
- 확인하지 않은 하드웨어 동작은 `[x]`로 표시하지 않습니다.
- PR 본문은 실제 커밋과 diff를 기준으로 작성합니다.
- 작성 후 GitHub에서 PR 본문을 다시 읽어 최신 내용이 반영됐는지 확인합니다.
