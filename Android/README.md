# Android

MyFrameworkMobile의 Android 실행 프로젝트입니다.

Android Studio에서 이 폴더를 열어 APK 빌드와 실행 테스트를 진행합니다.

```text
C:\MyFrameworkMobile\Android
```

## 역할

- Gradle / NDK / CMake 기반 Android 빌드
- NativeActivity 진입점 구성
- ANativeWindow를 Engine에 전달
- Android 앱 생명주기와 입력 이벤트 연결
- Vulkan 기반 Diligent 렌더링 초기화 경로 사용
- APK 생성 및 실기기 테스트

## 코드 작성 기준

Android 폴더에는 Android 실행에 필요한 코드만 둡니다.

- Android 전용 진입점
- NativeActivity 연결
- 터치/입력 이벤트 전달
- APK 빌드 설정
- Android assets 리소스 포함

게임 로직은 Android 폴더에 중복 작성하지 않고 `Game` 프로젝트에서 관리합니다.

## 빌드

PowerShell 기준:

```powershell
cd C:\MyFrameworkMobile\Android
.\gradlew.bat :app:assembleDebug
```

Debug APK 결과물:

```text
C:\MyFrameworkMobile\Android\app\build\outputs\apk\debug\app-debug.apk
```

Release APK 결과물:

```text
C:\MyFrameworkMobile\Android\app\build\outputs\apk\release\app-release-unsigned.apk
```

## Android Studio에서 실행

1. Android Studio 실행
2. `C:\MyFrameworkMobile\Android` 폴더 열기
3. Gradle Sync 진행
4. 에뮬레이터 또는 실기기 선택
5. Run 실행

## ABI

에뮬레이터와 실기기 테스트를 위해 다음 ABI를 고려합니다.

```text
x86_64      Android Emulator
arm64-v8a   일반 Android 실기기
```

실기기에서 APK 설치가 실패한다면 ABI 포함 여부를 먼저 확인합니다.

## 리소스

현재 테스트 리소스는 APK assets에 포함하는 방식을 사용합니다. 리소스 규모가 커질 경우 다운로드 방식이나 별도 패키징 방식으로 확장할 수 있습니다.
