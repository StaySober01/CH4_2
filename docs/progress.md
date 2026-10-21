# 진행 현황

기준일: 2026-10-21. M04 IN_PROGRESS: C++ 구현 및 WBP 생성 스크립트 작성, 사용자 빌드 후 에셋 생성/연결 대기.

## 이번 변경
- UCH4_2ChatWidget: WBP의 ChatHistory/ChatScroll/MessageInput/FeedbackText BindWidget, Enter 제출과 Escape/포커스 이탈 취소.
- PC: 소유 PC의 ServerSubmitChat, 서버 검증 후 각 PC ClientReceiveChat, 최근 50건 로컬 기록, 전송 거부 안내.
- 사용자 확정 200자/0.5초 제한. 발신자 이름은 서버 PS 사용, 공백/제어문자 처리.
- Enter로 UIOnly 입력 전환, 기존 차량 입력 중립화, 종료 시 GameOnly 복귀.
- 기존 개인/경기 패널은 네이티브 RebuildWidget을 제거하고 WBP_PlayerStats의 BindWidget으로 전환.
- Scripts/create_m04_widgets.py: 빌드 후 WBP 두 개 생성 및 BP_VehicleAdvPlayerController 클래스 연결. 미실행이며 WBP 바이너리는 아직 없다.

## 상태와 사용자 작업
- CHAT-001/004/006: IMPLEMENTED. CHAT-002/003/005: IN_PROGRESS(에셋 생성·연결 필요).
- M02/M03은 이전 사용자 검증 이력을 보존하되 UI 변경으로 PS-009/GM-007/GM-009 및 통합 마일스톤 재검증 대기.
- 사용자가 에디터 종료 후 전체 빌드 → Python Editor Script Plugin 활성화 → Tools/Execute Python Script에서 Scripts/create_m04_widgets.py 실행 → WBP/BP Compile·Save → 2인 PIE 확인.
- 생성 이전에는 개인/경기/채팅 패널이 표시되지 않으며 로그에 클래스 연결 누락이 나온다. 기존 차량 속도·기어 WBP는 유지된다.
- 에이전트는 빌드/에디터 실행/스크립트 실행/작업 후 검증 명령을 실행하지 않았다.

## 다음 작업
- docs/m04-chat-wbp.md에 따라 사용자 생성/검증 결과 수신 후 오류 수정 또는 상태 갱신.
- 이후 다음 지시에 따라 M05 코인 아이템.
- 승자 판정 M09, 상태별 운전 잠금 M13, 실제 속도 버프 M06은 아직 범위 밖.

## 현재 수정 사항: WBP 생성 오류
- 사용자 생성 스크립트 실행 중 widget_tree 접근 실패. 빈 WBP가 남아 있을 수 있어 에셋 존재/완료를 단정하지 않는다.
- CH4_2WidgetAuthoringLibrary와 에디터 전용 의존성을 추가하고 Python을 호출용으로 단순화했다.
- 사용자 에디터 종료/전체 빌드/재시작 후 스크립트 재실행 대기. 에이전트 빌드·검증 미실행.
