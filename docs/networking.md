# 네트워크 설계

현재 구현: ACH4_2GameMode의 서버 차량 스폰 위치 선택, ACH4_2PlayerController의 VehiclePawn OwnerOnly 복제와 로컬 HUD, 템플릿 Chaos 차량 이동 기반. 별도 Host/Client에서 접속·빙의·참조 수신을 확인했다. 아래 코인/채팅/게임 흐름/공격/아이템/도전 설계는 여전히 미구현이다.

차량 Reset은 원본 템플릿의 로컬 물리 초기화 방식을 유지한다. 멀티플레이용 서버 요청/게임 상태 제한은 후속 설계·구현이 필요하며 현재 네트워크 완성 기능으로 간주하지 않는다.

Authority는 해당 액터의 서버 권한, Ownership은 RPC 라우팅과 소유자 전용 복제의 연결 관계, IsLocalController/IsLocallyControlled는 로컬 입력·표시 여부다. 서로 대체하지 않는다. Listen Host는 서버이면서 로컬 플레이어이므로 두 경로 중복 실행을 주의한다.

## 소유권 및 복제
| 객체/값 | 변경 위치 | 수신 범위/경로 |
|---|---|---|
| GameMode/타이머 | 서버 | 클라이언트에 존재하지 않음 |
| GameState 단계·남은 시간·승자 | 서버 | 모든 연결에 프로퍼티 복제 |
| PlayerState Coins/BonusMaxSpeed | 서버 API | 각각 DOREPLIFETIME_CONDITION(..., COND_OwnerOnly) |
| PlayerController | 서버와 해당 소유 클라이언트 | 다른 클라이언트 PC를 로컬에서 찾지 않음 |
| 소유 Pawn | 서버 Spawn/Possess | 소유 클라이언트에서 Server RPC, 이동 복제는 차량 기반 검증 필요 |
| Pickup | 서버 Spawn/Destroy | bReplicates, NetUpdateFrequency=1, 복제 파괴 |
| Spawner | 서버 책임 | 배치 정보 외 상태 복제는 필요성 확인 후 결정 |
| Mine | 서버 Spawn, Owner=설치 Pawn | 복제 액터, Owner만 지정한다고 소유자 피해 면제되는 것은 아님 |

PlayerState의 기본 Owner 관계를 유지하고 소유 연결에 실제 도달하는지 원격 클라이언트에서 검사한다. OwnerOnly는 서버까지 값을 숨기는 보안 장치가 아니다.

## 기능 흐름과 RPC 계약
| 기능 | 호출 주체 → 실행 위치 | 서버 검증 및 결과 |
|---|---|---|
| 채팅 | 로컬 Widget Enter → 소유 PC ServerSubmitChat → 서버 | 빈 입력·길이·빈도 검사(한도 미정), 이름은 서버 PC/PS에서 가져옴. 서버가 각 PC의 ClientReceiveChat 호출, 로컬 위젯 표시 |
| 공격 | 로컬 좌클릭 → 소유 Pawn ServerFire → 서버 | 소유/빙의·대상·공격 빈도 검사, 범퍼 위치와 방향은 서버에서 산출. Trace → ApplyDamage → 피해 처리 → 피해자 5원 이상이면 -5/+5 |
| 코인/버프 | 서버 Overlap → 서버 Pickup | 유효 Pawn/PS·중복 획득 검사. +10원 또는 BonusMaxSpeed +10 → 소비 처리 → Destroy |
| 리스폰 | 서버 OnDestroyed → 서버 Spawner | 기존 아이템 제거, 2초 타이머 후 동일 클래스 생성, 생성 실패/종료 방어 |
| 매치 | 첫 접속 → 서버 GameMode | Waiting 타이머 1초 감소 → Playing 타이머 → Finished 및 동률 포함 승자 계산 → GameState 복제 |
| 부위 공격 (도전) | ServerFire → ApplyPointDamage | 서버 HitResult Bone 판별, 바퀴일 때 음수 패널티, 서버 타이머로 0까지만 회복 |
| 지뢰 (도전) | 로컬 F → 소유 Pawn ServerPlaceMine | Playing·빙의·설치 빈도/위치 검사, 뒤쪽 Spawn 및 Owner 설정. 서버 Overlap에서 가속 0, 2초 후 해제 |

공격 탈취는 동일 서버 처리에서 양쪽 PS 유효성과 잔액을 확인하고 중복 피해 이벤트로 이중 지급되지 않게 한다. 피해 대상, 금액, 명중 Bone을 클라이언트 인자로 신뢰하지 않는다.

## 표시와 일시적 효과
- 지속 값: 서버 setter → 공통 변경 알림, 원격 OnRep → 동일 변경 알림 → HUD. 서버에서 C++ 프로퍼티를 대입했다고 OnRep가 자동 호출된다고 가정하지 않는다.
- 초기 UI는 현재 스냅샷을 읽고, 이후 이벤트를 구독한다. GameState와 PlayerState 복제 도착 순서에 의존하지 않는다.
- Trace 선/피격 파티클은 서버 판정 결과를 Pawn의 cosmetic NetMulticast로 전달하는 안. Dedicated Server에서는 렌더링을 건너뛴다. 게임 결과는 이 RPC 성공에 의존하지 않는다.
- 픽업/지뢰 파괴 효과는 RPC 직후 Destroy로 누락될 위험이 있다. 오래 살아 있는 전달 액터 또는 소비 상태·종료 사유 기반 방식 중 실제 2창 테스트 후 결정한다. 모든 EndPlay에서 효과를 내면 맵 종료에도 잘못 재생되므로 금지한다.
- 도전 입력 제한은 로컬 입력뿐 아니라 서버 실행에서도 검사한다. Waiting/Finished는 throttle/steering 0, handbrake true, reset/공격/지뢰 거부. Playing 진입 시 상태용 브레이크 잠금만 해제한다.
- 지뢰 장애는 소유 클라이언트 입력과 서버 이동 모두에 반영되어야 한다. Owner 설정 자체가 가속 차단을 동기화하지 않는다.

## 연결 예외
후입장자는 GameState 스냅샷을 받아야 한다. 재접속·이탈·3번째 접속·종료 후 재시작 정책은 미정이다. 기존 플레이어 수와 무관하게 첫 접속 카운트다운이라는 원문 조건을 보존한다. 차량 물리 예측과 속도 제한은 템플릿 선정 후 Host/Client 각각 검증한다.

