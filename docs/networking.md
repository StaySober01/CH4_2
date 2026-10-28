# 네트워크 설계

현재 구현: ACH4_2GameMode의 서버 차량 스폰 위치 선택, ACH4_2PlayerController의 VehiclePawn OwnerOnly 복제와 로컬 HUD, 템플릿 Chaos 차량 이동 기반. 별도 Host/Client에서 접속·빙의·참조 수신을 확인했다. M02의 개인 Coins/BonusMaxSpeed 서버 변경·OwnerOnly 복제·개인 HUD 연결은 사용자 검증 완료 보고를 받았다. M03 게임 흐름/공용 HUD는 사용자 검증 완료 보고를 받았다. 채팅/승패/공격/아이템/도전 설계는 미구현이다.

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
| 매치 | 2명 접속 → 서버 GameMode | Waiting 타이머 1초 감소 → Playing 타이머 → Finished 및 동률 포함 승자 계산 → GameState 복제 |
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
후입장자는 GameState 스냅샷을 받아야 한다. 재접속·이탈·3번째 접속·종료 후 재시작 정책은 미정이다. 원문 첫 접속 카운트다운 조건은 사용자 지시로 2명 접속/5초 대기/60초 경기로 변경했다. 차량 물리 예측과 속도 제한은 템플릿 선정 후 Host/Client 각각 검증한다.


## M02 실제 연결
서버 ACH4_2PlayerState의 AddCoins/TrySpendCoins/IncreaseBonusMaxSpeed → OnStatsChanged → 로컬 Host PC의 RefreshPlayerStats. 소유 Client는 프로퍼티 복제 → OnRep → 동일 이벤트 → 본인 패널 갱신. PC는 처음 연결할 때 값도 읽으므로 알림이 위젯 생성보다 빨라도 초기 표시를 복구한다.

PIE 디버그 Exec 명령은 네트워크 RPC가 아니다. 변경은 PIE 서버 PC에서만 허용하며 Shipping에서는 대상 검색이 항상 실패한다. 일반 클라이언트의 임의 보상 요청 경로를 추가하지 않았다. 자세한 사용자 확인 절차는 m02-player-state.md.


## M03 흐름
PostLogin/Logout → 서버 GameMode의 참가 인원 갱신 → Waiting 카운트다운 시작/취소 → 서버 1초 타이머 → GameState 스냅샷 변경. Host는 서버 변경 이벤트, 원격 Client는 OnRep_MatchSnapshot으로 같은 HUD 갱신 경로를 사용한다. 초기/후입장 표시는 현재 스냅샷을 읽는다. 클라이언트가 시간·단계를 변경하는 RPC는 없다.


## M04 채팅 구현 (WBP 생성/연결 및 검증 대기)
소유 로컬 PC에서 Enter → WBP 입력 → ServerSubmitChat → 서버의 PS 소유 관계/200자/공백/0.5초 검사 → 월드의 각 CH4_2 PC에 ClientReceiveChat → 로컬 기록과 WBP 갱신. 서버에서 정한 이름만 사용하며 Host도 동일 ClientReceiveChat 경로로 한 번 표시한다. 거부 안내는 발신자 PC의 ClientChatRejected로만 전달한다. 위젯 생성보다 수신이 빨라도 PC의 최근 50건 기록을 생성 후 표시한다. 비소유 PC에 Server RPC를 보내는 것은 Unreal 소유 연결 규칙이 거부한다.

## M05 코인 구현 — 2026-10-23
서버 Sphere 오버랩 → 차량/PC/PS 소유·빙의 및 실제 겹침 검사 → 소비 가드 → AddCoins(10) → 각 PC ClientPlayPickupEffects → 서버 Destroy.
클라이언트 코인은 충돌을 비활성화하며 획득 Server RPC를 제공하지 않는다. 서버 코인 파괴와 OwnerOnly Coins 복제가 지속 상태를 전달한다.
효과는 좌표와 에셋 참조만 지속 PC 채널로 보내며 코인 채널의 소멸과 분리한다. 다른 채널 사이의 표시 순서는 보장하지 않는다. 후입장 과거 효과 없음. EndPlay는 델리게이트 해제만 수행한다.
실제 2인 파괴/효과 수신 검증은 사용자 대기. 앞선 M04 미구현/생성 대기 설명은 이후 사용자 확인으로 갱신되었다.

## M06 속도 제한
서버 버프 획득 → PS IncreaseBonusMaxSpeed(10) → OwnerOnly 복제/HUD 갱신. 서버 및 소유 Pawn이 현재 PS로 제한을 계산한다. 원격 비소유 프록시에 Bonus를 공개하거나 제한값을 별도로 복제하지 않는다. 코인과 동일한 지속 PC 효과 RPC 사용. 실제 물리 보정 품질은 2인 PIE 사용자 검증 대기.

## M07 스포너
스포너는 서버 전용 비복제 액터이며 Client에 생성 요청 RPC를 제공하지 않는다. 서버에서 생성한 코인/버프 자체가 복제되고, 서버 파괴 이벤트가 2초 타이머를 예약한다. 생성된 아이템 Owner는 스포너이며 플레이어 소유를 요구하지 않는다(획득은 서버 오버랩). 클라이언트 보상·효과 경로는 M05/M06과 동일하다. 종료/스트리밍 해제는 재생성하지 않는다.

## M08 공격
소유 로컬 Pawn의 Started → 인자 없는 ServerAttack. 서버 PC/PS/빙의 확인 및 2초 쿨타임 → 서버 범퍼부터 500cm Visibility Trace → 유효 차량 ApplyDamage. 대상 TakeDamage는 공격자 서버 명중 문맥을 한 번만 소비하고 PS 간 5원 이동을 수행한다.
돈은 기존 OwnerOnly 상태 복제/서버 알림으로 표시한다. 서버의 양쪽 잔액 합계는 보존하며 각 클라이언트 UI 수신 시각은 다를 수 있다.
선/명중 파티클은 각 소유 PC Reliable Client RPC로 전송하며 피해량/잔액을 공개하지 않는다. 비권한 직접 TakeDamage/PS 변경은 거부된다. 후입장자는 과거 공격 효과를 받지 않는다. 실제 2인 검증 대기.

## M09 승패
서버 종료 시점에 Coins를 비교하거나 참가자 Logout에서 남은 참가자를 승자로 확정한다. Phase/EndReason/확정 결과가 같은 GameState 스냅샷으로 모든 연결에 복제된다. 우승자 ID·이름만 공개하며 Coins OwnerOnly 유지.
Host는 서버 OnMatchChanged, Client는 OnRep_MatchSnapshot으로 기존 WBP를 갱신한다. 본인 PlayerId 도착 시에도 UI 재평가. 결과는 RPC 일회성 알림에 의존하지 않아 후입장에도 전달된다. 종료 이후 이탈/코인 변경으로 결과를 다시 계산하지 않는다. Listen 서버 프로세스 종료 시에는 세션도 종료되므로 승리 전송은 불가능하다.
