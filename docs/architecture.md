# 구조 분석 및 초기 아키텍처

기준일: 2026-10-21. 아래의 현재 상태와 제안 구조를 구별한다.

## 현재 상태: M02 개인 데이터와 HUD
- 실제 신규 클래스: ACH4_2PlayerState(APlayerState), UCH4_2PlayerStatsWidget(UUserWidget). 초기 Coins/BonusMaxSpeed는 0.
- GameMode 생성자에서 PlayerStateClass를 연결한다. Blueprint override는 사용자 확인 필요.
- PC는 로컬에서만 개인 패널을 생성하며 BeginPlay/InitPlayerState/OnRep_PlayerState에서 본인 PS 이벤트를 연결하고 현재 값을 읽는다.
- Coins/BonusMaxSpeed는 OwnerOnly, 서버 setter와 OnRep가 같은 알림을 사용한다. 수치용 Tick은 없다.
- 개인 패널은 네이티브 위젯으로 생성되어 별도 WBP가 필요 없다. 기존 차량 속도·기어 HUD는 그대로 유지한다.
- M02는 사용자 검증 완료 보고로 VERIFIED. 아래 제안의 CHPlayerState/개인 HUD 역할은 이 실제 클래스에 대응하며, 미구현 클래스만 향후 제안이다.

## 단계 2a Vehicle 도입 상태
- 설치 TP_VehicleAdv를 CH4_2로 치환해 도입. ACH4_2Pawn/PlayerController/GameMode, UCH4_2UI, SportsCar/OffroadCar 및 바퀴 클래스가 실제 존재한다.
- ChaosVehicles/PhysicsCore/UMG/Slate 의존성과 ChaosVehiclesPlugin/RawInput 연결. 기존 Game/Editor Target과 프로젝트 ID는 유지했다.
- Content에 템플릿 및 Vehicles/Input 공유 에셋을 복사. World Partition 외부 액터/객체도 포함한다. OffRoad/TimeTrial variants는 과제 기능이 아닌 공식 C++ 템플릿 콘텐츠다.
- 기본 맵은 /Game/VehicleTemplate/Maps/VehicleBasic, GameMode는 BP_VehicleAdvGameMode. PlayerStart 3개를 엔진에서 저장했다.
- ACH4_2PlayerController의 VehiclePawn은 OwnerOnly 복제, HUD는 로컬만 생성. ACH4_2GameMode는 차량 겹침을 검사해 Start를 선택한다.
- Blueprint의 기존 /Script/TP_VehicleAdv 참조는 CoreRedirects로 해석한다. 바이너리 문자열을 직접 치환하지 않는다.
- 빌드와 별도 Listen Host/Client 접속 검사는 성공. 키보드 운전/시각적 UI/물리 품질은 미검증이다.
- 아래 제안의 ACHVehiclePawn/ACHPlayerController/ACHGameMode/UCHHUDWidget 역할은 실제 ACH4_2Pawn/ACH4_2PlayerController/ACH4_2GameMode/UCH4_2UI 확장으로 적용한다. 이름만 맞추기 위한 재설계는 하지 않는다.

## 단계 1 조사 기록 (Vehicle 도입 전)
| 경로 | 확인 결과 |
|---|---|
| CH4_2.uproject | EngineAssociation 5.6, CH4_2 Runtime/Default 모듈 1개 |
| Source/CH4_2/CH4_2.cpp, .h | FDefaultGameModuleImpl 등록과 CoreMinimal include만 존재. 게임 UCLASS 없음 |
| Source/CH4_2/CH4_2.Build.cs | Core, CoreUObject, Engine, InputCore, EnhancedInput. UMG/차량 의존성 없음 |
| Source/CH4_2.Target.cs, CH4_2Editor.Target.cs | Game/Editor, BuildSettings V5, Unreal5_6 include 순서. Server Target 없음 |
| Config/DefaultEngine.ini | 기본 맵 /Engine/Maps/Templates/OpenWorld, TP_Blank 리다이렉트. Desktop/Maximum 설정 |
| Config/DefaultInput.ini | EnhancedInput 기본 클래스 지정. 게임용 IA/IMC는 없음 |
| Content | Collections/Developers 폴더만 존재. uasset/umap 없음 |
| README.md | 프로젝트 제목만 존재 |
| Binaries, Intermediate, Saved, DerivedDataCache, .vs | 로컬 생성 폴더. 존재 자체를 빌드 성공 증거로 사용하지 않음 |

uproject 명시 플러그인은 Editor용 ModelingToolsEditorMode뿐이다. 설치 엔진 Build.version은 5.6.1/CL 44394996이다. EngineAssociation 자체는 패치 버전을 고정하지 않는다.

## 단계 1에서 발견한 과제와의 차이
과제는 5.5.4/CX/Vehicle/Desktop/Scalable/Starter Content를 안내하지만 실제 프로젝트는 5.6/CH4_2/Blank 계열/Maximum이다. 차량 폰·메시·바퀴·물리 에셋·맵·PlayerStart·UI·아이템·게임 규칙이 모두 준비되지 않았다. 과제 예시 ACXPawn/ACXPlayerController는 현존 코드가 아니다. 기존 assignment와 AGENTS도 없었으며 assignment는 제공된 첨부 원문으로 신규 보존한다.

## 제안 클래스 (전부 미구현, 이름은 임시)
| 클래스 | 책임 | 주요 의존성 |
|---|---|---|
| ACHGameMode : AGameModeBase | 접속, 서버 타이머, 단계 전환, 승자 산정, 안전한 스폰 선택 | GameState, PlayerState |
| ACHGameState : AGameStateBase | Waiting/Playing/Finished, 남은 정수 초, 승자 목록 복제 | PlayerState 식별 |
| ACHPlayerState : APlayerState | Coins/BonusMaxSpeed, 서버 변경 API, 변경 알림 | UI와 직접 의존하지 않음 |
| ACHPlayerController : APlayerController | 로컬 HUD, 채팅 요청/수신, Pawn 교체 연결 | HUD, PlayerState, Pawn |
| ACHVehiclePawn | 차량 입력·이동, 서버 공격, 피해 처리, 도전 기능 | PlayerState, GameState, 차량 컴포넌트 |
| UCHHUDWidget / UCHChatWidget | 상태 표시 / Enter 제출 및 메시지 표시 | 로컬 PC가 바인딩 |
| ACHPickup / ACHCoinPickup / ACHSpeedPickup | 공통 획득 수명과 각 보상 | 서버 PlayerState |
| ACHItemSpawner | 아이템 참조·OnDestroyed 구독·2초 재생성 | Pickup 클래스 |
| ACHMine (도전) | 소유 차량 참조, 오버랩과 가속 장애 | VehiclePawn |

차량 기반은 AWheeledVehiclePawn을 상속한 ACH4_2Pawn으로 확정했다. 단일 모듈을 유지하고 별도 서비스 계층은 만들지 않는다. 제안 파일은 Source/CH4_2 아래 동명 .h/.cpp, 에셋은 Content/Input, UI, Vehicles, Items, Maps에 둔다.

## 데이터와 수명
- GameMode가 PlayerState 원본 Coins를 비교하고 GameState에 승자를 저장한다. 상대 Coins를 UI에 공개할 필요가 없다.
- 승자 목록은 복제 PlayerState 참조를 우선 검토한다. 종료 후 이탈 시 결과 보존을 위해 ID/이름 스냅샷을 사용할지는 decisions의 미결정 정책으로 남긴다.
- HUD는 로컬 PC 수명에 연결하며 초기 값 읽기와 RepNotify 후 갱신을 모두 지원한다. PlayerState/Pawn이 늦게 도착하거나 교체되어도 재바인딩하고 중복 구독을 해제한다.
- 아이템 보상은 서버의 한 번만 소비되는 플래그로 보호한다. 스포너 종료 시 타이머와 델리게이트를 해제하여 맵 종료를 리스폰으로 오인하지 않는다.
- BonusMaxSpeed의 단위·실제 차량 속도 제한 방식은 미정이다. 표시만 올려 실제 효과가 없는 구현을 완료로 보지 않는다.




## M03 실제 구현 (사용자 검증 완료 보고)
- ACH4_2GameMode는 서버 PostLogin/Logout으로 비관전자 참가자 목록을 관리한다.
- ACH4_2GameState는 FCH4_2MatchSnapshot(Phase, RemainingSeconds, ConnectedPlayers, bCountdownActive)을 모든 연결에 복제한다.
- UCH4_2PlayerStatsWidget은 개인 정보와 별도로 공용 경기 행을 표시한다. PC가 GameState 연결과 초기 스냅샷을 담당한다.
- 기본값 Waiting/0초/비활성 → 2명 접속 Waiting/5초 → Playing/60초 → Finished/0초. 대기 중 인원 미달이면 초기 Waiting으로 복귀.
- 기존 초기 설계의 첫 접속 시작 조건은 사용자 승인으로 2명 접속으로 대체되었다. 승자 목록은 아직 미구현.

