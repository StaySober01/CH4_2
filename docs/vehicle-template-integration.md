# 단계 2a: Vehicle 템플릿 도입

사용자 승인: 2026-10-21, “vehicle 템플릿을 사용하자.” 기존 CH4_2/UE5.6 프로젝트에 설치된 TP_VehicleAdv C++ 템플릿을 도입한다.

## 범위와 계획
- Source/CH4_2: 템플릿 이름을 CH4_2로 치환. 공식 C++ 템플릿에 포함된 SportsCar/Offroad/TimeTrial도 에셋 부모 의존성 보존을 위해 함께 가져온다. 과제 구현으로 집계하지 않는다.
- Content: 템플릿 맵/Blueprint 및 TemplateDefs의 Vehicles(Standard), Input(High) 공유 콘텐츠를 지정 MountName으로 복사한다. 바이너리는 수정하지 않고 엔진에서만 저장한다.
- Config/DefaultEngine.ini: 시작 맵/GameMode, 기존 템플릿 클래스 참조용 redirect를 추가한다. 프로젝트 ID와 기존 렌더 설정은 유지한다.
- Build.cs/uproject: ChaosVehicles/PhysicsCore/UMG/Slate, ChaosVehiclesPlugin/RawInput 등 템플릿 의존성을 연결한다.
- PC의 Pawn 참조는 복제로 원격 UI에 전달한다. GameMode는 차량 점유 공간을 검사해 Start를 선택한다. 서버가 스폰/빙의하며 로컬 PC만 UI를 만든다.
- 엔진 도구로 테스트 맵의 PlayerStart를 총 3개로 구성하고 Blueprint 로드/컴파일을 검사한다.

## 완료 기준과 위험
UHT/C++ 빌드, 참조/Blueprint/맵 검사, 가능한 실행 검증을 기록한다. 2인 실제 운전/시각 효과는 실행 증거 없이 VERIFIED로 처리하지 않는다. 복사 누락과 클래스 rename으로 인한 BP 부모 단절, World Partition 외부 액터 누락, 차량 스폰 충돌이 주요 위험이다.

코인/채팅/매치 규칙/공격/아이템은 이번 범위 밖이다. 게임 수치와 추가 정책은 계속 미정이다.

## 수행 결과
- 코드/에셋/설정 도입, GameMode의 Start 점유 검사, PC의 OwnerOnly VehiclePawn 복제 완료.
- 맵의 PlayerStart 3개 저장. 공식 템플릿 콘텐츠 235개와 추가 외부 Start 액터 2개로 총 237개 콘텐츠 파일.
- UHT/C++ 빌드 및 Blueprint commandlet 성공. 별도 Listen Host/Client 접속·차량 생성·참조 복제 확인.
- 시각적 UI·실제 운전과 예외 경로는 미검증. 상세 증거는 testing.md.
- Starter Content 추가/Scalable 전환은 하지 않았고 기존 프로젝트 ID/렌더 설정을 유지했다.
