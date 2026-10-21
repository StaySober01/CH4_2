# 진행 현황

기준일: 2026-10-21. M00 VERIFIED, M01 기본 2인 PIE 확인(일부 예외·편차 잔여), M02 VERIFIED, M03 VERIFIED(사용자 완료 보고).

## M03 구현
- ACH4_2GameState: Waiting/Playing/Finished와 남은 정수 초, 접속 인원, 카운트다운 활성 여부를 구조체로 공용 복제.
- ACH4_2GameMode: 서버 PostLogin/Logout 참가자 추적, 2명 접속 시 5초 대기, 60초 경기, 종료. EndPlay 타이머 정리.
- 대기 중 2명 미만이면 취소하고 다시 모이면 5초부터 시작(사용자 선택).
- PC: GameStateSetEvent 및 변경 델리게이트 연결, 초기 스냅샷 읽기, 종료 구독 해제.
- 기존 개인 패널에 경기 상태·시간 표시 추가. 로컬 타이머/Tick 추가 없음.
- GM-001~003/005~010 VERIFIED. 원문 GM-006의 첫 접속 시작은 사용자 지시로 2명 접속 시작으로 변경.

## 검증
- 이번 M03에 대해 빌드/UHT/PIE/자동 테스트/작업 후 검사 명령을 실행하지 않았다.
- 사용자가 M03 안내 후 “검증 완료”를 보고했다. 사용자 수용 보고를 근거로 기록하며 개별 로그/스크린샷은 제공받지 않았다.
- 기존 M02 이력과 이번 M03 사용자 수용 보고는 testing.md에 구분해 기록했다.

## 다음 작업
- M03 검증 완료 반영. 다음 단계 시작 지시 대기.
- 다음 지시 후 M04 채팅. 승자 목록/승패는 M09, 상태별 운전 제한은 M13.
- Playing 중 이탈은 시간 유지, 후입장 시 현재 스냅샷 수신, Finished 자동 재시작 없음.

## 잔여 사항
- Blueprint GameMode의 GameStateClass/PlayerStateClass override는 사용자 확인.
- M01 전체 Start 점유 예외·리스폰·Scalable/Starter Content 편차는 남아 있다.
- BonusMaxSpeed 실제 적용/단위는 M06, 재접속 개인 데이터 보존은 미구현.

