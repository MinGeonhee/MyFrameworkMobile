#pragma once
#include "CBase.h"
#include "Engine_Struct.h"

class ENGINE_DLL CRenderStateManager final : public CBase
{
private: CRenderStateManager() = default;
public:
    ~CRenderStateManager() override = default;
    HRESULT Add_RenderState(uint32 iIndex, const Engine::RENDER_STATE_DESC& Desc);
    const Engine::RENDER_STATE_DESC* Get_RenderState(uint32 iIndex) const;
    HRESULT Ready_Default();
    static CRenderStateManager* Create();
    void Free() override;
private: std::unordered_map<uint32, Engine::RENDER_STATE_DESC> m_RenderStates;
};
