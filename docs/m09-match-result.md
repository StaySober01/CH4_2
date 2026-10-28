# M09 승패·공동 우승

## 구현 전 계획 — 2026-10-27
- 목표: Playing 60초 종료 시 서버 Coins 비교, 최고점 동률 전원 우승, 로컬 승패 WBP 표시.
- GameState의 기존 단일 MatchSnapshot에 결과 확정 여부·참가 ID·우승자 ID/이름 목록을 추가. Coins는 공개하지 않는다.
- 결과는 종료 시점 스냅샷이며 이후 코인/이름 변경이나 우승자 이탈로 다시 계산하지 않는다.
- 기존 WBP_PlayerStats의 MatchText에 종료 결과를 표시하여 새 위젯 생성/런타임 트리 작성 없이 Designer 배치를 유지한다.
- 수정: GameMode.h/.cpp, GameState.h, PlayerState.h/.cpp, PlayerStatsWidget.h/.cpp, PlayerController.cpp.
- 중도 이탈자 포함 여부는 사용자 답변 대기. 그 외 판정 코드 적용 전에 확정한다.
- 위험: 전원 0원/참가자 없음, 후입장, PlayerState/결과 복제 순서, 종료 후 데이터 변화, 긴 이름/중복 이름.
- 완료 기준: 단독 승리/패배, 동점/전원 0원 공동 우승, 후입장/이탈 결과 유지, 기존 타이머/게임플레이 회귀.
- 에이전트 빌드/UHT/PIE/작업 후 검사 미실행. 사용자 전체 빌드·WBP 표시/2인 PIE 확인 안내.

## 확정 정책 및 구현
- 사용자 결정: Playing 중 참가자가 이탈하면 남아 있는 플레이어 즉시 승리. Waiting 중 이탈은 기존 카운트다운 취소/재충족 시 5초 재시작. Finished 이후 이탈은 결과 유지.
- 기본 2인 경기에서 남은 한 명은 코인 잔액과 무관하게 승리. 현재 3명 이상 접속 차단은 없으므로 이 경우에는 이탈 시점의 남은 유효 참가자 전원이 공동 우승한다.
- 시간 종료는 종료 시 유효한 접속 참가자 Coins 비교. 최고 동점 모두 우승, 전원 0원도 공동 우승. 아무도 없으면 No eligible participants.
- 종료 이유 TimeExpired/PlayerLeft, 결과 확정 여부, 참가 PlayerId 배열과 우승자 PlayerId/이름을 기존 MatchSnapshot에 함께 복제한다. 코인 금액은 포함하지 않는다.
- 이름은 종료 시점의 64자 한 줄 스냅샷. 동명 플레이어는 (#PlayerId)로 구별한다. 목록은 ID 순 정렬.
- GameMode가 결과를 한 번만 확정하며 종료 후 접속/이탈은 연결 인원만 갱신한다. 후입장자는 기존 우승자 목록과 Not a participant in this result 표시.
- PC가 본인 PlayerId를 WBP에 전달한다. PlayerState OnRep_PlayerId도 기존 UI 이벤트로 갱신하여 GameState와 도착 순서가 달라도 최종 승패를 다시 계산한다.
- 기존 WBP MatchText에 Finished / Victory!·Defeat·Joint Victory! / 종료 이유 / 우승자 목록 표시. 기존 Coins/BonusSpeedText는 유지한다.
- 종료 후 코인 획득·공격·스포너는 아직 가능하지만 확정 결과는 바뀌지 않는다. 상태별 입력 제한은 M13에서 처리한다.
- 새 Tick/타이머/RPC/플러그인/런타임 위젯 트리 생성 없음. 기존 서버 타이머와 GameState 이벤트/RepNotify 경로 사용.
- 초기 계획의 이탈 정책 응답 대기 항목은 위 사용자 결정으로 해결됐다.

## 사용자 빌드 및 WBP 확인
1. 에디터 종료 후 Development Editor / Win64 전체 빌드. Live Coding은 비활성화 상태를 유지한다.

    & 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' CH4_2Editor Win64 Development '-Project=C:\UnrealProject\CH4_2\CH4_2.uproject' -WaitMutex -NoHotReloadFromIDE

2. WBP_PlayerStats를 열어 Compile/Save. 새 위젯 생성/생성 스크립트 재실행은 필요 없다.
3. MatchText가 최소 3줄을 표시할 공간이 있는지 확인. 기존 자동 크기 패널이면 그대로 사용하고, 고정 높이를 넣었다면 Designer에서 늘린다. 긴 이름은 Auto Wrap Text와 적당한 폭(예: 500px)을 설정할 수 있다. Is Variable 유지.
4. 기존 BP GameMode의 GameStateClass는 CH4_2GameState, PlayerStateClass는 CH4_2PlayerState 유지. 결과를 계산하는 별도 BP Tick/Event 로직은 만들지 않는다.

## 사용자 검증 순서
각 시간 종료 시나리오는 새 PIE에서 2 Players / Play As Listen Server / New Editor Window (PIE)로 진행한다. 차량은 아이템 생성 위치에서 벗어나 정차해 둔다.
호스트 콘솔 CHStatsList로 H/C PlayerId를 확인하고 아래 자리표시자를 실제 숫자로 바꾼다. 디버그 명령으로 수치를 준비한 뒤에는 공격/아이템 획득으로 의도치 않게 잔액을 바꾸지 않는다.

1. 단독 Host 승리: CHStatsAddCoins H 10. 5초 대기+60초 경기 종료 시 Host Victory!, Client Defeat, 두 창의 우승자 목록은 Host만.
2. 단독 Client 승리: 새 PIE에서 CHStatsAddCoins C 10. Client Victory!, Host Defeat.
3. 동점: 새 PIE에서 H/C 각 10원. 종료 시 양쪽 Joint Victory!, 두 이름 모두 표시.
4. 전원 0원: 새 PIE에서 아무것도 획득하지 않고 종료. 양쪽 Joint Victory!, 둘 모두 목록에 포함.
5. Playing 중 Client 이탈: Client에게 더 많은 코인을 줘도, Client 창 종료/접속 종료 직후 Host가 Finished / Victory! / Player left가 되어야 한다. 타이머는 0, 60초까지 계속 기다리지 않는다.
6. Waiting 중 Client 이탈: Finished로 가지 않고 인원 대기 복귀. 다시 2명이 모이면 5초부터 재시작.
7. 결과 확정 후 코인 획득/공격/호스트 디버그 명령으로 순위 역전: 현재 개인 Coins는 바뀔 수 있지만 우승자/승패는 그대로.
8. Client가 우승한 뒤 Client 종료: Host 화면의 Client 우승 이름/ID와 Defeat 결과 유지.
9. 종료된 Listen Server에 별도 클라이언트로 후입장할 수 있는 환경이면 기존 우승자 목록 수신 및 Not a participant in this result 확인. 일반 새 PIE 재시작은 후입장 테스트가 아니다.
10. 기존 채팅·운전 복귀·속도/아이템/2초 스포너/공격 회귀 확인. Waiting/Finished 운전·공격 잠금은 현재 기대값이 아니다.
11. Listen Host 프로세스 종료는 서버 종료이므로 남은 Client에게 승리를 통지할 수 없다. 즉시 승리 테스트는 살아 있는 서버에서 원격 참가자가 나가는 경우로 수행한다.

## 상태와 오류 전달
- M09/GM-004/GM-011~013 IMPLEMENTED(사용자 검증 대기).
- M03의 경기 중 이탈 정책을 변경했으므로 M03/GM-010도 IMPLEMENTED로 재검증 대기. 과거 사용자 통과 이력은 보존한다.
- 에이전트 빌드/UHT/PIE/작업 후 검증 명령 미실행.
- 실패 시 첫 빌드 오류, Host/Client 구분, 종료 직전 잔액/실행 명령, 표시된 결과, LogCH4_2의 Match result finalized 및 Match winner 행, Saved/Logs/CH4_2.log를 전달한다.
- 다음 지시 후 M10 필수 기능 통합 검증·결함 수정(검증 실행은 사용자 담당).

## M09 사용자 수용 결과 — 2026-10-28
- 사용자 보고: M09 검증 완료. M09 및 GM-004/011~013을 VERIFIED로 갱신한다.
- 안내된 경기 중 이탈 즉시 승리/Finished 전환을 포함한 M09 수용 보고로 M03/GM-010을 VERIFIED로 복원한다.
- 검증 근거는 사용자 완료 보고이며 개별 시나리오 로그는 별도 제공되지 않았다. 선택적 후입장 및 3인 이상 시나리오까지 검증됐다고 확대하지 않는다.
- 에이전트 빌드/UHT/테스트/PIE/작업 후 검증 명령은 실행하지 않았다. 기존 구현 시점 기록은 이력으로 보존한다.
- M10 통합 검증·결함 수정은 다음 사용자 지시 후 진행한다.