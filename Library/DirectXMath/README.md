# DirectXMath

Windows SDK 10.0.26100 에 포함된 DirectXMath 3.19 헤더(MIT License, https://github.com/microsoft/DirectXMath)를 복사해 둔 것이다.
Windows / Android(NDK clang, arm64 NEON / x86_64 SSE) 양쪽에서 같은 수학 코드를 쓰기 위해 엔진 내부에 포함한다.

- `Inc/`  : DirectXMath, DirectXCollision, DirectXPackedVector, DirectXColors
- `Stub/sal.h` : Windows SDK가 없는 플랫폼(Android)용 SAL 매크로 빈 정의. Android 빌드에서만 include 경로에 추가한다.
