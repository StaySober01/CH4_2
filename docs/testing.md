# 검증 계획과 결과

## 사용자 PIE 결과 — 2026-10-21
- 사용자 보고: “PIE 검증 완료”. 직전 안내한 VehicleBasic, 2 Players, Play As Listen Server, New Editor Window (PIE) 체크리스트의 통과 보고로 기록한다.
- 확인 범위: 두 차량의 분리 스폰, 양쪽 가속·조향·브레이크, 상대 차량 움직임 표시, 각자 속도·기어 HUD 갱신.
- 증거 형태는 사용자 완료 보고이며 별도 로그/스크린샷은 받지 않았다. 에이전트는 재검증이나 빌드를 실행하지 않았다.
- BASE-003 VERIFIED. BASE-002는 전체 Start 점유 예외 미검증으로 IMPLEMENTED 유지. M01의 기타 예외·기반 편차까지 확인된 것으로 확장하지 않는다.

## 현재 검증 담당 방침 — 2026-10-21
- 사용자가 코드 작업 후 빌드와 모든 검증을 직접 수행한다. 아래 명령과 시나리오는 사용자 실행용이며 에이전트 자동 실행 지시가 아니다.
- 에이전트는 구현에 필요한 코드/API를 읽고 수정하되, 작업 후 별도 빌드/UHT/PIE/테스트/정적 검사 명령을 실행하지 않는다. 명시적인 실행 요청이 있을 때만 예외다.
- 매 작업 보고에 준비 사항, 빌드 명령 또는 IDE 절차, 에디터 설정, 확인 순서, 예상 결과, 실패 시 공유할 로그를 포함한다.
- 사용자 결과가 오기 전에는 미검증으로 기록한다. 사용자가 확인한 항목만 VERIFIED로 변경하며 빌드 통과와 실제 플레이 검증을 구별한다.
- 아래 기존 검사 결과는 이 방침 이전의 이력이다. 신규 변경의 검증 증거로 재사용하지 않는다.

## 단계 1 실제 결과 — 2026-10-21
- 프로젝트/모듈/Target/Config 및 Content 전체 파일 목록 정적 조사 수행.
- 게임 C++ 클래스 및 uasset/umap 부재 확인. 기존 AGENTS/docs 부재 확인.
- 엔진 Build.version에서 설치본 5.6.1 확인. 기존 git 작업 트리는 깨끗했음.
- 문서만 변경했으므로 C++ 빌드/UHT/PIE/자동화 게임 테스트는 이번에 실행하지 않음. 기존 생성 산출물은 성공 근거가 아님.
- 생성 후 정적 검사 통과: 첨부/assignment SHA256 일치, 원문 체크박스 65개와 추적 행의 내용·순서 1:1 일치, ID 중복 없음, 7개 문서 존재 및 모든 T-* 검증 참조 대응 확인.
- 필수 47개/도전 18개는 모두 TODO. 별도 BASE 5개는 체크박스 외 안내의 추적 항목이다.
- git diff로 Source/Config/uproject/README 변경 없음 확인. git status의 신규 항목은 AGENTS.md와 docs뿐이다. git diff --check 통과(신규 untracked 문서에는 적용되지 않는 한계 있음).

## 향후 빌드
프로젝트 루트 PowerShell, 실제 설치 경로 기준:
```powershell
& 'C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' CH4_2Editor Win64 Development '-Project=C:\UnrealProject\CH4_2\CH4_2.uproject' -WaitMutex -NoHotReloadFromIDE
```
UHT 포함 종료 코드와 오류 로그를 기록한다. 에디터/Live Coding 잠금 여부 확인 후 실행하며 실패를 숨기지 않는다. 엔진 로그 등 허용 경로 밖 쓰기에 승인이 필요하면 도구 승인 절차를 따른다.

자동화 테스트 추가 후 사용할 명령 예시(현재 테스트 없음):
```powershell
& 'C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UnrealProject\CH4_2\CH4_2.uproject' -unattended -nop4 -NullRHI '-ExecCmds=Automation RunTests CH4_2; Quit' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=C:\UnrealProject\CH4_2\Saved\TestReports'
```
자동화는 잔액 경계/동률 계산/음수 회복 상한 등 실제 불변조건을 대상으로 한다. NullRHI 결과로 UI·VFX·물리 네트워크를 검증했다고 하지 않는다.

## PIE 준비
차량 기반, 테스트 맵, 안전한 PlayerStart 3개와 GameMode 연결 후 Play As Listen Server, 2 Players, 별도 창으로 실행한다. 단일 프로세스와 별도 프로세스 결과를 구분한다. 배경 Use Less CPU when in Background는 명세 권고에 따라 확인하되 사용자 설정을 임의 변경하지 않는다.

## 시나리오 (추적 표의 검증 ID)
| ID | 수행 및 합격 기준 |
|---|---|
| T-BASE | 맵/폰/GameMode/3개 Start 연결 확인. 두 차량 겹치지 않고 생성, 양쪽 운전과 상대 이동 확인. 사용 중 Start 및 사용 가능한 Start 없음도 크래시 없이 처리 |
| T-CHAT | Host→Client/Client→Host Enter 제출, 양쪽 한 번씩 표시. 빈 값/긴 값/반복 전송/포커스 전환 검사. 위젯은 각 로컬 PC에만 생성 |
| T-PS | Host와 Client 각각 서버에서 코인/버프 변경, 본인 UI 갱신. 원격 비소유 PS에는 값 복제되지 않음을 로그/복제 관찰로 확인. 클라이언트 변경 경로 거부, 음수·오버플로 경계 검사 |
| T-ATTACK | 앞 범퍼/범위/장애물/빗나감/자기 타격/유효하지 않은 대상 검사. 잔액 0·4면 변화 없음, 5면 피해자 0/공격자 +5, 그 이상도 총합 보존. 양쪽 선과 피격 효과 확인, 반복 요청 및 이탈 방어 |
| T-ITEM | 각 플레이어가 코인 +10/속도 +10을 정확히 한 번 획득. 동시 오버랩에 중복 지급 없음. 양쪽 파괴와 효과/소리, 기본 NetUpdateFrequency=1 확인. 실제 속도 효과도 확인 |
| T-SPAWN | 서버에서만 생성, 생성 참조와 OnDestroyed 연결 확인. 파괴 2초 뒤 동일 종류 한 개 생성, 반복 획득/맵 종료/잘못된 클래스에서 중복·잔존 타이머 없음 |
| T-MATCH | 첫 접속으로 Waiting 카운트다운 시작, 서버 1초 감소, Playing→Finished 양쪽 일치. 단독 최고/동률/0원 동률 결과와 각 로컬 승패 확인. 미정 접속 정책은 결정 후 확장 |
| T-WHEEL | PointDamage 서버 Bone 확인, 몸체와 바퀴 구별. 음수 패널티와 시간 회복, 0 초과 회복 없음. 기존 버프 상호작용은 Q-004 결정 기준 |
| T-MINE | F 서버 요청, 뒤쪽 Spawn과 Owner 확인. Host/Client 피해 모두 즉시 throttle 0, 2초 후 해제, 양쪽 폭발/소리/파괴 확인. 반복 오버랩/연속 설치 정책 결정 후 검사 |
| T-GATE | Waiting/Finished에서 throttle/steering 0, handbrake true, reset/공격/지뢰 거부. Playing에서 상태 브레이크 해제. 누른 채 전환/채팅 포커스/서버 직접 요청도 확인 |

## 증거 규칙
각 실행에 엔진/커밋 또는 작업 트리 상태, 맵, NetMode, 프로세스 옵션, 역할, 재현 순서, 예상/실제, 로그·스크린샷 경로를 기록한다. IMPLEMENTED는 파일 존재 및 구현 완료, VERIFIED는 해당 수용 시나리오 성공 증거가 있을 때만 사용한다. 빌드 성공만으로 전체 기능 VERIFIED를 부여하지 않는다.

## 단계 2a 실제 결과 — 2026-10-21
- UE 5.6.1, CH4_2Editor Win64 Development, MSVC 14.38.33145. UHT/C++ 최종 빌드 성공(exit 0).
- 최초 샌드박스 빌드는 UBA의 ProgramData 쓰기 권한 실패. 승인 후 외부 빌드 성공. 최초 샌드박스 UnrealEditor-Cmd는 로그 없이 종료하여 승인 후 다시 실행했다.
- Scripts/setup_vehicle_map.py를 PythonScriptPlugin 명령행 활성화로 실행. 엔진에서 맵 저장/에셋 로드/Blueprint compile 수행, commandlet 결과 0 errors / 0 warnings, exit 0.
- PlayerStart 위치: (4450,0,102), (4450,500,102), (4450,1000,102). 보고서: Saved/Validation/vehicle_map.json, 로그: Saved/Logs/VehicleSetup.log.
- 로그에는 소스 관리 checkout 불가 안내가 있었지만 맵 저장은 성공했고 후속 새 프로세스에서 추가 Start 위치 사용을 확인했다. 선택적 프로파일러 DLL 미설치는 게임 오류로 집계하지 않았다.
- Scripts/test_vehicle_network.ps1 성공(exit 0). 실제 별도 Listen Host/Client, NullRHI, loopback:17877. 호스트/클라이언트 차량 생성 위치가 각각 (4450,0,102)/(4450,500,102)로 다름.
- 서버에 두 PC/Pawn 빙의 기록, 클라이언트에 VehiclePawn OnRep와 로컬 HUD 생성 기록 확인. 검사 후 시작한 프로세스만 종료했다.
- 증거: Saved/Validation/vehicle-host-20261021-135627.log, Saved/Validation/vehicle-client-20261021-135627.log.
- 아직 검증하지 않은 것: 실제 키보드 운전, HUD 렌더링/속도·기어 표시, 물리 복제 품질, 모든 Start 점유 상태, 차량 파괴 리스폰, Reset의 멀티플레이 동작. BASE-002/003은 IMPLEMENTED이며 VERIFIED가 아니다.
- 로그/빌드 산출물은 Saved/Intermediate에 있어 Git에 포함되지 않는다. 재현 스크립트는 Scripts에 포함한다. 현재 커밋에 저장하지 않은 작업 트리에서 검증했다.

재현: 설치 UnrealEditor-Cmd.exe에 프로젝트 절대 경로와 -run=pythonscript -script=<Scripts/setup_vehicle_map.py 절대 경로> -EnablePlugins=PythonScriptPlugin -unattended -nop4 -NullRHI를 전달한다. 네트워크 검사는 프로젝트 루트에서 Scripts/test_vehicle_network.ps1을 실행한다.

맵 구성 스크립트는 3개 미만일 때만 Start를 추가하며 3개 초과면 기존 액터를 삭제하지 않고 실패한다. 접속 스크립트는 숨김 엔진 프로세스 두 개를 생성하고 종료하므로 실행 권한이 필요할 수 있다.

## M02 — 구현 완료 / 사용자 검증 대기
- 신규 PlayerState·네이티브 개인 HUD·PC 이벤트 바인딩·PIE 진단 명령 작성.
- 이번 변경에 대해 에이전트 빌드/UHT/PIE/자동 테스트/작업 후 검사 미실행.
- 사용자 실행 절차와 예시 명령은 [M02 작업 및 검증 안내](m02-player-state.md)에 정리했다.
- 새 클래스가 있으므로 에디터 종료 후 전체 빌드, BP GameMode의 PlayerStateClass 확인, 2인 PIE의 초기 표시·Host/Client 변경·OwnerOnly·잘못된 입력 거부 순서로 확인한다.
- 현재 PS-001~009는 IMPLEMENTED이며 사용자 보고 전 VERIFIED로 변경하지 않는다.

## M02 사용자 수용 결과 — 2026-10-21
- M02 구현 및 검증 안내 이후 사용자가 “검증 완료”를 보고했다.
- 이를 M02 수용 완료로 기록하고 M02/PS-001~009를 VERIFIED로 갱신했다.
- 증거는 사용자 완료 보고다. 개별 실행 로그·스크린샷은 제공받지 않았으며 에이전트가 빌드나 검증을 실행한 것은 아니다.
- 실제 속도 효과(M06), 기타 M01 예외 및 후속 기능까지 확인한 것으로 확대하지 않는다.

## M03 — 구현 완료 / 사용자 검증 대기
- GM-001~003/005~010 구현. GM-006은 사용자 승인 편차(첫 접속 대신 2명 접속).
- 사용자 확정: 5초 대기/60초 경기, 대기 중 2명 미만이면 취소·재충족 시 5초 재시작.
- 이번 변경 빌드/UHT/PIE/자동 테스트/작업 후 검사 미실행.
- 사용자 절차는 [M03 검증 안내](m03-match-flow.md): 클래스 연결, 1인 대기, 2인 카운트다운, 60초 종료, 대기 이탈/재접속, 진행 중 후입장, 기존 HUD 회귀.
- 기존 T-MATCH의 첫 접속 시작 기대값은 이 사용자 정책으로 대체한다. 승패 검사는 M09에서 수행한다.

## M03 사용자 수용 결과 — 2026-10-21
- M03 구현 및 검증 안내 후 사용자가 “검증 완료”를 보고했다.
- M03과 GM-001~003/005~010을 VERIFIED로 갱신했다. GM-006은 승인된 2명 접속 조건을 기준으로 한다.
- 증거는 사용자 수용 보고이며 개별 로그/스크린샷은 받지 않았다. 에이전트 재빌드·재검증은 수행하지 않았다.
- 승패(M09), 상태별 운전 제한(M13)과 기타 후속 기능의 검증 완료를 뜻하지 않는다.

## M04 — C++ 작성 / WBP 생성·검증 대기
- 빌드/UHT/PIE/에셋 생성 스크립트/작업 후 검증 미실행.
- 사용자 절차: docs/m04-chat-wbp.md. C++ 전체 빌드 후 WBP 생성과 BP 연결이 필요하다.
- CHAT-002/003/005는 에셋이 없어 IN_PROGRESS. 다른 채팅 코드 항목은 IMPLEMENTED.
- UI 전환으로 M02/M03의 표시 항목 PS-009/GM-007/GM-009 재검증 필요. 이전 사용자 결과는 변경 전 이력이다.
- 확인 항목: 양방향 한 번 표시, 한글, 입력 포커스 복귀, 공백/초과 길이/빈도 거부, WBP Designer 편집, 기존 개인 값/경기 시간 회귀.

## M04 사용자 오류와 수정
- 사용자 로그: create_m04_widgets.py에서 WidgetBlueprint의 widget_tree 속성 조회 실패.
- 원인: 해당 에디터 위젯 트리가 이 환경의 Python 속성으로 노출되지 않음. 에디터 C++ 보조 라이브러리로 접근 경로 수정.
- 빈 WBP 복구 및 기존 레이아웃 보존, 에디터 빌드에만 의존 모듈 추가.
- 에이전트 빌드/재실행 미수행. 사용자 전체 빌드 후 같은 스크립트 재실행 필요. M04는 IN_PROGRESS 유지.

## M04 WBP 생성 수정 사용자 확인 — 2026-10-21
- WidgetTree 접근 오류 수정 안내 후 사용자가 “검증 완료”를 보고했다.
- WBP 생성 오류 해결과 생성·연결 완료 보고로 기록한다. CHAT-002 VERIFIED, M04 IMPLEMENTED.
- 개별 로그·스크린샷은 제공받지 않았다. 양방향 채팅, 입력 복귀, 제한 처리 및 PS-009/GM-007/GM-009 회귀 확인 여부는 미확정이다.
- 에이전트는 빌드/검증 명령을 실행하지 않았다.

## M04 사용자 수용 범위 확인 — 2026-10-21
- 사용자가 “채팅, 운전 복귀, 기존 코인 타이머 표시까지 확인했어”라고 추가 확인했다.
- 앞선 2인 PIE 양방향 채팅/입력 복귀/기존 HUD 확인 질문에 대한 수용 보고로 CHAT-001~006 및 M04를 VERIFIED 처리한다.
- WBP 전환 표시 회귀에 해당하는 PS-009/GM-007/GM-009와 M02/M03을 VERIFIED로 복원한다. 이전 검증 이력은 보존한다.
- 증거는 사용자 보고다. 별도 로그/스크린샷은 없으며 에이전트 검증은 실행하지 않았다.
- 200자 경계/0.5초 제한/50건 기록 등 개별 예외 검사 결과까지 명시적으로 보고된 것은 아니다. 해당 세부 검사는 M10 통합 검증에서 확인한다.

## M05 구현 및 사용자 검증 대기 — 2026-10-23
- C++ 코인 +10/1Hz/서버 파괴/중복 소비 가드 및 PC 효과 전달 작성.
- 에이전트 빌드/UHT/PIE/에셋 생성/작업 후 검증 명령 미실행.
- 사용자 전체 빌드 명령, BP 생성/맵 배치/효과 연결 및 Host/Client 체크리스트: [M05 안내](m05-coin-pickup.md).
- ITEM-001~003 IMPLEMENTED. ITEM-004는 효과 코드 작성, 에셋 연결 대기로 IN_PROGRESS. M05 IN_PROGRESS.
- 사용자 수용 기준: 각각 +10, 합계 중복 지급 없음, 양쪽 파괴 및 일회성 효과/사운드, 초기 겹침, 종료 시 오재생 없음, 기존 UI 회귀.

## M05 사용자 수용 결과 — 2026-10-26
- M05 코인 BP/효과 연결 및 2인 PIE 안내 후 사용자가 “검증 완료”를 보고했다.
- M05/ITEM-001~004 VERIFIED. 코인 +10, 중복 지급 방지, 1Hz 설정, 양쪽 파괴·효과/사운드의 수용 완료 보고로 기록한다.
- 증거는 사용자 완료 보고이며 개별 로그·스크린샷은 제공받지 않았다. 에이전트 빌드/테스트/PIE/작업 후 검증 미실행.
- M06 이후 기능과 기타 마일스톤의 잔여 예외까지 검증된 것으로 확대하지 않는다.

## M06 구현 — 2026-10-26
- 기본 60km/h, 버프마다 +10 누적. ITEM-005~007 IMPLEMENTED, ITEM-008/M06 IN_PROGRESS(에셋 연결 대기).
- 전체 빌드 명령·BP 설정·2인 PIE 절차는 docs/m06-speed-pickup.md.
- Host/Client 각각 평지 60→70→80, 상대 값 유지, 중복 지급 없음, 양쪽 파괴/효과, 점프 Z 유지, 기존 코인/채팅/HUD 회귀 확인.
- 에이전트 빌드/UHT/PIE/작업 후 검사 미실행. 사용자 결과 수신 전 VERIFIED 처리하지 않는다.

## M06 사용자 수용 결과 — 2026-10-27
- M06 BP 연결·2인 PIE 안내 이후 사용자가 “검증 완료”를 보고했다.
- M06/ITEM-005~008 VERIFIED. 기본 60km/h 및 +10 누적 실제 속도 적용, 중복 획득 방지, 파괴·효과/사운드의 수용 완료 보고로 기록한다.
- 증거는 사용자 보고이며 개별 로그·스크린샷은 제공받지 않았다. 에이전트 재빌드/재검증은 실행하지 않았다.
- M07 이후 기능 및 다른 단계의 잔여 예외까지 검증된 것으로 확대하지 않는다.

## M07 구현 — 2026-10-27
- 서버 전용 아이템 스포너, 참조/OnDestroyed 구독, 2초 재생성 및 종료 정리 작성.
- SPAWN-001~003 IMPLEMENTED, SPAWN-004/M07 IN_PROGRESS(실제 맵 배치 대기).
- 사용자 전체 빌드/BP/맵 설정/2인 PIE 절차: docs/m07-item-spawner.md.
- 반복 획득, 동시 접근, 생성 위치 정차, 재생성 대기 중 종료, 잘못된 클래스, 기존 기능 회귀 확인.
- 에이전트 빌드/UHT/PIE/작업 후 검증 명령 미실행.
