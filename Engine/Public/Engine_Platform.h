#pragma once

#include <cstdint>

#if defined(_WIN32)
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>
#else
// Android/clang에서는 MSVC 전용 abstract 키워드가 없으므로 기존 클래스 선언 스타일을 유지하기 위해 비워둔다.
#    if !defined(abstract)
#        define abstract
#    endif

#    if !defined(MAX_PATH)
#        define MAX_PATH 260
#    endif

// Windows SDK가 없는 플랫폼에서도 기존 엔진의 HRESULT 반환 규칙을 유지한다.
using HRESULT = std::int32_t;

inline constexpr HRESULT S_OK         = 0;
inline constexpr HRESULT S_FALSE      = 1;
inline constexpr HRESULT E_FAIL       = static_cast<HRESULT>(0x80004005u);
inline constexpr HRESULT E_INVALIDARG = static_cast<HRESULT>(0x80070057u);
inline constexpr HRESULT E_NOTIMPL    = static_cast<HRESULT>(0x80004001u);

inline constexpr bool FAILED(HRESULT hr) noexcept { return hr < 0; }
inline constexpr bool SUCCEEDED(HRESULT hr) noexcept { return hr >= 0; }
#endif



