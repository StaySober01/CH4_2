# 진행 현황

기준일: 2026-10-21. M00 VERIFIED, M01 IN_PROGRESS. Vehicle 템플릿 도입은 완료했으며 수동 운전 검증은 남아 있다. 마일스톤 기준은 implementation-plan.md의 M00~M14를 따른다.

## 완료
- 단계 1: 프로젝트 분석, 원문 보존, 65개 요구사항 추적, 설계/검증 하네스 작성.
- UE5.6 TP_VehicleAdv C++/콘텐츠와 Vehicles/Input 공유 콘텐츠를 CH4_2에 도입.
- ChaosVehicles/RawInput 및 모듈 의존성, 기본 맵/GameMode, 클래스 redirect 연결.
- 서버 Start 점유 검사, PlayerStart 3개 배치, 소유 클라이언트의 VehiclePawn 참조 복제.
- 재실행 가능한 Scripts/setup_vehicle_map.py, Scripts/test_vehicle_network.ps1 작성.

## 검증
- UHT/C++ 빌드 성공. 최초 샌드박스 UBA 쓰기 권한 실패 후 승인된 외부 실행으로 성공.
- 엔진 맵 구성/Blueprint 검사: 0 errors, 0 warnings. PlayerStart 3개 저장.
- 별도 Listen Host/Client: 서로 다른 위치에 차량 빙의, 원격 Pawn 참조 수신, 로컬 HUD 생성 로그 확인. 프로세스 종료 완료.
- 실제 화면/키보드 운전/물리 동기화 품질/모든 Start 점유 예외/리스폰은 미검증.

## 다음 단계
- 사용자 지시 대기. 다음 작업은 M01의 에디터 2인 운전·HUD·예외 검증과 기반 편차 정리. 이후 M02 PlayerState + 개인 HUD를 진행한다.
- 코인/속도 정책(Q-002~006 등)은 해당 구현 전 결정. 게임 규칙 65개는 모두 TODO.
- 기존 GameMode/PC/Pawn/UI를 확장한다. 이름을 맞추기 위한 클래스 재작성은 하지 않는다.

## 남은 차이와 제한
- UE5.6/CH4_2 유지 승인. 과제의 Scalable/Starter Content는 미적용.
- 원본 템플릿 Reset은 아직 서버 RPC/상태 제한으로 보완되지 않았다.
- 다른 variants는 함께 도입했으나 활성 과제 맵이 아니며 별도 플레이 검증하지 않았다.
- 테스트 상세·증거는 testing.md, 변경 범위는 vehicle-template-integration.md 참조.

## 계획 정리 이력
- 2026-10-21: 기존 단계를 M00~M14 작업 마일스톤으로 재편. 필수 47개/도전 18개에 담당 마일스톤·선행 조건·완료 기준을 연결했다. 이번 변경은 문서만이며 후속 게임 구현은 시작하지 않았다.

