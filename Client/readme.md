# Client

Windows 실행 프로젝트입니다.

Client는 Windows 전용 실행부 역할만 담당합니다. 실제 게임 로직은 Game 프로젝트에 두고, 렌더링 초기화와 Present 같은 엔진 내부 처리는 Engine에서 처리합니다.

## 역할

- WinAPI 창 생성
- Windows 메시지 루프 처리
- Engine 초기화 호출
- Update / Render 루프 호출
- 창 크기 변경 정보를 Engine에 전달
- Windows에서 Game 프로젝트 실행 확인

## 작성 원칙

- Client에 게임 로직을 많이 넣지 않습니다.
- Client에서 Diligent Device, Context, SwapChain을 직접 관리하지 않습니다.
- Client에서 Present를 직접 호출하지 않습니다.
- Windows 전용 코드만 Client에 두고, 공통 콘텐츠는 Game에 작성합니다.

## 실행

Visual Studio에서 아래 솔루션을 엽니다.

```text
C:\MyFrameworkMobile\MyFrameworkMobile.sln
```

권장 구성:

```text
Configuration: Debug 또는 Release
Platform: x64
```

Client 프로젝트를 시작 프로젝트로 설정한 뒤 실행합니다.
