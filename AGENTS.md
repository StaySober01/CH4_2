# CH4_2 작업 규칙

## 범위와 기준
- UE 5.6 연결 프로젝트(설치 엔진 5.6.1), C++, Listen Server, 기본 2인 차량 경쟁 게임.
- 요구사항 원문은 `docs/assignment.md`, 추적 표는 `docs/implementation-plan.md`를 따른다.
- 현재 UE 5.6 Vehicle 템플릿을 CH4_2 모듈에 도입했다. 코인/채팅/매치 등 과제 게임 규칙은 아직 없다.
- 사용자가 다음 단계를 지시하기 전에는 단계 경계를 넘지 않는다. 이번 승인 범위는 Vehicle 기반 도입이다.

## 구조와 코드
- `Source/CH4_2`: 단일 Runtime 모듈. `Config`: 설정. `Content`: 차량·입력·맵 에셋. `Scripts`: 엔진 맵 구성·접속 검사. `docs`: 설계·검증 기록.
- 기존 코드를 먼저 읽는다. UE 명명 규칙(A/U/F/E, bool의 b 접두사), generated.h 마지막 include, 명시적 override를 따른다.
- 작은 책임 단위로 수정하고 관련 없는 리팩터링, 플러그인 도입, 설정 변경을 피한다.
- API는 설치된 엔진 소스와 현재 모듈 의존성으로 확인한다. 수치·정책을 명세처럼 임의 확정하지 않는다.

## 네트워크와 수명
- 코인, 버프, 공격, 아이템 생성·획득·파괴, 타이머, 승자 판정은 서버 권한으로 처리한다.
- Server RPC는 소유 PC/Pawn을 경유한다. Authority와 로컬 제어, 네트워크 Ownership을 구별한다.
- 요청의 상태·소유 관계·대상·거리·빈도·중복을 서버에서 검사한다. 클라이언트가 보낸 피해량·보상·명중 결과를 신뢰하지 않는다.
- Coins/BonusMaxSpeed는 COND_OwnerOnly. 공용 상태는 GameState. 지속 상태를 Multicast만으로 전달하지 않는다.
- RepNotify와 서버 변경이 공통 UI 이벤트에 도달하게 한다. 위젯은 로컬 컨트롤러만 생성한다.
- UObject 강한 참조는 UPROPERTY/TObjectPtr, 비소유 참조는 필요에 따라 TWeakObjectPtr. 사용 전 유효성을 확인한다.
- EndPlay에서 Timer/Delegate를 정리한다. 파괴·중복 오버랩·접속 종료를 방어한다. 불필요한 Tick을 추가하지 않는다.

## 절차와 완료 조건
- Understand → Plan → Implement → Verify → Document → Report 순서. 단계 목표·수정 파일·흐름·위험·완료 기준을 먼저 기록한다.
- 빌드 명령 및 PIE 절차는 `docs/testing.md`. C++/UHT 성공과 실제 멀티플레이 검증을 구별한다.
- 요구사항 상태: TODO / IN_PROGRESS / IMPLEMENTED / VERIFIED / BLOCKED. 구현 파일·검증 증거를 함께 갱신한다.
- 완료 조건: 승인된 범위 구현, 관련 검사 수행, 미검증·실패 공개, 요구사항 표·progress·testing 갱신. 주요 결정은 decisions에 기록한다.
- 테스트하지 않은 기능을 VERIFIED로 표시하지 않는다. 바이너리 에셋을 텍스트로 조작하거나 외부 파일을 임의 삭제하지 않는다.
- 결과는 작업 결과 / 핵심 설계 / 검증 / 다음 단계 순서로 설명하고 다음 단계 지시를 기다린다.

상세: `docs/architecture.md`, `docs/networking.md`, `docs/decisions.md`.

