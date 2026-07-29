#pragma once

#include <string>
#include <vector>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include "Engine_Macro.h"
#include "Engine_Platform.h"
#include "Engine_Define.h"
#include "Engine_Struct.h"

namespace Engine
{
    // Android Client??NativeWindow? VULKAN ?좏깮媛믩쭔 ?꾨떖?쒕떎.
    // Device, Context, SwapChain ?앹꽦怨?Present??Engine???대떦?쒕떎.
#if defined(__ANDROID__)
    ENGINE_DLL void Set_Android_AssetManager(AAssetManager* pAssetManager);
    ENGINE_DLL AAssetManager* Get_Android_AssetManager();
#endif

    ENGINE_DLL HRESULT Initialize_Android_Graphic(const Engine_Desc& Desc);
    ENGINE_DLL void    Draw_Android_Graphic();
    ENGINE_DLL void    Release_Android_Graphic();
}

