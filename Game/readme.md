# Game

Windows Client와 Android 실행부가 함께 사용하는 공통 게임 로직 프로젝트입니다.

## 역할

- Scene 구성
- GameObject / PartObject / ContainerObject 작성
- Terrain, SkyBox, 테스트 오브젝트 작성
- 플레이어, 몬스터, 애니메이션 등 콘텐츠 로직 작성
- Windows / Android 양쪽에서 공유할 게임 코드 관리

## 작성 위치

일반적인 게임 콘텐츠 작업은 우선 이 프로젝트에서 진행합니다.

```text
C:\MyFrameworkMobile\Game
```

Client와 Android는 실행부 역할만 담당하고, Game 프로젝트를 공유하는 구조를 목표로 합니다.

## 작업 원칙

- 플랫폼별 실행 코드를 Game에 넣지 않습니다.
- Windows API, Android NativeActivity 같은 코드는 Client / Android에 둡니다.
- Engine 기능 수정이 필요한 경우에만 Engine으로 이동합니다.
- Game은 가능한 한 플랫폼 독립적인 C++ 코드로 유지합니다.
