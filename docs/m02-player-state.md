# M02: PlayerState와 개인 HUD

## 작업 계획
- 범위: PS-001~009. Coins(int32), BonusMaxSpeed(float), 서버 변경 API, OwnerOnly 복제, 개인 HUD.
- 신규 파일: Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp.
- 변경 파일: CH4_2GameMode.cpp(PlayerStateClass), CH4_2PlayerController.h/.cpp(로컬 패널과 이벤트 연결), CH4_2.Build.cs(SlateCore).
- 데이터 흐름: 서버 변경 API → 변경 이벤트 → Host HUD. 동일 값의 OwnerOnly 복제 → OnRep → 변경 이벤트 → 소유 Client HUD.
- PC는 BeginPlay/InitPlayerState/OnRep_PlayerState에서 연결하고 초기 스냅샷을 읽는다. 참조 변경/종료 시 구독 해제. 별도의 수치 폴링 Tick은 추가하지 않는다.
- UI는 C++ UUserWidget으로 좌측 상단 패널을 구성하여 기존 차량 속도·기어 WBP를 유지한다.
- 위험: BP GameMode에 남은 PlayerStateClass override, PS/HUD 도착 순서, Host의 OnRep 미호출, 클라이언트 임의 변경, 음수/오버플로 입력.
- 가정: 초기 값은 0. BonusMaxSpeed는 단위 미확정인 추가 속도 값으로 표시하며 실제 이동 적용은 M06. 음수 패널티 API는 M11에서 설계한다.
- 사용자 수용 기준: 서버 변경 및 소유 클라이언트 표시, 비소유 복제 제외, Host 즉시 표시, 잘못된 변경 거부, PIE 재시작/후입장 초기 표시. 빌드·실행은 사용자 담당.

## 검증을 위한 PIE 전용 콘솔 명령
변경 명령은 PIE 서버 권한이 있는 PC에서만 실행된다. Server RPC가 아니며 원격 클라이언트에서 실행해도 서버로 전달되지 않는다. Shipping에서는 동작하지 않는다. 값은 테스트 편의를 위한 예시이며 게임 규칙이 아니다.

- `CHStatsList`: 현재 월드의 PlayerId, 본인 여부, Coins, BonusMaxSpeed를 Output Log에 출력.
- `CHStatsAddCoins <PlayerId> <Amount>`: 서버에서 코인 증가.
- `CHStatsSpendCoins <PlayerId> <Amount>`: 서버에서 코인 차감.
- `CHStatsAddSpeed <PlayerId> <Amount>`: 서버에서 추가 속도 증가.

## 사용자가 수행할 순서
1. 에디터를 종료하고 CH4_2Editor / Development Editor / Win64로 전체 빌드한다. 새 UCLASS가 있으므로 이번 변경은 Live Coding만으로 적용하지 않는다.
2. VehicleBasic의 BP_VehicleAdvGameMode Class Defaults에서 Player State Class가 CH4_2PlayerState인지 확인한다. 기존 override가 있으면 해당 속성을 기본값으로 되돌리거나 CH4_2PlayerState로 지정한다.
3. 2 Players / Play As Listen Server / New Editor Window (PIE)로 시작한다. 각 창 좌측 상단 Coins: 0, Bonus max speed: 0 표시와 기존 차량 HUD를 확인한다.
4. 호스트 콘솔에서 CHStatsList를 실행하고 Output Log에서 두 PlayerId를 확인한다. 호스트 ID를 H, 클라이언트 ID를 C로 아래 예시의 자리표시자만 바꾼다.
5. 호스트 콘솔: CHStatsAddCoins H 10, CHStatsAddSpeed H 10. 호스트 개인 패널만 10/10이 된다.
6. 호스트 콘솔: CHStatsAddCoins C 20, CHStatsAddSpeed C 10. 클라이언트 개인 패널은 20/10, 호스트는 10/10을 유지한다.
7. 클라이언트 콘솔 CHStatsList: 본인 값은 20/10, 비소유 호스트 PS의 값은 초기 0/0이어야 한다. 호스트에는 서버 원본이 모두 있으므로 호스트 목록에 모든 값이 보이는 것이 정상이다. 초기 복제 안정화 후 확인한다.
8. 호스트 콘솔: CHStatsSpendCoins C 5 → 클라이언트 15원. CHStatsSpendCoins C 16, CHStatsAddCoins C -1, CHStatsSpendCoins C 0, CHStatsAddSpeed C -10은 거부되고 값 유지.
9. 클라이언트 콘솔 CHStatsAddCoins C 100 → 권한 거부 로그, 양쪽 값 유지. 이 검사는 명령의 권한 제한이며 setter 직접 호출 검증과는 구분한다.
10. 코인 오버플로 예: 현재 C가 15원인 상태에서 CHStatsAddCoins C 2147483647 → 거부 및 15원 유지. 필요 시 디버거/테스트 Blueprint의 비권한 경로에서 AddCoins/IncreaseBonusMaxSpeed 직접 호출도 false인지 확인한다. BlueprintAuthorityOnly 노드는 비권한 Blueprint 실행을 차단한다.
11. PIE 종료/재시작 시 초기 0/0, 중복 패널 없음. 후입장/PS 교체 시 초기 스냅샷 표시와 재바인딩은 해당 상황을 구성해 확인한다. M02는 재접속 데이터 보존을 추가하지 않는다.

성공·실패는 빌드/초기 UI/Host 변경/Client 변경/OwnerOnly/거부 입력으로 구분해 알려준다. 실패 시 첫 C++ 또는 UHT 오류 전문, 어느 창인지, 실행한 명령, 관련 LogCH4_2 행을 전달한다.

## 상태
VERIFIED(2026-10-21 사용자 “검증 완료” 보고). 개별 실행 로그는 제공받지 않았으며 사용자 수용 보고를 근거로 기록한다. 에이전트는 빌드, PIE, 자동 테스트, 작업 후 검사 명령을 실행하지 않는다.

