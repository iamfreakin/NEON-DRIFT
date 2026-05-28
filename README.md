# NEON DRIFT — 중력 없는 아케이드 웨이브 디펜스

▶︎ [랜딩 페이지](https://iamfreakin.github.io/NEON-DRIFT/) &nbsp;|&nbsp; ▶︎ [소스 코드](https://github.com/iamfreakin/NEON-DRIFT) &nbsp;|&nbsp; ▶︎ [구현 설계서](docs/NEONDRIFT_설계서.pdf) &nbsp;|&nbsp; ▶︎ [실행 파일 다운로드](https://drive.google.com/file/d/1C71W51GIf9KWpUrPd3QW4Mcj6TCb8lJ-/view?usp=sharing)

![게임플레이](docs/Images/몬스터방향%20공중.gif)

---

## 한 줄 요약

장르: 3D 아케이드 웨이브 디펜스 &nbsp;/&nbsp; 제작 기간: 3일 &nbsp;/&nbsp; 사용 기술: Unreal Engine 5.8 · C++20 &nbsp;/&nbsp; 1인 개발

---

## 핵심 기능

1. **7단계 상태머신** — 코드보다 설계가 먼저라고 생각한다. 전이 조건을 문서로 확정하고 코딩을 시작했고, 3일 내내 아키텍처 수정 0회·상태 충돌 버그 0건
2. **IDamageable 인터페이스** — 피격 대상 4종을 단일 인터페이스 캐스트로 처리, 새 대상 추가 시 발사 로직 수정 불필요
3. **Blueprint 0% 코드베이스** — Enhanced Input · HUD를 `.uasset` 없이 런타임 생성. 초기 비용이 있어도 전체 로직이 텍스트로 있어야 한다는 원칙을 끝까지 지켰고, `git diff` 한 줄로 모든 변경을 추적할 수 있다

---

## 내가 직접 만든 부분

- C++ 클래스 설계 및 전체 구현 (100%)
- 상태머신(GameMode), 6DOF 비행 물리(PlayerShip), 자원 채집 자석(ResourceShard), 포탑 탑승·조준(ManualTurret), HUD 전체(NeonHUD)
- 외부 에셋: UE5 내장 기본 지오메트리·머티리얼만 사용

---

## 기술적 도전

**문제**: 상점 진입 시 방향키·Enter 입력이 게임 로직에 전달되지 않음

**원인 추적**: UE 입력 파이프라인을 추적하니, `PlayerController → Enhanced Input` 경로와 `UMG 위젯 포커스 트리` 경로가 독립적으로 존재하며 UI 모드에서는 위젯 경로가 이벤트를 선점함을 확인. `FInputModeUIOnly()` 적용 상태에서는 키 이벤트가 게임 로직에 도달하기 전에 소비됨

**해결**: 임시 패치 대신 입력 경로 자체를 단일화 — 상점 UI를 UMG 위젯에서 Canvas HUD 직접 렌더링으로 교체하고 `FInputModeGameOnly()`를 유지. 모든 입력이 PlayerController 한 경로만 통과하게 되어 충돌 구조가 원천 제거됨

**배움**: 버그를 만났을 때 증상부터 막으면 빠르지만, 같은 문제가 다른 방식으로 돌아온다. 근본 원인을 찾아 구조를 바꾸는 게 결국 더 빠르다는 걸 이번에 확인했다.

→ [전체 트러블슈팅 및 설계 결정은 구현 설계서 참조](docs/NEONDRIFT_설계서.pdf)
→ [기획 의도 및 상세 내용은 게임 기획서 참조](docs/NEONDRIFT_기획서.pdf)

---

## 기술 스택

Unreal Engine 5.8 · C++20 · Enhanced Input System · Unreal Build Tool (CLI)

---

## 조작법

| 키 | 동작 |
|---|---|
| `W` `A` `S` `D` | 기체 이동 |
| `Space` / `Ctrl` | 상승 / 하강 |
| `마우스` | 시점 조준 |
| `좌클릭` | 발사 |
| `E` | 수동 포탑 탑승 / 해제 |
| `F` | 웨이브 준비 완료 |
| `↑` `↓` / `Enter` | 상점 항목 이동 / 구매 |
| `R` | 재시작 |

---

## 빌드 방법

1. UE 5.8 설치 후 `NEONDRIFT.uproject` 우클릭 → **Generate Visual Studio project files**
2. `Build.bat NEONDRIFTEditor Win64 Development -Project="<경로>/NEONDRIFT.uproject"` 실행
3. `.uproject`를 UE 5.8 에디터로 열고 **PIE(Play In Editor)** 실행

---

## 문서

- [구현 설계서](docs/NEONDRIFT_설계서.pdf) — 아키텍처 · 클래스 구조 · 트러블슈팅 전체
- [게임 기획서](docs/NEONDRIFT_기획서.pdf) — 기획 의도 · 규칙 · 밸런스
- [발표 자료](docs/NEONDRIFT_발표자료.pdf) — 발표 슬라이드

---

<div align="center">

© 2026 [iamfreakin](https://github.com/iamfreakin) · UE5 C++ Arcade Wave Defense · 3-Day Sprint

</div>
