# M08 차량 공격·5원 탈취

## 구현 전 계획 — 2026-10-27
- 사용자 확정: 앞 범퍼에서 전방 500cm, 쿨타임 2초. 좌클릭 한 번에 한 번 공격.
- IA_Fire(Boolean)를 기존 차량 IMC에 Left Mouse Button으로 연결하고 Pawn의 Started 이벤트에서 소유 Pawn Server RPC 호출.
- 서버는 소유/빙의/PS 관계와 PC 수명의 쿨타임을 확인한다. 클라이언트로부터 대상/피해량/좌표를 받지 않는다.
- BP에서 위치 조정 가능한 AttackOrigin을 앞 범퍼 중앙에 배치. 서버 Visibility 라인트레이스로 최초 차단 대상을 판정하며 장애물 뒤 차량은 공격하지 않는다.
- ApplyDamage → 피해 Pawn TakeDamage에서 이번 서버 명중의 일회성 문맥 확인 → PS 간 5원 이동. 두 잔액을 먼저 변경하고 알림을 보내 합계 보존.
- 상대 0~4원, 자기 자신, 유효하지 않은 소유 관계, 수신자 정수 오버플로는 무변화.
- 기존 PC 채널에 공격 시각 효과 전달: DrawDebugLine과 차량 명중 파티클. 돈은 OwnerOnly 복제 유지.
- 수정: Pawn.h/.cpp, PlayerState.h/.cpp, PlayerController.h/.cpp. 사용자 에디터 작업: IA_Fire/IMC/차량 BP의 AttackAction·AttackOrigin·AttackHitEffect 설정.
- 위험: 범퍼 위치/Visibility 응답 설정, 입력 중복 바인딩, 피해 이벤트 재진입, 연결 종료/재빙의, 쿨타임 우회. 새 Tick/타이머 없음.
- 완료 기준: Host/Client 양방향, 0/4/5/10원 분기, 합계 보존, 장애물/범위/쿨타임/채팅 포커스, 양쪽 선·파티클.
- M09 승패, M11 부위 공격, M13 Playing 전용 제한은 범위 밖. 현재 모든 경기 단계에서 공격 가능.
- 에이전트 빌드/UHT/테스트/PIE/작업 후 검사 미실행. 에셋 생성/연결 및 검증은 사용자 수행.

## 구현 결과
- AttackAction Started → 로컬 제어/채팅/2초 간격 확인 → 인자 없는 ServerAttack.
- 서버 PC 수명에 2초 쿨타임 저장. 빗나감/잔액 부족/장애물도 승인된 공격 한 번으로 소비한다. 클릭을 누른 채 유지해도 자동 연사는 없다.
- 서버 소유/빙의/PS 검사 후 AttackOrigin부터 차량 전방(+X) 500cm Visibility 단일 Trace. 최초 차단 대상만 처리.
- 차량 원점→AttackOrigin 구간이 막혀 있으면 공격을 막아 범퍼가 얇은 벽 너머에 있을 때의 관통을 방지한다. 이 경우 디버그 선은 원점에서 장애물까지 표시한다.
- 기본 AttackOrigin 상대 위치 (200,0,50)cm는 편집 시작점이다. 정확한 범퍼 중앙은 실제 차량 메시에서 사용자가 맞춰야 한다. Arrow 회전은 공격 방향을 바꾸지 않으며 서버 차량 전방을 사용한다.
- 서버의 일회성 명중 문맥이 있는 ApplyDamage만 5원 이동을 수행. 임의 ApplyDamage나 같은 문맥의 재진입으로 중복 지급할 수 없다.
- TryTransferCoinsTo는 잔액/오버플로/동일 PS/권한 검사 후 두 값을 함께 변경하고 알림을 보낸다. 서버 합계는 보존되지만 서로 다른 클라이언트 화면 도착 시각까지 원자적으로 동기화되는 것은 아니다.
- 성공 시 표준 TakeDamage 알림도 전달한다. Blueprint AnyDamage에 별도 코인 변경 로직을 추가하지 않는다.
- 각 PC에 서버 확정 선/명중 위치를 보내 로컬에서 표시한다. 초록 선은 유효 차량 명중, 빨강 선은 빗나감/장애물. 선은 1초 표시(게임 규칙이 아닌 표시 기본값).
- 명중 파티클은 잔액과 무관한 접촉 표시다. 상대가 5원 미만이면 양쪽 돈과 피해 이벤트는 무변화지만 선/명중 표시는 유지한다.
- 기존 Coins OwnerOnly와 UI 공통 이벤트 유지. 사운드/새 HUD/공격 애니메이션은 추가하지 않는다.
- 새 Tick/타이머/지속 델리게이트 없음. 시각 효과는 월드 자동 정리, 명중 문맥은 호출 안에서 소비/해제한다.
- 현재 C++ 구현 완료. VEH-002~006 IMPLEMENTED, VEH-001/007 및 M08 IN_PROGRESS(입력 에셋/차량 BP/파티클 연결 대기).

## 사용자 빌드
에디터 종료 후 Development Editor / Win64 전체 빌드한다. RPC와 컴포넌트가 추가되어 Live Coding만 사용하지 않는다.

    & 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' CH4_2Editor Win64 Development '-Project=C:\UnrealProject\CH4_2\CH4_2.uproject' -WaitMutex -NoHotReloadFromIDE

## 사용자 에디터 설정
1. Content Browser의 /Game/Input에서 Input → Input Action 생성, 이름 IA_Fire. Value Type은 Digital (bool), Triggers/Modifiers는 비워 둔다.
2. /Game/VehicleTemplate/Input/IMC_Vehicle_Default를 열고 Mapping 하나 추가: IA_Fire → Left Mouse Button. 같은 매핑을 중복 추가하지 않는다. PC에서 이 IMC를 교체했다면 실제 활성 IMC에 추가한다.
3. 실제 사용하는 차량 BP(기본 /Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn)를 열고 Class Defaults → Input → Attack Action에 IA_Fire 지정한다. 다른 차량 BP도 플레이한다면 동일 연결.
4. Components에서 상속된 AttackOrigin 선택. Viewport에서 앞 범퍼 중앙 높이에 위치시키고, 차체 바로 앞·장애물 안으로 과하게 돌출하지 않도록 조정한다. Scale (1,1,1), 차량 전방은 +X다.
5. Class Defaults → Vehicle → Attack → Attack Hit Effect에 짧은 비반복 Particle System 지정. 코인/버프 효과를 재사용해도 된다. Niagara System은 지원 타입이 아니다.
6. 차량 메시의 Collision Enabled는 Query and Physics, Trace Responses → Visibility는 Block. 대상 차량도 동일해야 한다. 공격을 막아야 하는 벽/장애물 역시 Visibility Block.
7. Can Be Damaged 켜짐 확인. BP Compile/Save, IA/IMC Save. Event Graph에 별도의 공격/피해 코인 로직을 추가하지 않는다.
8. AttackAction/AttackHitEffect 미지정이면 LogCH4_2 경고가 나온다. C++ 코드 작성만으로 입력 에셋 생성과 BP 연결을 완료한 것은 아니다.

## 사용자 검증 순서
기본 환경: VehicleBasic, 2 Players, Play As Listen Server, New Editor Window (PIE).
코인 스포너와 떨어진 평지에서 정차한 두 차량을 일렬로 두고, 공격자 범퍼로부터 상대 차체까지 500cm 이내로 맞춘다.

1. 호스트 콘솔 CHStatsList로 두 PlayerId 확인. 아래 H/C는 실제 ID로 치환하며 모든 값 변경 명령은 호스트 콘솔에서 실행한다.
2. 피해자 0원: 공격 → 돈 변화 없음, 양쪽에 선/명중 파티클. 각 공격 사이 2초 이상 기다린다.
3. CHStatsAddCoins C 4 → Host가 Client 공격 → 4원 유지, Host 증가 없음.
4. CHStatsAddCoins C 1 → Client 5원. Host 공격 → Client 0, Host +5. 두 HUD 확인.
5. CHStatsAddCoins C 10 → Host 공격 두 번(2초 간격) → Client 10→5→0, Host 매번 +5. 서버 CHStatsList로 합계 보존 확인.
6. 공격·피해 역할을 바꾸어 Client가 Host 공격: 서버와 소유 HUD에 같은 -5/+5 결과. 다른 플레이어 Coins가 UI에 공개되지는 않는다.
7. 2초 안에 여러 번 클릭: 첫 승인 공격만 처리. 계속 누르기: 한 번만 공격. 2초 이후 떼고 다시 누르면 재공격. 전송 지연 때문에 경계에서는 약간 더 기다린다.
8. 500cm 밖, 옆/뒤로 빗나감, 두 차량 사이 Visibility Block 벽: 돈 변화 없음. 선은 첫 장애물에서 끝난다. 범퍼를 벽에 붙여도 벽 뒤 차량으로 피해가 전달되지 않아야 한다.
9. 자기 차체는 Trace에서 제외된다. 상대가 나가거나 빙의되지 않은 차량이면 돈 이동 없음. 차체 중심→범퍼 구간에 다른 물체가 끼면 공격은 차단된다.
10. 채팅 중 좌클릭은 공격하지 않고, Enter/Escape 후 운전과 공격이 복귀한다.
11. 오버플로: 새 PIE/잔액 0에서 호스트 CHStatsAddCoins H 2147483647, CHStatsAddCoins C 5. Host가 Client 공격해도 두 값 유지. 테스트 종료 후 PIE 재시작.
12. 차량이 교체되어도 같은 PC의 2초 서버 쿨타임은 유지된다. 새 접속은 새 PC 수명이다.
13. 기존 코인/버프·60→70→80·2초 스포너·5초/60초 타이머 회귀 확인.
14. 현재 Waiting/Finished에서도 공격 가능하다. Playing 전용 제한은 M13에서 구현한다. DebugLine은 PIE/Development용이며 Shipping 표시를 보장하지 않는다.

## 오류 전달 및 현재 상태
- 빌드/UHT/PIE/입력 에셋 생성/작업 후 검사 에이전트 미실행.
- 입력 실패: IA_Fire/IMC 매핑, 차량 Attack Action, 어느 창인지, 관련 경고.
- 명중 실패: 범퍼 위치, 두 차량의 위치/방향, Visibility 설정, 서버 Vehicle attack 로그의 target/originBlocked.
- 돈 이동 문제: Host/Client 공격 전후 잔액, CHStatsList, Vehicle attack transferred 값, Saved/Logs/CH4_2.log.
- 효과 문제: Attack Hit Effect 에셋 경로와 어느 창인지. 파티클이 영구 잔존하면 루프 여부 확인.
- 다음 지시 후 M09 승패/공동 우승 결과 UI.
