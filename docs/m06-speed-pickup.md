# M06 속도 버프

## 구현 전 계획 — 2026-10-26
- 사용자 다음 작업 지시로 M06 승인. 스피드 버프 서버 획득 시 BonusMaxSpeed +10, 1Hz, 중복 방지·파괴·효과 전달.
- 신규 CH4_2SpeedPickup.h/.cpp, PC 초기 겹침 처리 확장. 기존 코인 구현은 유지.
- 실제 차량 적용의 단위/기본 최고속도는 사용자 질문 응답 대기. 해당 정책 의존 구현은 응답 후 진행.
- 위험: 서버/소유 클라이언트 물리 차이, OwnerOnly 값의 비소유 차량 사용, 공중/충돌 속도 보정, 실제 엔진 출력으로 목표 속도에 도달하지 못하는 경우.
- 완료 기준: Host/Client 각각 +10 데이터·실제 최고속도 변화, 중복 획득 방지, 양쪽 파괴·효과, 기존 코인/HUD 회귀.
- 에이전트 빌드/UHT/테스트/PIE/작업 후 검사 미실행. BP 생성·효과 연결·검증은 사용자 담당.

## 확정 정책 및 구현
- 사용자 선택: 기본 60km/h 제한, 획득마다 +10km/h 누적. 60 → 70 → 80. 시간 만료 없음.
- Pawn BaseMaxSpeedKmh 기본값 60. 기존 Tick에서 수평 물리 속도만 제한한다. 엔진 출력/가속력은 변경하지 않는다.
- 서버와 소유 클라이언트만 본인의 PlayerState 값을 읽는다. 비소유 프록시는 복제 물리를 따른다. BonusMaxSpeed의 OwnerOnly 조건 유지.
- 점프/낙하의 Z 속도는 유지한다. 전진/후진/횡방향의 수평 속도는 모두 제한되며 충돌·경사에서 생긴 초과 수평 속도도 잘린다.
- 매 프레임 보정이므로 물리 서브스텝 사이의 미세 초과와 네트워크 보정은 가능하다. 엔진 출력이 부족하면 높은 누적 제한까지 도달하지 못할 수 있다. 제한 증가는 즉시 가속을 뜻하지 않는다.
- 별도 Tick/타이머/영구 델리게이트를 추가하지 않는다. 현재 PS를 읽으므로 재빙의/차량 교체에도 누적값이 적용된다. PS가 아직 없으면 기본 60을 적용한다.
- CH4_2SpeedPickup은 서버 소유/빙의/겹침 검사와 소비 가드 후 IncreaseBonusMaxSpeed(10)을 호출한다. 성공 시 각 PC 효과 전달 후 Destroy. 1Hz 설정, 초기 겹침 및 EndPlay 정리 포함.
- 기본 Cube 메시로 코인과 구별한다. 파티클/사운드는 사용자 BP에서 연결한다. WBP 레이아웃은 유지하고 기존 추가 속도 텍스트에 km/h 단위를 표시한다.

## 사용자 빌드 및 에디터 설정
1. 에디터 종료 후 Development Editor / Win64 전체 빌드:

    & 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' CH4_2Editor Win64 Development '-Project=C:\UnrealProject\CH4_2\CH4_2.uproject' -WaitMutex -NoHotReloadFromIDE

2. CH4_2SpeedPickup 부모로 /Game/Items/BP_SpeedPickup 생성. 기존 BP_CoinPickup을 복제해서 부모를 바꾸지 않는다.
3. Class Defaults → Speed Buff → Effects에서 비반복 Pickup Effect(Particle System)/Pickup Sound 지정. M05에서 사용한 에셋을 재사용해도 된다.
4. Sphere는 Query Only, Vehicle/Pawn Overlap, 반지름 100, Generate Overlap Events 켜짐. Replicates 켜짐, Net Update Frequency와 Min Net Update Frequency는 1.
5. 실제 차량 BP의 Vehicle → Speed → Base Max Speed Kmh가 60인지 확인한다. 기존 값을 직접 덮어쓴 BP는 기본값으로 재설정한다.
6. BP Compile/Save 후 VehicleBasic 평지에 버프 2개 이상을 배치한다. 자동 재생성은 아직 없다. 기존 WBP는 새로 만들 필요 없다.

## 사용자 검증
1. 2 Players / Play As Listen Server / New Editor Window (PIE). 평탄한 직선에서 Host/Client 각각 기본 최고속도가 약 60km/h인지 확인한다.
2. Host 버프 1개 획득: Host 추가속도 10km/h, Coins 유지, 직선 최고속도 약 70. Client 추가속도 0과 제한 60 유지.
3. Host 2번째 획득: 추가속도 20, 제한 약 80. 가속 시간은 여전히 필요하다.
4. 새 PIE에서 Client도 같은 순서로 60→70→80 확인. 양쪽에서 아이템 파괴와 파티클/사운드가 한 번씩 나와야 한다.
5. 같은 버프에 동시 접근/복수 바디 겹침 시 한 사람에게만 +10. 다시 지나가도 재지급 없음. 초기 겹침/빙의 후 획득도 한 번.
6. 후진·점프·경사에서 수직 움직임이 유지되고 심한 떨림이 없는지 확인. 속도 HUD가 전체 속력을 표시하면 공중 Z 성분 때문에 수평 제한보다 높게 보일 수 있으므로 최고속도 비교는 평지에서 한다.
7. 차량 Reset/재빙의 후 동일 PlayerState의 누적 버프 유지. PIE 재시작 시 0.
8. 기존 코인 +10, 채팅/운전 복귀 및 5초/60초 타이머 회귀 확인.
9. PIE 호스트 기존 CHStatsList / CHStatsAddSpeed <PlayerId> 10 명령도 실제 제한을 올린다. 아이템 검증은 디버그 명령 없이 별도 실행한다.

## 상태 및 오류 전달
- ITEM-005~007 IMPLEMENTED, ITEM-008 및 M06 IN_PROGRESS(실제 BP·효과 연결 및 사용자 검증 대기).
- 빌드/UHT/PIE/작업 후 검증 명령은 에이전트 미실행.
- 실패 시 첫 빌드 오류, 어느 창인지, 버프 전후 값과 평지 최고속도, 차량 BP의 Base Max Speed Kmh, 연결한 효과 경로, Saved/Logs/CH4_2.log의 Speed pickup collected 및 경고를 전달한다.
- 다음 지시 후 M07 아이템 스포너(2초 재생성) 진행.

## 사용자 수용 결과 — 2026-10-27
- 위 구현 및 설정/검증 안내 후 사용자가 “검증 완료”를 보고했다.
- M06/ITEM-005~008 VERIFIED로 갱신. 위 구현 직후 대기 상태는 과거 이력으로 보존한다.
- 증거는 사용자 수용 보고이며 개별 로그·스크린샷은 제공받지 않았다. 에이전트 빌드/검증 미실행.
- 다음 작업은 사용자 지시 후 M07 아이템 스포너다.
