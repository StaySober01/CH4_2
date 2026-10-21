## **분노의 질주: 원화 채굴**

<aside>
<img src="notion://custom_emoji/83c75a39-3aba-4ba4-a792-7aefe4b07895/22a2dc3e-f514-8093-86f1-007a320f43a0" alt="notion://custom_emoji/83c75a39-3aba-4ba4-a792-7aefe4b07895/22a2dc3e-f514-8093-86f1-007a320f43a0" width="40px" />

### 2명의 플레이어가 제한 시간 동안 누가 더 많은 원화를 모으는지 경쟁!

---

여러 번 읽는 것 보다 한 번 해보는게 이해가 쉽다고들 하죠?
서버-클라이언트 구조의 데이터 흐름을 이해할 수 있는 과제를 준비했습니다.

멀티 플레이어 환경에서는 "누가 이 데이터를 가지고 있는가?"와 
"어떻게 전달하는가?"가 가장 중요해요.
서버-클라이언트 구조에서 데이터가 동기화되는 흐름을 직접 설계해 봅시다. 
직접 코드를 짜다 보면 막연했던 언리얼 네트워크의 개념이 명확하게 잡힐 거예요.

</aside>

<aside>
💡

### 학습 목표 : 이번 과제로 무엇을 배우나요?

---

- **멀티플레이어 기초:** Server/Client 간의 역할 분담과 RPC 통신 이해
- **PlayerState :** 플레이어의 개별 데이터를 독립적으로 관리/동기화하기
- **GameModeBase:** 게임의 전체적인 규칙을 서버 측에서 제어하기
- **UMG 연동:** C++ 로직의 결과를 위젯을 통해 플레이어에게 시각적으로 전달하기
- **유효성 검사:** 잘못된 입력이 서버 로직을 해치지 않도록 방어 코드 작성하기
</aside>

<aside>
😊

### Goal : 이 과제를 끝내면 무엇을 할 수 있나요?

---

- 멀티플레이어 게임에서 가장 중요한 **판정 로직의 서버 집중화**와
**상태 동기화**의 메커니즘을 완벽히 이해하게 됩니다.
- 이는 이후 배틀로얄 게임의 생존 판정이나, 
FPS 게임의 데미지 계산 시스템 등을 구축하는 데 탄탄한 밑거름이 됩니다!
</aside>

## 2️⃣ 개발 프로세스 가이드

---

<aside>
⚾

### **1단계 : 요구사항 분석 및 설계**

- 새로운 프로젝트를 만들고, 게임 룰을 숙지한 후, 개발을 시작해보세요.
- 구현을 시작하기 전에 데이터가 서버와 **클라이언트** 사이를 
어떻게 오가는지 설계해 보세요. 텍스트나 순서도로 흐름을 정리하면 
RPC를 어디에 배치할지 명확해집니다.
- 문제를 읽고, 구현을 시작하기 전에 구현해야 할 프로그램의 
입력과 출력 흐름을 그림이나 글로 정리해보세요.
</aside>

---

<aside>
⚾

### **2단계 : 코드 작성 및 구현**

- **함수 모듈화: 각 기능 별로 함수를 잘 나눠주는 것이 중요합니다.**
    - CanFire(): 공격 가능 확인 함수
    - Fire(): 실제 공격 함수
- **네트워크 함수: 서버에서 실행될 함수는 `Server`, 
모든 클라이언트에 알릴 함수는 `NetMulticast` 지정자를 잊지 마세요!**
</aside>

---

<aside>
⚾

### **3단계 : 테스트 및 디버깅**

- 유저들은 기상천외한 플레이를 선보입니다. 
예외상황을 방어할 수 있도록 코드를 작성해보세요.
- 빌드 에러가 나면 `LNK2019` 같은 링크 에러인지, 단순 오타인지 
메시지를 꼼꼼히 읽어보는 습관이 중요합니다!
</aside>

---

## 3️⃣ 필수 기능 가이드

<aside>
💡

- 필수 기능은 반드시 구현해야 합니다.
- 아래 항목을 개인 노션으로 복사해서, 
각 항목을 완료할 때마다 체크리스트를 표시하며 진행해보세요.
    - 기능 구현을 더 세세하게 나눠두었습니다. 과제 페이지 최상단 추천 강의 진도를 확인하시고, 차근차근 진행해보세요!
- 멀티플레이 기능은 가능하면 서버에서 실제 값을 변경하고,
클라이언트는 복제된 결과를 표시하도록 구현합니다.
</aside>

<aside>
💡

- 프로젝트 생성 방법
    
    !**언리얼 5.5.4 버전 기준 새 프로젝트 생성.
    Games > Vehicle 샘플 프로젝트, C++, Desktop, Scalable, Starter Content 체크
    프로젝트 이름은 CX**
    
    **언리얼 5.5.4 버전 기준 새 프로젝트 생성.
    Games > Vehicle 샘플 프로젝트, C++, Desktop, Scalable, Starter Content 체크
    프로젝트 이름은 CX**
    
    !**리슨 서버 기준 실습 환경.
    다만, 리슨 서버 기준 실습 시에 에디터 창이 호스트의 역할을 하게 되는데,
    만약 에디터 창의 포커싱이 풀릴 경우 서버의 틱 레이트가 심각하게 떨어질 수 있음.
    그럼 게스트의 경우 지터링이 매우 심해짐.
    Editor Preferences > Use Less CPU when in Background를 체크해제 하면 조금 나아짐.**
    
    **리슨 서버 기준 실습 환경.
    다만, 리슨 서버 기준 실습 시에 에디터 창이 호스트의 역할을 하게 되는데,
    만약 에디터 창의 포커싱이 풀릴 경우 서버의 틱 레이트가 심각하게 떨어질 수 있음.
    그럼 게스트의 경우 지터링이 매우 심해짐.
    Editor Preferences > Use Less CPU when in Background를 체크해제 하면 조금 나아짐.**
    
    !**데디 서버 기준 실습 환경.**
    
    **데디 서버 기준 실습 환경.**
    
    !**기존에 배치되어 있던 PlayerStart를 복사 붙혀넣기 2번해서 총 3개의 PlayerStart 배치.**
    
    **기존에 배치되어 있던 PlayerStart를 복사 붙혀넣기 2번해서 총 3개의 PlayerStart 배치.**
    
    !image.png
    
    !image.png
    
    ```cpp
    // CXPlayerController.h
    
    ...
    class CX_API ACXPlayerController : public APlayerController
    {
    	...
    
    protected:
    	...
    
    	/** Pointer to the controlled vehicle pawn */
    	//TObjectPtr<ACXPawn> VehiclePawn;
    	UPROPERTY(Replicated)
    	TObjectPtr<ACXPawn> VehiclePawn;
    
    	...
    
    protected:
    	...
    	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
    	
    public:
    	...
    	
    };
    
    ```
    
    ```cpp
    // CXPlayerController.cpp
    
    ...
    #include "Net/UnrealNetwork.h"
    
    void ACXPlayerController::BeginPlay()
    {
    	Super::BeginPlay();
    	
    	if (IsLocalController() == false)
    	{
    		return;
    	}
    
    	...
    }
    
    ...
    
    void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
    {
    	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    	
    	DOREPLIFETIME(ThisClass, VehiclePawn);
    }
    
    ...
    
    ```
    
    ```cpp
    // CXGameMode.h
    
    ...
    class ACXGameMode : public AGameModeBase
    {
    	...
    
    public:
    	...
    
    protected:
    	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    
    private:
    	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;
    
    private:
    	UPROPERTY(EditDefaultsOnly)
    	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);
    
    	UPROPERTY(EditDefaultsOnly)
    	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);
    
    };
    
    ```
    
    ```cpp
    // CXGameMode.cpp
    
    ...
    #include "EngineUtils.h"
    #include "Engine/World.h"
    #include "GameFramework/PlayerStart.h"
    
    ...
    
    AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
    {
    	TArray<APlayerStart*> AvailablePlayerStarts;
    
    	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    	{
    		APlayerStart* PlayerStart = *It;
    
    		if (IsPlayerStartAvailable(PlayerStart))
    		{
    			AvailablePlayerStarts.Add(PlayerStart);
    		}
    	}
    
    	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));
    
    	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
    	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
    }
    
    bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
    {
    	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
    	{
    		return false;
    	}
    
    	const FTransform PlayerStartTransform =	PlayerStart->GetActorTransform();
    	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
    	const FQuat CheckRotation =	PlayerStartTransform.GetRotation();
    	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
    	FCollisionObjectQueryParams ObjectQueryParams;
    	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);
    
    	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);
    
    	const bool bVehicleExists =
    		GetWorld()->OverlapAnyTestByObjectType(
    			CheckLocation,
    			CheckRotation,
    			ObjectQueryParams,
    			CheckShape,
    			QueryParams
    		);
    
    	return !bVehicleExists;
    }
    
    ```
    
- 어려우신 분들을 위해 멀티 구현을 위한 기본 프로젝트를 준비했습니다.
아래 .zip 파일을 가져가서 과제를 도전해보세요.
    
    CX.zip
    
</aside>

<aside>

### 필수 구현 기능

---

1. 채팅 시스템
    
    간단한 채팅을 주고 받을 수 있도록 기능을 구현해주세요.
    
    - [ ]  채팅창 위젯 클래스 생성
    - [ ]  채팅창 위젯 블루프린트 클래스 애셋 생성
    - [ ]  위젯 객체 생성 및 연결
    - [ ]  채팅창 엔터 이벤트 관련 로직 작성
    - [ ]  채팅창에 작성한 메세지를 화면에 띄우기
    - [ ]  채팅 동기화

---

1. PlayerState 클래스 관련 기능
    - 각 플레이어가 획득한 원화(코인)와 추가 최대 속도를 
    플레이어 스테이트 클래스에 저장하고, `자신의 화면`에서 확인할 수 있게 해야함.
    - [ ]  습득한 원화 속성(`Coins`) 추가 및 프로퍼티 레플리케이션 로직 작성
    - [ ]  원화를 증가시키는 함수와 차감하는 함수 정의
    - [ ]  원화 속성의 값은 서버에서만 변경되도록 검사
    - [ ]  원화 속성을 DOREPLIFETIME_CONDITION(COND_OwnerOnly)로 복제되게 작성.
    - [ ]  추가 최대 속도 속성(`BonusMaxSpeed`) 추가 및 프로퍼티 레플리케이션 로직 작성
    - [ ]  추가 최대 속도 속성을 증가시키는 함수 정의
    - [ ]  추가 최대 속도 속성의 값은 서버에서만 변경되도록 검사
    - [ ]  해당 속성을 DOREPLIFETIME_CONDITION(COND_OwnerOnly)로 복제되게 작성.
    - [ ]  습득한 원화와 추가 최대 속도를 표시 해주는 UI 제작 및 적절히 연결

---

1. Vehicle 클래스 관련 기능
    - 다른 차량을 공격해서 원화를 빼앗을 수 있게 하고,
    맵에 배치된 아이템을 모든 플레이어가 동일하게 확인하고 획득할 수 있게 구현해야함.
    - [ ]  공격을 위한 좌클릭 InputAction 애셋 생성 및 차량 폰 연결
    - [ ]  좌클릭 시 차량 폰 클래스의 ServerRPC 함수 호출
    - [ ]  위 ServerRPC에서 차량 폰 앞 범퍼 중앙에서 라인트레이싱으로 충돌 감지
    - [ ]  위 라인트레이싱으로 상대 차량 감지 및 상대 차량 ApplyDamage() 함수 호출
    - [ ]  피해 차량 폰에게 원화(`Coins`)가 있다면 가해 차량은 5원 획득, 피해 차량은 5원 회수
    - [ ]  피해 차량이 5원 미만 갖고 있으면 아무런 동작도 안함.
    - [ ]  DrawDebugLine()을 이용하여 라인트레이싱 표시. 닿은 차량에는 파티클 재생.
    - [ ]  코인 액터 클래스 생성 및 오버랩 시에 대상 플레이어는 원화(`Coins`) 10원 획득
    - [ ]  코인 액터 클래스의 NetUpdateFrequency는 1로 설정.
    - [ ]  오버랩된 코인 액터는 파괴.
    - [ ]  코인 아이템 액터 파괴 시 파티클 및 사운드 재생.
    - [ ]  스피드 버프 액터 클래스 생성 및 오버랩 시에 대상 플레이어는 추가 최대 속도 10 상승
    - [ ]  스피드 버프 액터 클래스의 NetUpdateFrequency는 1로 설정.
    - [ ]  오버랩된 버프 아이템 액터는 파괴.
    - [ ]  버프 아이템 액터 파괴 시 파티클 및 사운드 재생.
    - [ ]  스포너 액터 클래스 생성 및 액터 스폰 로직 작성.
    - [ ]  스포너는 생성한 아이템 객체를 속성으로 저장. OnDestroyed 델리게이트 구독.
    - [ ]  스포너가 생성한 아이템 객체가 파괴되면 2초 뒤 같은 종류의 아이템을 리스폰.
    - [ ]  코인을 스폰하는 스포너를 레벨에 적절히 배치. 스포너 별로 코인과 버프 액터 설정.

---

1. GameMode 클래스 관련 기능
    - 게임을 대기, 진행, 종료의 세 단계로 나누고 
    모든 플레이어가 같은 상태와 같은 남은 시간을 확인할 수 있게 구현해야함.
    - [ ]  GameState 클래스 생성
    - [ ]  Waiting, Playing, Finished를 표현하는 게임 상태 Enum 정의
    - [ ]  GameState 클래스에 현재 게임 상태를 표현하는 속성과 남은 시간 속성 정의
    - [ ]  GameState 클래스에 우승자 목록 속성을 정의
    - [ ]  게임 시작 시 게임 상태 속성이 Waiting으로 설정되게끔 로직 작성
    - [ ]  첫 플레이어가 접속 시 대기 카운트다운 시작하게끔 로직 작성
    - [ ]  서버 타이머가 1초마다 남은 시간 속성을 감소시키고 이를 UI에서 확인할 수 있게 작성.
    - [ ]  대기 카운트다운이 끝나면 게임 상태 속성이 Playing으로 설정되게끔 로직 작성
    - [ ]  서버 타이머가 1초마다 남은 시간 속성을 감소시키고 이를 UI에서 확인할 수 있게 작성.
    - [ ]  플레이 카운트 다운이 끝나면 게임 상태 속성이 Finished로 설정되게끔 로직 작성
    - [ ]  모든 플레이어의 원화를 비교해 가장 많은 원화를 가진 플레이어 탐색
    - [ ]  최고 원화가 같으면, 우승자 목록 속성에 모두 넣어서 공동 우승자로 처리
    - [ ]  로컬 플레이어의 승리 또는 패배 결과가 UI로 나오게끔 구현
</aside>

## 4️⃣ 도전 기능 가이드

<aside>
💡

- 한 단계 더, 점프업을 위해서 도전기능을 구현해보세요!
- 반드시 필수 기능을 완성한 후 도전 기능을 시작해주세요.
- 아래 항목을 개인 노션으로 복사해서, 
각 항목을 완료할 때마다 체크리스트를 표시하며 진행해보세요.
</aside>

<aside>

### 도전 구현 기능

---

1. 부위 공격
    - 차량의 특정 부위를 공격했을 때 별도 효과를 적용하고,
    네트워크 오너십과 RPC를 활용하여 지뢰 액터를 구현해야함.
    - [ ]  좌클릭 시 ServerRPC 함수에서 ApplyDamage() 대신 ApplyPointDamage() 호출
    - [ ]  맞은 Bone이 바퀴인지 판별
    - [ ]  바퀴를 맞히면 피해 차량의 추가 최대 속도(`BonusMaxSpeed`)에 음수 패널티 적용
    - [ ]  음수 패널티는 시간에 따라 0까지 회복. 회복값이 0을 넘어 양수가 되지 않게 제한.
    - [ ]  지뢰 매설을 위한 F키 InputAction 애셋 생성 및 차량 폰 연결
    - [ ]  F키 입력 시 차량 폰 클래스의 ServerRPC 함수 호출
    - [ ]  위 ServerRPC에서 차량 뒤쪽에 지뢰를 스폰
    - [ ]  생성한 지뢰의 Owner 속성을 설치한 차량으로 지정
    - [ ]  지뢰와 오버랩된 차량은 전진 가속 입력이 즉시 0으로 설정되게 구현(가속 페달 고장)
    - [ ]  2초 뒤 가속 페달 고장 상태 회복(해제)
    - [ ]  지뢰와 오버랩되면 폭발 파티클과 사운드 재생 및 지뢰 액터 파괴

---

1. 게임 상태별 제약 사항
    - 게임이 실제로 진행 중일 때만 차량을 운전하고 공격할 수 있게 구현해야함.
    - [ ]  `Playing` 상태에서만 가속과 조향 입력을 허용한다.
    - [ ]  `Waiting`과 `Finished`에서는 스로틀과 조향 입력을 0으로 유지한다.
    - [ ]  `Waiting`과 `Finished`에서는 핸드브레이크를 true로 유지한다
    - [ ]  `Waiting`과 `Finished`에서는 차량 위치 초기화 기능도 사용할 수 없게 한다.
    - [ ]  `Playing`으로 전환되면 게임 상태용 핸드브레이크 잠금을 자동으로 해제한다.
    - [ ]  `Playing` 상태에서만 좌클릭 공격 입력을 허용한다.
    - [ ]  `Playing` 상태에서만 F키 지뢰 설치 입력을 허용한다.
</aside>