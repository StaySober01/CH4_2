# 구현 계획과 요구사항 추적

2026-10-21. Source of Truth는 assignment.md. 아래는 원문 체크박스를 순서대로 1:1 추적한다. Vehicle 템플릿 기반을 도입했고 M02의 PS-001~009는 사용자 완료 보고에 따라 VERIFIED다. M03의 GM-001~003/005~010은 사용자 완료 보고에 따라 VERIFIED, M04는 사용자 채팅·운전 복귀·기존 HUD 표시 확인 보고로 VERIFIED다. PS-009/GM-007/009도 WBP 전환 후 표시 확인을 반영했으며, M05 코인 획득은 2026-10-26 사용자 검증 완료 보고로 VERIFIED다. 그 외 후속 과제 기능은 TODO다. 예정 클래스는 Source/CH4_2/<이름>.h/.cpp, 에셋 경로는 Content 아래이며 확정된 구현 파일이 아니다. CHVehiclePawn/CHPlayerController/CHGameMode/CHHUDWidget 예정 역할은 도입된 CH4_2Pawn/CH4_2PlayerController/CH4_2GameMode/CH4_2UI를 확장하며 후속 구현 시 각 행을 실제 경로로 갱신한다.

## 작업 마일스톤

2026-10-21 재정리. 한 마일스톤은 목표 정리 → 구현 → 사용자 검증 절차 안내 → 문서 갱신 → 결과 보고의 작업 단위다. 빌드와 검증 실행은 전부 사용자가 담당한다. 아래 번호를 앞으로의 작업 지시 기준으로 사용한다. 기존 단계 1은 M00, 단계 2a는 M01에 해당하며, 이후의 기존 단계 번호는 이 표로 대체한다.

기능을 화면과 네트워크에서 확인할 수 있도록 관련 UI를 같은 작업에 포함한다. 각 마일스톤 종료 후 다음 지시를 기다리며, 계획 정리 자체는 후속 구현 승인이 아니다.

| ID | 작업 단위 / 주요 산출물 | 요구사항 | 선행 | 완료 기준 / 검증 | 현재 상태 |
|---|---|---|---|---|---|
| M00 | 프로젝트 분석·하네스: AGENTS, 명세, 책임/네트워크 설계, 요구사항 추적 | 기반 문서 | 없음 | 원문 보존, 65개 항목 대응, 계획·검증·미정 정책 기록 | VERIFIED |
| M01 | Vehicle 기반 및 2인 운전 검증: 기존 Pawn/PC/GameMode, 맵·입력·PlayerStart 3개 | BASE-001~003, BASE-005 | M00 | 빌드/접속/분리 스폰/참조 복제에 더해 Host·Client 실제 운전, 속도·기어 HUD, 점유 Start 예외 확인. Scalable/Starter Content 편차 및 Dedicated 안내 처리 방침 기록 | IN_PROGRESS |
| M02 | PlayerState와 개인 HUD: Coins/BonusMaxSpeed, 서버 변경 API, OwnerOnly 복제, 초기 표시·이벤트 갱신 | PS-001~009 | M01 | 서버 변경이 Host와 해당 소유 Client UI에 반영. 비소유 연결에 값 미복제, 잘못된 변경 거부. T-PS | VERIFIED |
| M03 | 경기 상태·타이머와 공용 HUD: GameState, Waiting/Playing/Finished, 2명 접속 후 5초 대기/60초 경기 | GM-001~003, GM-005~010 | M02 | 2명 접속 후 서버가 1초마다 감소, 단계 전환과 시간이 양쪽 UI에서 일치. 이 단계의 Finished는 종료 상태만 표시. T-MATCH 중 상태·시간 | VERIFIED |
| M04 | 채팅: C++ Widget/WBP, Enter 제출, 소유 PC Server RPC와 각 PC 수신 경로 | CHAT-001~006 | M02 | Host↔Client 양방향 한 번씩 표시, 빈 입력·길이·빈도 검증, 입력 포커스 복귀. T-CHAT | VERIFIED |
| M05 | 코인 획득: 코인 액터·충돌·효과/사운드, 서버 보상 및 파괴 | ITEM-001~004 | M02 | 획득자 +10원, NetUpdateFrequency=1, 중복 오버랩에도 한 번 지급, 양쪽 파괴·효과 확인. T-ITEM 코인 | VERIFIED |
| M06 | 속도 버프: 버프 액터, +10 데이터와 실제 차량 속도 적용, 효과/사운드 | ITEM-005~008 | M05 | NetUpdateFrequency=1, 한 번 획득·파괴, 개인 HUD +10과 합의된 단위/방식의 실제 속도 효과 확인. T-ITEM 버프, T-PS | TODO |
| M07 | 아이템 스포너와 맵 배치: 생성 참조, OnDestroyed, 2초 재생성 | SPAWN-001~004 | M05~06 | 서버에서 동일 종류 한 개만 재생성, 코인/버프 스포너 배치, 반복 획득·맵 종료 정리 확인. T-SPAWN | TODO |
| M08 | 차량 공격·탈취: 좌클릭 IA, Server RPC, 앞 범퍼 Trace, ApplyDamage, 선/피격 효과 | VEH-001~007 | M02, M05 | 잔액 0·4는 무변화, 5 이상은 -5/+5와 총합 보존. 범위·장애물·빈도 검사, 양쪽 효과 확인. T-ATTACK | TODO |
| M09 | 승패·공동 우승: 서버 Coins 비교, GameState 승자 목록, 로컬 결과 UI | GM-004, GM-011~013 | M03, M07~08 | 실제 수집/탈취 경기 종료 후 단독 우승·동률·전원 0원 결과를 양쪽에서 확인. 상대 Coins 공개 없이 결과 표시. T-MATCH 결과 | TODO |
| M10 | 필수 기능 통합 검증·결함 수정 | 필수 47개 전체, BASE-004 | M01~09 | 두 플레이어가 접속→대기→수집/공격/채팅→종료까지 수행. 필수별 구현 파일·검증 증거 확보, 미해결 필수 결함 없음 | TODO |
| M11 | 도전: 바퀴 부위 공격과 패널티 회복 | WHEEL-001~004 | M10 | ApplyPointDamage, 서버 Bone 판별, 음수 패널티가 0까지만 회복. 기존 버프와의 합의된 상호작용 확인. T-WHEEL | TODO |
| M12 | 도전: 지뢰 설치·가속 장애 | MINE-001~007 | M10 (권장 순서 M11 다음) | F키→서버 뒤쪽 Spawn, Owner=설치 Pawn, 피해 차량 throttle 즉시 0·2초 후 회복, 양쪽 폭발/소리/파괴. T-MINE | TODO |
| M13 | 도전: 게임 상태별 운전·공격 제한 | GATE-001~007 | M03, M08, M12 | Waiting/Finished의 throttle/steering 0·handbrake true·Reset/공격/지뢰 거부, Playing 전환 잠금 해제. 클라이언트 입력 및 서버 실행 모두 검사. T-GATE | TODO |
| M14 | 최종 회귀 검증·정리: 필수/도전 조합, 문서·재현 절차 확정 | 전체 65개 + 기반 편차 | M10~13 | 도전 도입 후 필수 재검증, 버프/바퀴/지뢰/상태 전환 조합 검사, 잔여 결함·미검증·편차 명시. 코드와 문서 일치 | TODO |

### 실행 순서와 범위 경계

권장 실행 순서는 M00 → M01 → … → M14이다. 선행 열은 실제 의존성이다. M04 채팅은 M03과 독립적으로 구현할 수 있고, M08 공격은 M06/M07을 기술적으로 요구하지 않는다. 다만 학습과 검증 편의를 위해 획득 시스템을 먼저 완성한다. 병렬 에이전트 작업을 의미하지 않는다.

- M01은 템플릿 도입·자동 접속 검사 및 사용자의 기본 2인 PIE 확인까지 완료했다. 전체 Start 점유 예외와 기반 편차 정리는 남아 IN_PROGRESS로 유지한다. M02는 사용자 검증 완료 보고로 VERIFIED이며, M03 사용자 검증 완료. M04 C++과 WBP 전환 작성. WBP 생성 수정과 채팅·운전 복귀·기존 코인/타이머 표시 사용자 확인 완료.
- M02 검증에 필요한 값 변경은 테스트 전용 서버 경로로 수행한다. 이를 플레이어가 사용할 임의 보상 RPC로 노출하지 않는다. 버프의 실제 차량 적용은 M06 범위다.
- M03은 상태/시간, M09는 승자/결과를 담당한다. M03 완료만으로 경기 종료 기능 전체를 완료 처리하지 않는다.
- M08은 필수 ApplyDamage 버전이며 M11에서 ApplyPointDamage로 확장해 5원 탈취 동작을 회귀 검증한다.
- 도전 기능은 M10 필수 검증이 끝난 후 시작한다. Playing 전용 입력 제한의 완료 책임은 M13이며, 그 전부터 완료된 것처럼 보고하지 않는다.
- M14는 새 기능 추가 단계가 아니다. 발견된 결함 수정과 필요한 최소 정리만 수행한다.

### 각 작업 시작 전에 결정할 사항

| 시점 | 필요한 결정 | 근거 |
|---|---|---|
| M01 종료 전 | Scalable/Starter Content 차이의 적용 또는 편차 유지, Dedicated 실습 범위 | BASE-001/005, decisions.md |
| M02~M06 | M02는 float 값 저장/표시만 구현하고 단위는 표시하지 않음. M06 전에 단위·실제 적용 방식을 결정 | Q-003 |
| M03 | 5초 Waiting/60초 Playing 확정. 대기 중 2명 미만이면 취소·재충족 시 5초 재시작. 경기 중 이탈은 시간 유지, 후입장 스냅샷 수신, 자동 재시작/인원 차단 없음 | Q-002, Q-005 |
| M04 | 사용자 확정: 최대 200 UTF-16 코드 단위, 최소 0.5초 전송 간격 | Q-002 해결(채팅) |
| M08 | 공격 거리·빈도 제한 | Q-002 |
| M11 | 바퀴 Bone 목록, 패널티 크기·회복 주기·기존 양수 버프와 결합 | Q-004 |
| M12 | 지뢰 자기 피해·연속 피격·설치 빈도 정책 | Q-006 |

명세의 5원 탈취, 코인 +10, 속도 +10, 1Hz 아이템, 2초 리스폰/가속 회복은 확정 요구다. 위 미정 정책은 구현 전에 합의하고 decisions.md에 기록하며 임의 수치를 명세로 취급하지 않는다.

### 공통 완료 기준

1. 에이전트는 해당 범위를 구현하고 빌드·검증을 실행하지 않은 상태로 보고한다.
2. 사용자가 수행할 빌드 절차와 Host/Client 확인 순서·예상 결과를 안내한다. 표의 검증 기준은 사용자 수용 기준이며, M10/M14도 사용자가 검증하고 에이전트는 절차 안내·전달된 오류 수정·기록을 담당한다.
3. 요구사항 표의 구현 파일·상태와 testing/progress/decisions를 갱신.
4. 결과·서버/클라이언트 흐름·실패/미검증·다음 작업을 보고하고 다음 지시 대기.

마일스톤도 TODO / IN_PROGRESS / IMPLEMENTED / VERIFIED / BLOCKED로 구별한다. 구현과 안내가 끝나면 에이전트 작업은 완료하며 상태는 IMPLEMENTED(사용자 검증 대기)다. 사용자가 수용 기준 통과를 알려준 범위만 VERIFIED로 기록한다. 미검증 항목을 검증 완료로 표현하지 않는다. 요구사항별 세부 상태는 아래 표가 기준이며, 통합 마일스톤은 선행 기능의 상태를 임의로 변경하지 않는다.

## 체크박스 외 기반 요구

| ID | 원문 요구/전제 | 상태 | 현재 / 예정 | 검증 |
|---|---|---|---|---|
| BASE-001 | UE5.5.4, CX, Vehicle, Desktop, Scalable, Starter Content | IN_PROGRESS | CH4_2/5.6 Vehicle 도입 승인·완료. Scalable/Starter Content 미적용 | 빌드·BP·접속 검사 통과, 편차 decisions.md |
| BASE-002 | PlayerStart 총 3개와 사용 가능한 위치 선택 | IMPLEMENTED | Source/CH4_2/CH4_2GameMode.h/.cpp, Content/VehicleTemplate/Maps/VehicleBasic.umap 및 외부 액터 | T-BASE: 3개 저장·서로 다른 스폰 확인, 전체 점유 분기 미검증 |
| BASE-003 | Listen Server 로컬 PC UI와 VehiclePawn 참조 동기화 | VERIFIED | Source/CH4_2/CH4_2PlayerController.h/.cpp | 기존 참조 복제 로그 및 2026-10-21 사용자 2인 PIE 속도·기어 HUD 갱신 확인 |
| BASE-004 | 2인 경쟁, 입력→판정→표시 설계 | TODO | 문서 설계만 작성 / 전체 통합 | T-BASE 및 T-MATCH |
| BASE-005 | Listen 배경 CPU 설정 및 Dedicated 환경 안내 검토 | TODO | 설정 미변경, Server Target 없음 | T-BASE; Dedicated 실습 범위는 별도 합의 |

## 기능 추적

상태: TODO → IN_PROGRESS → IMPLEMENTED → VERIFIED. BLOCKED는 진행 불가능한 의존성과 해소 조건을 함께 기록한다. 구현만으로 VERIFIED로 올리지 않는다. 여러 파일에 걸친 요구는 모두 완료되어야 IMPLEMENTED다.

| ID | 구분 | 원문 요구사항 | 상태 | 실제 구현 / 예정 파일 | 검증 방법 / 증거 |
|---|---|---|---|---|---|
| CHAT-001 | 필수 | 채팅창 위젯 클래스 생성 | VERIFIED | Source/CH4_2/CH4_2ChatWidget.h/.cpp, CH4_2PlayerController.h/.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| CHAT-002 | 필수 | 채팅창 위젯 블루프린트 클래스 애셋 생성 | VERIFIED | Scripts/create_m04_widgets.py, CH4_2WidgetAuthoringLibrary.h/.cpp; Content/UI/WBP_Chat 및 WBP_PlayerStats (사용자 생성 보고) | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| CHAT-003 | 필수 | 위젯 객체 생성 및 연결 | VERIFIED | Scripts/create_m04_widgets.py, CH4_2WidgetAuthoringLibrary.h/.cpp; Content/UI/WBP_Chat 및 WBP_PlayerStats (사용자 생성 보고) | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| CHAT-004 | 필수 | 채팅창 엔터 이벤트 관련 로직 작성 | VERIFIED | Source/CH4_2/CH4_2ChatWidget.h/.cpp, CH4_2PlayerController.h/.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| CHAT-005 | 필수 | 채팅창에 작성한 메세지를 화면에 띄우기 | VERIFIED | Scripts/create_m04_widgets.py, CH4_2WidgetAuthoringLibrary.h/.cpp; Content/UI/WBP_Chat 및 WBP_PlayerStats (사용자 생성 보고) | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| CHAT-006 | 필수 | 채팅 동기화 | VERIFIED | Source/CH4_2/CH4_2ChatWidget.h/.cpp, CH4_2PlayerController.h/.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| PS-001 | 필수 | 습득한 원화 속성(`Coins`) 추가 및 프로퍼티 레플리케이션 로직 작성 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-002 | 필수 | 원화를 증가시키는 함수와 차감하는 함수 정의 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-003 | 필수 | 원화 속성의 값은 서버에서만 변경되도록 검사 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-004 | 필수 | 원화 속성을 DOREPLIFETIME_CONDITION(COND_OwnerOnly)로 복제되게 작성. | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-005 | 필수 | 추가 최대 속도 속성(`BonusMaxSpeed`) 추가 및 프로퍼티 레플리케이션 로직 작성 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-006 | 필수 | 추가 최대 속도 속성을 증가시키는 함수 정의 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-007 | 필수 | 추가 최대 속도 속성의 값은 서버에서만 변경되도록 검사 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-008 | 필수 | 해당 속성을 DOREPLIFETIME_CONDITION(COND_OwnerOnly)로 복제되게 작성. | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | T-PS / 2026-10-21 사용자 검증 완료 보고, docs/m02-player-state.md |
| PS-009 | 필수 | 습득한 원화와 추가 최대 속도를 표시 해주는 UI 제작 및 적절히 연결 | VERIFIED | Source/CH4_2/CH4_2PlayerState.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2GameMode.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| VEH-001 | 필수 | 공격을 위한 좌클릭 InputAction 애셋 생성 및 차량 폰 연결 | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-002 | 필수 | 좌클릭 시 차량 폰 클래스의 ServerRPC 함수 호출 | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-003 | 필수 | 위 ServerRPC에서 차량 폰 앞 범퍼 중앙에서 라인트레이싱으로 충돌 감지 | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-004 | 필수 | 위 라인트레이싱으로 상대 차량 감지 및 상대 차량 ApplyDamage() 함수 호출 | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-005 | 필수 | 피해 차량 폰에게 원화(`Coins`)가 있다면 가해 차량은 5원 획득, 피해 차량은 5원 회수 | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-006 | 필수 | 피해 차량이 5원 미만 갖고 있으면 아무런 동작도 안함. | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| VEH-007 | 필수 | DrawDebugLine()을 이용하여 라인트레이싱 표시. 닿은 차량에는 파티클 재생. | TODO | 없음 / CHVehiclePawn; Content/Input/IA_Fire | T-ATTACK / 미실행 |
| ITEM-001 | 필수 | 코인 액터 클래스 생성 및 오버랩 시에 대상 플레이어는 원화(`Coins`) 10원 획득 | VERIFIED | Source/CH4_2/CH4_2CoinPickup.h/.cpp, CH4_2Pawn.h/.cpp, CH4_2PlayerController.h/.cpp | T-ITEM / 2026-10-26 사용자 검증 완료 보고, docs/m05-coin-pickup.md; 개별 로그 미제공 |
| ITEM-002 | 필수 | 코인 액터 클래스의 NetUpdateFrequency는 1로 설정. | VERIFIED | Source/CH4_2/CH4_2CoinPickup.h/.cpp, CH4_2Pawn.h/.cpp, CH4_2PlayerController.h/.cpp | T-ITEM / 2026-10-26 사용자 검증 완료 보고, docs/m05-coin-pickup.md; 개별 로그 미제공 |
| ITEM-003 | 필수 | 오버랩된 코인 액터는 파괴. | VERIFIED | Source/CH4_2/CH4_2CoinPickup.h/.cpp, CH4_2Pawn.h/.cpp, CH4_2PlayerController.h/.cpp | T-ITEM / 2026-10-26 사용자 검증 완료 보고, docs/m05-coin-pickup.md; 개별 로그 미제공 |
| ITEM-004 | 필수 | 코인 아이템 액터 파괴 시 파티클 및 사운드 재생. | VERIFIED | Source/CH4_2/CH4_2CoinPickup.h/.cpp, CH4_2Pawn.h/.cpp, CH4_2PlayerController.h/.cpp | T-ITEM / 2026-10-26 사용자 검증 완료 보고, docs/m05-coin-pickup.md; 개별 로그 미제공 |
| ITEM-005 | 필수 | 스피드 버프 액터 클래스 생성 및 오버랩 시에 대상 플레이어는 추가 최대 속도 10 상승 | TODO | 없음 / CHPickup, CHCoinPickup, CHSpeedPickup | T-ITEM / 미실행 |
| ITEM-006 | 필수 | 스피드 버프 액터 클래스의 NetUpdateFrequency는 1로 설정. | TODO | 없음 / CHPickup, CHCoinPickup, CHSpeedPickup | T-ITEM / 미실행 |
| ITEM-007 | 필수 | 오버랩된 버프 아이템 액터는 파괴. | TODO | 없음 / CHPickup, CHCoinPickup, CHSpeedPickup | T-ITEM / 미실행 |
| ITEM-008 | 필수 | 버프 아이템 액터 파괴 시 파티클 및 사운드 재생. | TODO | 없음 / CHPickup, CHCoinPickup, CHSpeedPickup | T-ITEM / 미실행 |
| SPAWN-001 | 필수 | 스포너 액터 클래스 생성 및 액터 스폰 로직 작성. | TODO | 없음 / CHItemSpawner; Content/Maps | T-SPAWN / 미실행 |
| SPAWN-002 | 필수 | 스포너는 생성한 아이템 객체를 속성으로 저장. OnDestroyed 델리게이트 구독. | TODO | 없음 / CHItemSpawner; Content/Maps | T-SPAWN / 미실행 |
| SPAWN-003 | 필수 | 스포너가 생성한 아이템 객체가 파괴되면 2초 뒤 같은 종류의 아이템을 리스폰. | TODO | 없음 / CHItemSpawner; Content/Maps | T-SPAWN / 미실행 |
| SPAWN-004 | 필수 | 코인을 스폰하는 스포너를 레벨에 적절히 배치. 스포너 별로 코인과 버프 액터 설정. | TODO | 없음 / CHItemSpawner; Content/Maps | T-SPAWN / 미실행 |
| GM-001 | 필수 | GameState 클래스 생성 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-002 | 필수 | Waiting, Playing, Finished를 표현하는 게임 상태 Enum 정의 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-003 | 필수 | GameState 클래스에 현재 게임 상태를 표현하는 속성과 남은 시간 속성 정의 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-004 | 필수 | GameState 클래스에 우승자 목록 속성을 정의 | TODO | 없음 / CHGameMode, CHGameState, CHHUDWidget | T-MATCH / 미실행 |
| GM-005 | 필수 | 게임 시작 시 게임 상태 속성이 Waiting으로 설정되게끔 로직 작성 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-006 | 필수 | 첫 플레이어가 접속 시 대기 카운트다운 시작하게끔 로직 작성 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-007 | 필수 | 서버 타이머가 1초마다 남은 시간 속성을 감소시키고 이를 UI에서 확인할 수 있게 작성. | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| GM-008 | 필수 | 대기 카운트다운이 끝나면 게임 상태 속성이 Playing으로 설정되게끔 로직 작성 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-009 | 필수 | 서버 타이머가 1초마다 남은 시간 속성을 감소시키고 이를 UI에서 확인할 수 있게 작성. | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | 2026-10-21 사용자 채팅·운전 복귀·기존 코인/타이머 표시 확인; 세부 범위 docs/testing.md |
| GM-010 | 필수 | 플레이 카운트 다운이 끝나면 게임 상태 속성이 Finished로 설정되게끔 로직 작성 | VERIFIED | Source/CH4_2/CH4_2GameState.h/.cpp, CH4_2GameMode.h/.cpp, CH4_2PlayerController.h/.cpp, CH4_2PlayerStatsWidget.h/.cpp | T-MATCH 상태/시간 / 2026-10-21 사용자 검증 완료 보고. GM-006은 2명 접속 조건으로 변경, docs/m03-match-flow.md |
| GM-011 | 필수 | 모든 플레이어의 원화를 비교해 가장 많은 원화를 가진 플레이어 탐색 | TODO | 없음 / CHGameMode, CHGameState, CHHUDWidget | T-MATCH / 미실행 |
| GM-012 | 필수 | 최고 원화가 같으면, 우승자 목록 속성에 모두 넣어서 공동 우승자로 처리 | TODO | 없음 / CHGameMode, CHGameState, CHHUDWidget | T-MATCH / 미실행 |
| GM-013 | 필수 | 로컬 플레이어의 승리 또는 패배 결과가 UI로 나오게끔 구현 | TODO | 없음 / CHGameMode, CHGameState, CHHUDWidget | T-MATCH / 미실행 |
| WHEEL-001 | 도전 | 좌클릭 시 ServerRPC 함수에서 ApplyDamage() 대신 ApplyPointDamage() 호출 | TODO | 없음 / CHVehiclePawn, CHPlayerState | T-WHEEL / 미실행 |
| WHEEL-002 | 도전 | 맞은 Bone이 바퀴인지 판별 | TODO | 없음 / CHVehiclePawn, CHPlayerState | T-WHEEL / 미실행 |
| WHEEL-003 | 도전 | 바퀴를 맞히면 피해 차량의 추가 최대 속도(`BonusMaxSpeed`)에 음수 패널티 적용 | TODO | 없음 / CHVehiclePawn, CHPlayerState | T-WHEEL / 미실행 |
| WHEEL-004 | 도전 | 음수 패널티는 시간에 따라 0까지 회복. 회복값이 0을 넘어 양수가 되지 않게 제한. | TODO | 없음 / CHVehiclePawn, CHPlayerState | T-WHEEL / 미실행 |
| MINE-001 | 도전 | 지뢰 매설을 위한 F키 InputAction 애셋 생성 및 차량 폰 연결 | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-002 | 도전 | F키 입력 시 차량 폰 클래스의 ServerRPC 함수 호출 | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-003 | 도전 | 위 ServerRPC에서 차량 뒤쪽에 지뢰를 스폰 | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-004 | 도전 | 생성한 지뢰의 Owner 속성을 설치한 차량으로 지정 | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-005 | 도전 | 지뢰와 오버랩된 차량은 전진 가속 입력이 즉시 0으로 설정되게 구현(가속 페달 고장) | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-006 | 도전 | 2초 뒤 가속 페달 고장 상태 회복(해제) | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| MINE-007 | 도전 | 지뢰와 오버랩되면 폭발 파티클과 사운드 재생 및 지뢰 액터 파괴 | TODO | 없음 / CHMine, CHVehiclePawn; Content/Input/IA_Mine | T-MINE / 미실행 |
| GATE-001 | 도전 | `Playing` 상태에서만 가속과 조향 입력을 허용한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-002 | 도전 | `Waiting`과 `Finished`에서는 스로틀과 조향 입력을 0으로 유지한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-003 | 도전 | `Waiting`과 `Finished`에서는 핸드브레이크를 true로 유지한다 | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-004 | 도전 | `Waiting`과 `Finished`에서는 차량 위치 초기화 기능도 사용할 수 없게 한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-005 | 도전 | `Playing`으로 전환되면 게임 상태용 핸드브레이크 잠금을 자동으로 해제한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-006 | 도전 | `Playing` 상태에서만 좌클릭 공격 입력을 허용한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |
| GATE-007 | 도전 | `Playing` 상태에서만 F키 지뢰 설치 입력을 허용한다. | TODO | 없음 / CHVehiclePawn, CHGameState | T-GATE / 미실행 |







M04 UI 전환 이력: WBP 전환 시 표시 항목을 재검증 대기로 변경했으며, 이후 사용자 기존 코인/타이머 표시 확인 보고로 PS-009/GM-007/GM-009와 M02/M03을 VERIFIED로 복원했다. 서버 데이터·타이머 항목은 유지한다. 현재 UI 생성/표시의 실제 파일 및 최신 검증 기준은 docs/m04-chat-wbp.md를 따른다.





