#pragma once

#include "Engine_Define.h"
#include "Engine_Enum.h"

#include <string>
#include <vector>

namespace Engine
{
    inline constexpr uint32 GLOBAL_LEVEL = 0;

    struct TOUCH_STATE
    {
        bool isPressed = false;
        bool isDown = false;
        bool isUp = false;
        float fX = 0.f;
        float fY = 0.f;
        float fDeltaX = 0.f;
        float fDeltaY = 0.f;
        float fStartX = 0.f;
        float fStartY = 0.f;
    };

    struct GAMEOBJECT_DESC
    {
        std::wstring strGameObjectName;
        int32 iEventIndex = -1;
    };

    struct TRANSFORM_DESC
    {
        float vPosition[3]{};
        float vRotationRadian[3]{};
        float vScale[3]{1.f,1.f,1.f};
        float fSpeedPerSecond = 5.f;
        float fRotationPerSecond = 1.5f;
    };

    struct CAMERA_OBJECT_DESC : public GAMEOBJECT_DESC
    {
        CAMERA_UPDATE_DEPTH eCameraUpdateDepth = CAMERA_UPDATE_DEPTH::DEPTH_0;
        CAMERA_RENDER_DEPTH eCameraRenderDepth = CAMERA_RENDER_DEPTH::DEPTH_0;
        PROJECTION_TYPE eProjectionType = PROJECTION_TYPE::PERSPECTIVE;
        float fScreenStartX = 0.f;
        float fScreenStartY = 0.f;
        float fFilmingSizeX = 1280.f;
        float fFilmingSizeY = 720.f;
        float fFovDegree = 60.f;
        float fNear = 0.1f;
        float fFar = 1000.f;
        bool isUpdateActive = true;
        bool isRenderActive = true;
        TRANSFORM_DESC TransformDesc{};
    };

    struct DEFAULT_CAMERA_DESC : public CAMERA_OBJECT_DESC
    {
        MOVE_TYPE eMoveType = MOVE_TYPE::NORMAL;
        CAMERA_TYPE eCameraType = CAMERA_TYPE::FIRSTPERSON;
        UPDATE_MODE eUpdateMode = UPDATE_MODE::DYNAMIC;
    };

    struct CREATE_SCENE_DESC
    {
        SCENE_ISPOP eIsPop = SCENE_ISPOP::NO;
        uint32 iMakeSceneID = 0;
        bool isNeedSceneOverlay = false;
        uint32 iNextSceneID = 0;
        std::wstring strNextSceneDataPath;
    };

    struct SCENE_INFO
    {
        uint32 iSceneID = 0;
        SCENE_STACK_TYPE eStackType = SCENE_STACK_TYPE::REPLACE;
        bool isRenderWithPreviousScene = false;
    };

    struct LIGHT_DESC
    {
        LIGHT_TYPE eType = LIGHT_TYPE::DIRECTIONAL;
        float vPosition[4]{0.f,0.f,0.f,1.f};
        float vDirection[4]{0.f,-1.f,0.f,0.f};
        float vColor[4]{1.f,1.f,1.f,1.f};
        float fRange = 10.f;
        bool isActive = true;
    };

    struct CELL_DESC
    {
        float vPosition[3][3]{};
        uint32 iIndex = 0;
    };

    struct COLLIDER_DESC
    {
        COLLIDER_TYPE eType = COLLIDER_TYPE::SPHERE;
        float vCenter[3]{};
        float vExtents[3]{0.5f, 0.5f, 0.5f};
        float fRadius = 0.5f;
        bool isActive = true;
    };

    // Diligent?먯꽌??DX11??媛쒕퀎 State 媛앹껜 ???PSO ?앹꽦 ?뺣낫濡??곹깭瑜?臾띕뒗??
    // 湲곗〈 RenderStateManager ?ъ슜 媛먭컖???좎??섍린 ?꾪븳 ?붿쭊 ?낅┰ ?꾨━?뗭씠??
    struct RENDER_STATE_DESC
    {
        SHADER_CULL_MODE eCullMode = SHADER_CULL_MODE::BACK;
        bool isDepthEnable = true;
        bool isDepthWriteEnable = true;
        bool isBlendEnable = false;
    };

    struct FONT_DESC
    {
        std::wstring strFontPath;
        float fOriginFontSize = 16.f;
    };

    struct VTXPOSTEX
    {
        float vPosition[3]{};
        float vTexcoord[2]{};
    };

    struct SHADER_INPUT_ELEMENT_DESC
    {
        uint32 iInputIndex = 0;
        uint32 iBufferSlot = 0;
        uint32 iComponentCount = 0;
        SHADER_VALUE_TYPE eValueType = SHADER_VALUE_TYPE::FLOAT32;
        bool isNormalized = false;
    };

    struct SHADER_PASS_DESC
    {
        std::string strPassKey = "Default";
        const char* pVertexShaderSource = nullptr;
        const char* pPixelShaderSource = nullptr;
        const SHADER_INPUT_ELEMENT_DESC* pInputElements = nullptr;
        uint32 iInputElementCount = 0;
        SHADER_PRIMITIVE_TOPOLOGY eTopology = SHADER_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST;
        SHADER_CULL_MODE eCullMode = SHADER_CULL_MODE::BACK;
        bool isDepthEnable = true;
        bool isBlendEnable = false;
        bool isPointSampler = false;
        SHADER_TEXTURE_FORMAT eRTVFormat = SHADER_TEXTURE_FORMAT::RGBA8_UNORM_SRGB;
        SHADER_TEXTURE_FORMAT eDSVFormat = SHADER_TEXTURE_FORMAT::D32_FLOAT;
    };

    struct KEY_FRAME
    {
        float fTime = 0.f;
        float vScale[3]{1.f, 1.f, 1.f};
        float vRotation[4]{0.f, 0.f, 0.f, 1.f};
        float vTranslation[3]{};
    };

    struct BONE_INFO
    {
        std::string strName;
        int32 iParentIndex = -1;
        float matTransform[16]{
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f};
    };

    struct CHANNEL_INFO
    {
        uint32 iConnectedBoneIndex = 0;
        std::vector<KEY_FRAME> vecKeyFrame;
    };

    struct ANIMATION_INFO
    {
        std::string strName;
        float fDuration = 0.f;
        float fTicksPerSecond = 1.f;
        std::vector<CHANNEL_INFO> vecChannel;
    };

    struct PrototypeManager_Desc
    {
        // Index 0? ??긽 Global ??μ냼濡??ъ슜?쒕떎.
        uint32 iTotalLevelCount = 8;
    };

    struct Engine_Desc
    {
        GRAPHIC_API eGraphicAPI = GRAPHIC_API::VULKAN;
        void* pNativeWindow = nullptr;
        unsigned int iWinSizeX = 1280;
        unsigned int iWinSizeY = 720;
        PrototypeManager_Desc tPrototypeManagerDesc{};
    };
}

