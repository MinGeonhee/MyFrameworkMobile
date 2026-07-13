#pragma once

#include "Engine_Enum.h"

struct Engine_Desc
{
    GRAPHIC_API eGraphicAPI = GRAPHIC_API::VULKAN;
    void* pNativeWindow = nullptr;
    unsigned int iWinSizeX = 1280;
    unsigned int iWinSizeY = 720;
};
