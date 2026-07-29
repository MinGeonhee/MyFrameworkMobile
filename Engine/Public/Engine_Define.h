#pragma once

#include <cstdint>

#include "Engine_Platform.h"

// MSVC ?꾩슜 ???????쒖? 怨좎젙 ???뺤닔瑜??ъ슜??Android?먯꽌??媛숈? ?ш린瑜?蹂댁옣?쒕떎.
using int64 = std::int64_t;
using int32 = std::int32_t;
using int16 = std::int16_t;
using wchar = wchar_t;
using uint32 = std::uint32_t;
using uint16 = std::uint16_t;

namespace Diligent { struct IRenderDevice; struct IDeviceContext; }
using Device = Diligent::IRenderDevice;
using Context = Diligent::IDeviceContext;

