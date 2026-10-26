# M04 채팅 및 WBP 전환

## 승인된 범위와 계획
- 사용자 요청에 따라 UI 배치는 WBP Designer로 관리한다. 기존 네이티브 개인/경기 패널을 WBP_PlayerStats로 전환하고 WBP_Chat을 추가한다. 기존 속도/기어 WBP는 유지한다.
- 사용자 확정: 최대 200자(FString UTF-16 코드 단위), 서버 전송 간격 최소 0.5초. 화면 기록은 최근 50건만 보관하는 UI 메모리 제한을 둔다.
- 신규 CH4_2ChatWidget.h/.cpp, 변경 PlayerStatsWidget/PlayerController, Scripts/create_m04_widgets.py 신규. Runtime 위젯 트리 생성은 제거한다.
- Enter로 입력 활성화, Enter 전송 후 게임 포커스 복귀, Escape 또는 다른 곳으로 포커스를 옮기면 전송 없이 취소. 입력 중 유지되던 차량 가속/조향/브레이크 입력을 0으로 돌리고 UIOnly 입력 모드를 사용한다. 차량의 물리적 관성은 유지된다.
- 로컬 Widget → 소유 PC ServerSubmitChat → 서버의 길이/공백/빈도/PlayerState 소유 관계 검사 → 각 PC의 ClientReceiveChat → WBP 표시. 사용자 이름은 서버 PlayerState에서 가져온다. 로컬 선표시 없이 서버 수신 경로로만 표시해 Host 중복을 피한다.
- 채팅은 모든 경기 상태에서 사용 가능하다. 후입장자에게 과거 기록을 동기화하거나 영구 저장하지 않는다.
- 위험: WBP 부모 클래스/BindWidget 이름 불일치, C++ 빌드 전 에셋 생성, 포커스 이탈의 재진입, 위젯 생성 전 Client RPC 도착. C++ 빌드/PIE/에셋 생성 스크립트는 에이전트가 실행하지 않는다.

## 빌드 후 에셋 생성
1. 에디터를 닫고 Development Editor / Win64 전체 빌드한다.
2. 에디터에서 Python Editor Script Plugin을 활성화하고 필요 시 재시작한다.
3. Tools → Execute Python Script에서 Scripts/create_m04_widgets.py를 실행한다. 이는 검증 스크립트가 아니라 WBP 생성/연결 작업이다.
4. 생성 대상: /Game/UI/WBP_PlayerStats, /Game/UI/WBP_Chat. BP_VehicleAdvPlayerController의 PlayerStatsWidgetClass/ChatWidgetClass를 연결한다. 기존 동명 에셋이 있으면 Designer 내용을 덮어쓰지 않고 필수 위젯과 부모를 확인한다.
5. 생성된 WBP 두 개를 열어 Compile/Save, 컨트롤러 BP도 Compile/Save한다. Designer에서 위치/크기/색상을 수정할 수 있다.

## 수동 생성 대안 및 필수 이름
스크립트가 실패하면 첫 Python 오류 전문을 전달한다. 수동 생성 시 다음을 사용한다.

| WBP | 부모 | 필수 위젯 이름/타입 |
|---|---|---|
| WBP_PlayerStats | CH4_2PlayerStatsWidget | CoinsText, BonusSpeedText, MatchText: TextBlock |
| WBP_Chat | CH4_2ChatWidget | ChatHistory: TextBlock, ChatScroll: ScrollBox, MessageInput: EditableTextBox, FeedbackText: TextBlock |

모든 필수 위젯의 Is Variable을 켠다. ChatHistory는 ChatScroll 아래에 둔다. ChatScroll과 MessageInput/FeedbackText는 같은 패널에 배치한다. MessageInput은 처음 Hidden(공간 유지)이며 코드는 입력 시 Visible로 전환한다. 별도 Blueprint 이벤트 그래프로 전송을 중복 구현하지 않는다.

## 사용자 검증
1. 먼저 생성한 WBP가 컨트롤러에 연결되었는지 확인한다. 연결하지 않으면 네이티브 UI로 대체하지 않고 로그에 설정 누락을 알린다.
2. 2인 Listen Server PIE. 코인/추가 속도 및 5초/60초 경기 상태가 기존처럼 표시되어야 한다.
3. 각 창에서 Enter → 한글/영문 메시지 → Enter. 양쪽 기록에 발신자 이름과 메시지가 정확히 한 번 표시되어야 한다.
4. 입력 종료 후 W/A/S/D 조작과 카메라 입력이 돌아와야 한다. W를 누르다 채팅을 열어도 가속 입력이 계속 유지되지 않아야 한다.
5. Escape 또는 입력 포커스 이탈은 전송 없이 종료. 빈 문자열/공백만 전송하면 메시지가 추가되지 않는다.
6. 200자 초과는 거부 안내, 0.5초 내 재전송은 서버 제한 안내. 이후 정상 메시지는 다시 전송 가능해야 한다. 이름/줄바꿈/제어문자를 통해 다른 발신자처럼 표시할 수 없어야 한다.
7. 각 경기 상태에서 채팅 가능, 최근 50건 제한, 재시작 후 기록 초기화, 위젯 중복 구독/전송 없음.
8. M02 변경 명령과 M03 대기 이탈/재접속 표시도 회귀 확인한다. 새 접속자가 과거 채팅을 받지 않는 것은 현재 정책이다.

## 상태
2026-10-21 WBP 생성 오류 수정 후 사용자 검증 완료 보고 수신. 이후 사용자가 채팅·운전 복귀·기존 코인/타이머 표시까지 확인했다고 보고했다. CHAT-001~006 및 M04는 VERIFIED로 기록한다. 길이/빈도 등 개별 경계 시나리오 결과는 별도 보고받지 않았으며 통합 검증에 남긴다. 에이전트 빌드/스크립트/PIE 미실행.

## WidgetTree 파이썬 접근 오류 수정
- 사용자 실행에서 WidgetBlueprint.get_editor_property("widget_tree")가 공개 속성을 찾지 못해 실패했다. 기존 스크립트가 엔진 파이썬 노출 범위를 잘못 가정했다.
- CH4_2WidgetAuthoringLibrary.h/.cpp 추가: WITH_EDITOR에서 네이티브 WidgetTree를 사용해 에디터 WBP 에셋을 작성한다. 런타임 위젯 생성으로 되돌리는 것이 아니다.
- Build.cs의 UnrealEd/UMGEditor 의존성은 Target.bBuildEditor일 때만 추가한다. 게임 빌드에는 에디터 구현을 포함하지 않는다.
- 파이썬은 새 공개 함수를 호출하며 WidgetTree/RootWidget/is_variable/BlueprintStatus에 직접 접근하지 않는다.
- 기존 빈 Canvas WBP는 이어서 채운다. 비어 있지 않은 레이아웃은 덮어쓰지 않으며 필수 위젯이 없으면 오류를 안내한다.
- 다시 실행: 에디터 종료 → Development Editor / Win64 전체 빌드 → 에디터 재시작 → 같은 create_m04_widgets.py 실행. 기존 빈 에셋은 삭제할 필요 없다.
- 수정 후 사용자 “검증 완료” 보고 수신. 생성 오류 해결 범위로 기록하며 채팅 PIE 전체 결과로 확대하지 않는다. 에이전트 빌드/스크립트/PIE 미실행.


