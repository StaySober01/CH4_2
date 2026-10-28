# M03 경기 상태·타이머·공용 HUD

## 계획 및 정책
- 범위: GM-001~003, GM-005~010. 원문의 첫 접속 시작 조건은 사용자 지시로 2명 접속으로 변경한다. 원문 assignment.md는 보존한다.
- 두 명이 접속하면 Waiting 5초 → Playing 60초 → Finished 0초. 대기 중 2명 미만이면 취소, 다시 모이면 5초부터 시작.
- 경기 중 이탈은 남은 시간 진행 유지. 후입장은 현재 상태를 받으며 타이머를 재시작하지 않는다. 종료 후 자동 재시작 없음. 세 번째 접속을 자동 차단하는 정책은 추가하지 않는다.
- 신규 GameState에 상태/정수 남은 시간/접속 인원/카운트다운 활성 여부를 한 구조체로 복제한다. 부분적으로 서로 다른 단계/시간을 표시하지 않도록 공통 RepNotify 이벤트를 사용한다.
- 기존 GameMode는 PostLogin/Logout으로 참가 PC를 추적하고 서버 1초 타이머로만 상태를 변경한다. EndPlay에서 타이머 해제.
- 기존 개인 HUD에 경기 상태/시간 행을 추가. PC가 World의 GameStateSetEvent와 GameState 변경 이벤트를 구독하고 현재 스냅샷을 읽는다. 로컬 Tick으로 카운트다운하지 않는다.
- 변경 파일: CH4_2GameState.h/.cpp 신규, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp.
- 위험: GameStateClass BP override, GameState가 PC보다 늦게 도착, PostLogin이 BeginPlay보다 빠른 경우, Logout 중 잔존 객체, 5초 타이머 중복.
- 승자 목록/결과는 M09, 상태별 운전 잠금은 M13이다. Finished는 이번에 종료 상태만 표시한다.

## 사용자 검증 절차
1. 에디터를 종료한 뒤 Development Editor / Win64 전체 빌드. 새 UCLASS/구조체가 있으므로 Live Coding만으로 적용하지 않는다.
2. BP_VehicleAdvGameMode Class Defaults에서 Game State Class = CH4_2GameState, Player State Class = CH4_2PlayerState 확인. World Settings의 GameMode Override도 BP_VehicleAdvGameMode여야 한다.
3. 1 Player / Listen Server PIE: Waiting for players: 1/2 표시, 5초 이상 지나도 시작하지 않아야 한다.
4. 2 Players / Listen Server PIE: 두 창에서 Waiting: 5s부터 감소, 약 5초 후 Playing: 60s, 60초 후 Finished: 0s. 네트워크 전달 지연으로 순간적인 표시 차이는 가능하지만 클라이언트 독립 카운트다운이나 지속적인 상태 차이는 없어야 한다.
5. Waiting 중 Client에서 콘솔 disconnect(에디터 전체 Stop 대신 해당 연결 종료): Host가 Waiting for players: 1/2로 복귀. 같은 클라이언트에서 open 127.0.0.1:<호스트 PIE 포트>로 재접속하면 다시 5초 시작. 실제 포트는 호스트 Output Log의 listening on port 행을 사용하며 7777로 고정 가정하지 않는다. 연결 해제 후 콘솔을 사용할 수 없으면 별도 클라이언트 세션으로 재접속한다.
6. Playing 중 Client 이탈: Host 시간 계속 감소. 후입장 Client는 현재 단계/시간 표시, 새로운 60초 시작 금지.
7. Finished 이후 시간이 음수가 되거나 자동으로 다시 시작하지 않음. PIE 종료/재시작 시 초기 Waiting, 타이머 가속/중복 HUD 없음.
8. M02 개인 수치 표시와 콘솔 변경이 계속 정상 동작하는지도 확인.

문제 발생 시 첫 UHT/C++ 오류, 어느 창인지, 표시 상태/남은 시간, 접속/이탈 순서와 LogCH4_2의 Match 행을 전달한다. 에이전트는 빌드·검증을 실행하지 않는다.

## 상태
VERIFIED / 2026-10-21 사용자 “검증 완료” 보고. 개별 실행 로그는 제공받지 않았으며 에이전트가 빌드·검증을 실행한 것은 아니다.


## M09 정책 변경 — 2026-10-27
사용자 지시로 Playing 중 참가자가 이탈하면 남은 참가자가 즉시 승리하며 Finished/0초로 전환한다. 이전 시간 계속 진행 정책은 과거 이력이다. Waiting 이탈 시 취소·재충족 5초 재시작은 유지한다. 새 수용 절차는 docs/m09-match-result.md이며 M03/GM-010은 해당 회귀 사용자 확인 대기다.
