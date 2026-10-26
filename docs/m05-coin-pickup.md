# M05 코인 획득

## 구현 전 계획 — 2026-10-23
- 목표: 서버 차량 오버랩으로 +10원, 중복 지급 방지, NetUpdateFrequency=1, 획득 시 파괴와 파티클/사운드 전달.
- 신규 CH4_2CoinPickup.h/.cpp. PlayerController에 일시적 획득 효과 Client RPC 추가. Pawn은 서버 BeginPlay에서 메시 오버랩 활성화.
- 흐름: 서버 Sphere 오버랩 → 소유 PC/PS/빙의 관계 검사 → 소비 가드 → AddCoins(10) → 각 PC 효과 RPC → Destroy.
- 코인 트리거 반지름 100cm는 조정 가능한 배치 기본값이며 과제 규칙이 아니다. 지급액 10과 1Hz는 명세 고정값.
- 위험: 차량 BP 충돌 설정, 맵 배치/효과 에셋 미지정, 최초 겹침, 복수 메시/차량 동시 겹침, 파괴 액터 RPC 누락.
- 효과 전달은 지속 PC 채널을 사용하며 보상/파괴의 상태 복제와 분리한다. EndPlay에서는 효과를 재생하지 않는다.
- 완료 기준: 사용자 빌드, 에디터 코인 배치·효과 연결, Host/Client 각각 +10·중복 없음·양쪽 파괴/효과 확인. 에이전트는 빌드/검증을 실행하지 않는다.
- M06 버프/M07 스포너/경기 상태별 획득 제한은 이번 구현에 포함하지 않는다. 현재 단계는 모든 경기 상태에서 획득 가능.

## 구현 결과
- Source/CH4_2/CH4_2CoinPickup.h/.cpp: 서버 전용 Sphere 획득, 고정 +10, 1Hz 복제, 소비 가드, 즉시 서버 Destroy.
- PlayerController.h/.cpp: 지속 PC 채널의 ClientPlayPickupEffects. 현재 연결된 모든 플레이어에게 위치/파티클/사운드 전달. 호스트도 같은 경로로 한 번 재생한다.
- Pawn.h/.cpp: 서버 BeginPlay에서 차량 메시 GenerateOverlapEvents 활성화. PC OnPossess에서 이미 겹친 코인 재처리.
- RPC는 파괴되는 코인 참조를 전송하지 않는다. 양 채널의 도착 순서는 보장하지 않으므로 효과와 원격 코인 사라짐 사이에 짧은 차이가 날 수 있다.
- 지속 상태는 코인의 서버 파괴와 PlayerState OwnerOnly 복제다. 뒤늦게 접속한 사람에게 지난 효과를 재생하지 않는다.
- 맵 종료/스트리밍 해제/일반 외부 Destroy는 보상과 획득 효과를 발생시키지 않는다.
- 효과는 독립된 월드 파티클/사운드로 자동 정리된다. 루프 파티클·루프 사운드는 사용하지 않는다.
- 현재 C++ 구현은 완료했지만 코인 BP 배치 및 효과/사운드 에셋 연결은 사용자 에디터 작업으로 남아 있다. ITEM-001~003 IMPLEMENTED, ITEM-004 및 M05 IN_PROGRESS. 빌드/UHT/PIE/작업 후 검사 미실행.

## 사용자 빌드
에디터를 닫고 Development Editor / Win64로 전체 빌드한다. PowerShell 명령:

    & 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' CH4_2Editor Win64 Development '-Project=C:\UnrealProject\CH4_2\CH4_2.uproject' -WaitMutex -NoHotReloadFromIDE

새 클래스 및 RPC가 있으므로 Live Coding만으로 반영하지 않는다.

## 에디터 준비
1. C++ Classes → CH4_2 → CH4_2CoinPickup을 부모로 Blueprint를 생성하여 /Game/Items/BP_CoinPickup으로 저장한다.
2. 기본 Cylinder 메시가 제공된다. CoinMesh의 모양/머티리얼은 필요에 따라 변경한다. 메시 Collision은 NoCollision으로 유지한다.
3. PickupSphere: Sphere Radius 100, Query Only, WorldDynamic, Vehicle/Pawn Overlap, Generate Overlap Events 켜짐. 차량 메시 Collision Object Type은 Vehicle, WorldDynamic 응답은 Ignore가 아니어야 한다. 기본 Vehicle 프로필은 유지한다.
4. Class Defaults에서 Replicates 켜짐, Net Update Frequency/Min Net Update Frequency 1을 확인한다.
5. Class Defaults → Coin → Effects의 Pickup Effect(Particle System)와 Pickup Sound(Sound Wave 또는 Sound Cue)에 짧은 비반복 효과를 지정한다. Niagara System은 이 필드의 타입이 아니다.
6. 프로젝트에 효과가 없으므로 직접 보유한 에셋을 가져오거나 Content Browser의 Add → Add Feature or Content Pack → Content → Starter Content를 사용자 에디터에서 추가할 수 있다. 설치본 Samples/StarterContent에 P_Sparks와 Explosion_Cue가 있지만, 루프 여부를 확인하고 짧은 일회성 Particle System(예: P_Explosion) 및 사운드를 선택한다. 엔진 폴더 파일의 탐색만으로 프로젝트 연결이 완료되지는 않는다. Starter Content 추가는 시각/청각 테스트용 선택이며 기존 기반 설정 편차를 자동 해소한 것으로 기록하지 않는다.
7. BP Compile/Save. VehicleBasic에서 두 출발 차량이 접근할 수 있는 평지에 BP_CoinPickup을 2~3개 배치하고 저장한다. 트리거 중심은 차량 차체 높이 부근에 두고 지형에 묻히지 않게 한다. 스포너는 아직 없다.
8. 파티클/사운드 미지정이면 PIE 로그에 Coin ... assign PickupEffect and PickupSound 경고가 나온다. 코인 보상은 동작하지만 ITEM-004 완료 조건을 충족하지 않는다.

## 사용자 검증 순서와 예상 결과
1. VehicleBasic, Players=2, Play As Listen Server, New Editor Window (PIE). 오디오 비교는 별도 창/프로세스 및 각 플레이어 오디오 장치 설정을 사용한다. 비활성 창 오디오 음소거와 실제 누락을 구별한다.
2. Host가 코인 A를 획득: Host Coins만 정확히 +10, Client Coins는 유지. 두 창에서 A가 사라지고 획득 위치에서 효과/사운드가 한 번씩 발생.
3. Client가 코인 B를 획득: Client만 +10, Host는 유지. 두 창에서 파괴·효과 확인.
4. 같은 코인에 두 차량이 동시에 접근: 전체 지급 합계는 +10, 서버에서 먼저 처리한 유효 차량 한 명만 획득. 메시 여러 바디가 겹쳐도 추가 지급 없음.
5. 사라진 위치를 왕복해도 추가 보상 없음. 현재 재생성은 없으며 다시 PIE를 시작하면 배치 코인이 복원됨.
6. 차량 스폰 위치에 코인 하나를 겹쳐 배치하여 새 PIE 실행: 빙의 후 유효 플레이어에게 한 번만 지급. 시험 후 일반 배치로 복구한다.
7. 코인 근처에서 PIE 종료/재시작 또는 차량 이탈: 종료 자체로 획득 효과가 발생하거나 오류가 없어야 한다. 소비 전 코인은 남아 있어야 한다.
8. 채팅/운전 복귀, 소유자 코인 HUD, 기존 5초/60초 타이머가 유지되는지 확인한다. 이번 단계는 Waiting/Finished에서도 코인 획득 가능하다.
9. 에셋 연결 후에도 효과가 없다면 Particle System의 Loop Count/Duration과 Sound의 Looping/Attenuation 설정을 확인한다.

## 문제 발생 시 전달할 내용
- 빌드 실패: 첫 UHT/C++ 오류와 앞뒤 로그.
- 획득 실패: BP_CoinPickup의 Sphere 충돌 설정, 차량 메시 충돌 설정, 코인/차량 위치, Host 또는 Client 여부.
- 중복/표시 문제: 획득 전후 각 창 Coins, 서버 LogCH4_2의 Coin collected 행(코인 actor/player/reward), 관련 경고와 Saved/Logs/CH4_2.log.
- 효과 문제: 연결한 파티클/사운드 경로와 어느 창에서 재생되지 않았는지.
- 개별 항목을 직접 확인한 뒤 결과를 전달하면 해당 범위만 VERIFIED로 변경한다.

## 다음 단계
M05 사용자 설정·검증 후 다음 지시에 따라 M06 속도 버프를 진행한다. 실제 속도 단위/적용 방식은 M06 전에 결정한다.

## 사용자 수용 결과 — 2026-10-26
- 위 에디터 설정 및 검증 안내 이후 사용자가 “검증 완료”를 보고했다.
- 이를 M05 코인 획득 수용 완료로 기록하고 M05/ITEM-001~004를 VERIFIED로 갱신한다. 위 구현 직후 대기 상태는 과거 이력이다.
- 증거는 사용자 보고이며 개별 로그·스크린샷은 제공받지 않았다. 에이전트 재빌드/재검증은 실행하지 않았다.
- 후속 버프·스포너 등은 검증 범위에 포함하지 않는다.
