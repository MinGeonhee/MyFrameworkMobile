#pragma once

// 원본 MyFramework는 Windows SDK 타입/함수(HRESULT, HWND, MessageBox, VK_ 등)를 엔진 전반에서 그대로 사용한다.
// Windows에서는 SDK를 그대로 쓰고, Android(NDK clang)에서는 같은 이름을 최소한으로 정의해
// 원본 코드를 수정 없이 컴파일할 수 있게 한다.

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cwchar>
#include <cstdarg>

#if defined(_WIN32)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <Windows.h>
#    define ENGINE_PLATFORM_WINDOWS 1
#else
#    define ENGINE_PLATFORM_ANDROID 1

// MSVC 전용 키워드
#    ifndef abstract
#        define abstract
#    endif
#    ifndef __forceinline
#        define __forceinline inline __attribute__((always_inline))
#    endif

#    ifndef MAX_PATH
#        define MAX_PATH 260
#    endif

#    ifndef CONST
#        define CONST const
#    endif
#    ifndef PURE
#        define PURE = 0
#    endif
#    ifndef TRUE
#        define TRUE 1
#    endif
#    ifndef FALSE
#        define FALSE 0
#    endif
#    ifndef _countof
#        define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#    endif

using HRESULT   = std::int32_t;
using BOOL      = int;
using BYTE      = unsigned char;
using WORD      = unsigned short;
using DWORD     = std::uint32_t;
using UINT      = unsigned int;
using UINT64    = std::uint64_t;
using LONG      = std::int32_t;
using WCHAR     = wchar_t;
using TCHAR     = wchar_t;
using LPVOID    = void*;
using HANDLE    = void*;
using HWND      = void*;   // Android에서는 ANativeWindow* 를 담는다.
using HINSTANCE = void*;
using D3DCOLOR  = std::uint32_t;

struct POINT
{
    LONG x = 0;
    LONG y = 0;
};

struct RECT
{
    LONG left = 0;
    LONG top = 0;
    LONG right = 0;
    LONG bottom = 0;
};

#    ifndef UNREFERENCED_PARAMETER
#        define UNREFERENCED_PARAMETER(P) (void)(P)
#    endif

#    ifndef TEXT
#        define TEXT(quote) L##quote
#    endif

// BMP 헤더 (높이맵 로딩)
#    pragma pack(push, 2)
struct BITMAPFILEHEADER
{
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
};
#    pragma pack(pop)

struct BITMAPINFOHEADER
{
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};

// SAL 주석 (Windows SDK 전용)
#    ifndef _In_
#        define _In_
#    endif
#    ifndef _Out_
#        define _Out_
#    endif
#    ifndef _Outptr_
#        define _Outptr_
#    endif
#    ifndef _Inout_
#        define _Inout_
#    endif
#    ifndef _In_opt_
#        define _In_opt_
#    endif
#    ifndef _Out_opt_
#        define _Out_opt_
#    endif

inline constexpr HRESULT S_OK         = 0;
inline constexpr HRESULT S_FALSE      = 1;
inline constexpr HRESULT E_FAIL       = static_cast<HRESULT>(0x80004005u);
inline constexpr HRESULT E_INVALIDARG = static_cast<HRESULT>(0x80070057u);
inline constexpr HRESULT E_NOTIMPL    = static_cast<HRESULT>(0x80004001u);
inline constexpr HRESULT E_OUTOFMEMORY = static_cast<HRESULT>(0x8007000Eu);

inline constexpr bool FAILED(HRESULT hr) noexcept { return hr < 0; }
inline constexpr bool SUCCEEDED(HRESULT hr) noexcept { return hr >= 0; }

#    define ZeroMemory(Destination, Length) std::memset((Destination), 0, (Length))

// MessageBox 계열: Android에는 모달 창이 없으므로 logcat으로 출력한다.
#    define MB_OK 0
int  Engine_Android_MessageBoxW(void* hWnd, const wchar_t* pText, const wchar_t* pCaption, unsigned int uType);
void Engine_Android_OutputDebugStringW(const wchar_t* pText);
void Engine_Android_OutputDebugStringA(const char* pText);
#    define MessageBox  Engine_Android_MessageBoxW
#    define MessageBoxW Engine_Android_MessageBoxW
#    define OutputDebugString  Engine_Android_OutputDebugStringW
#    define OutputDebugStringW Engine_Android_OutputDebugStringW
#    define OutputDebugStringA Engine_Android_OutputDebugStringA

// MSVC 보안 CRT 함수 대체
template <size_t N>
inline int swprintf_s(wchar_t (&Buffer)[N], const wchar_t* pFormat, ...)
{
    va_list Args;
    va_start(Args, pFormat);
    const int Result = std::vswprintf(Buffer, N, pFormat, Args);
    va_end(Args);
    return Result;
}
inline int swprintf_s(wchar_t* pBuffer, size_t iCount, const wchar_t* pFormat, ...)
{
    va_list Args;
    va_start(Args, pFormat);
    const int Result = std::vswprintf(pBuffer, iCount, pFormat, Args);
    va_end(Args);
    return Result;
}
template <size_t N>
inline int sprintf_s(char (&Buffer)[N], const char* pFormat, ...)
{
    va_list Args;
    va_start(Args, pFormat);
    const int Result = std::vsnprintf(Buffer, N, pFormat, Args);
    va_end(Args);
    return Result;
}
inline int sprintf_s(char* pBuffer, size_t iCount, const char* pFormat, ...)
{
    va_list Args;
    va_start(Args, pFormat);
    const int Result = std::vsnprintf(pBuffer, iCount, pFormat, Args);
    va_end(Args);
    return Result;
}
template <size_t N>
inline int strcpy_s(char (&Dest)[N], const char* pSrc)
{
    std::strncpy(Dest, pSrc, N - 1);
    Dest[N - 1] = '\0';
    return 0;
}
inline int strcpy_s(char* pDest, size_t iCount, const char* pSrc)
{
    std::strncpy(pDest, pSrc, iCount - 1);
    pDest[iCount - 1] = '\0';
    return 0;
}
template <size_t N>
inline int wcscpy_s(wchar_t (&Dest)[N], const wchar_t* pSrc)
{
    std::wcsncpy(Dest, pSrc, N - 1);
    Dest[N - 1] = L'\0';
    return 0;
}
inline int wcscpy_s(wchar_t* pDest, size_t iCount, const wchar_t* pSrc)
{
    std::wcsncpy(pDest, pSrc, iCount - 1);
    pDest[iCount - 1] = L'\0';
    return 0;
}

// Windows 가상 키코드. Android 입력은 InputManager에서 이 값으로 매핑한다.
#    define VK_LBUTTON  0x01
#    define VK_RBUTTON  0x02
#    define VK_MBUTTON  0x04
#    define VK_BACK     0x08
#    define VK_TAB      0x09
#    define VK_RETURN   0x0D
#    define VK_SHIFT    0x10
#    define VK_CONTROL  0x11
#    define VK_MENU     0x12
#    define VK_ESCAPE   0x1B
#    define VK_SPACE    0x20
#    define VK_PRIOR    0x21
#    define VK_NEXT     0x22
#    define VK_END      0x23
#    define VK_HOME     0x24
#    define VK_LEFT     0x25
#    define VK_UP       0x26
#    define VK_RIGHT    0x27
#    define VK_DOWN     0x28
#    define VK_INSERT   0x2D
#    define VK_DELETE   0x2E
#    define VK_F1       0x70
#    define VK_F2       0x71
#    define VK_F3       0x72
#    define VK_F4       0x73
#    define VK_F5       0x74
#    define VK_F6       0x75
#    define VK_F7       0x76
#    define VK_F8       0x77
#    define VK_F9       0x78
#    define VK_F10      0x79
#    define VK_LSHIFT   0xA0
#    define VK_RSHIFT   0xA1
#    define VK_LCONTROL 0xA2
#    define VK_RCONTROL 0xA3
#    define VK_LMENU    0xA4
#    define VK_RMENU    0xA5
#endif

// 공통 매크로 (Windows 는 WIN32_LEAN_AND_MEAN 으로 일부가 빠진다)
#ifndef PURE
#    define PURE = 0
#endif
#ifndef CONST
#    define CONST const
#endif

// 그래픽 API 선택값. 실제 초기화는 CGraphicDevice가 담당한다.
enum class GRAPHIC_API
{
    D3D11,
    VULKAN
};
