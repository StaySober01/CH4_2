# 검증 계획과 결과

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
