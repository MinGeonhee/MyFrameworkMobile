#include "pch.h"
#include "CRenderStateManager.h"
CRenderStateManager* CRenderStateManager::Create() { return new CRenderStateManager{}; }
HRESULT CRenderStateManager::Add_RenderState(uint32 iIndex, const Engine::RENDER_STATE_DESC& Desc)
{ return m_RenderStates.emplace(iIndex, Desc).second ? S_OK : E_FAIL; }
const Engine::RENDER_STATE_DESC* CRenderStateManager::Get_RenderState(uint32 iIndex) const
{ const auto Iter=m_RenderStates.find(iIndex); return Iter==m_RenderStates.end()?nullptr:&Iter->second; }
HRESULT CRenderStateManager::Ready_Default()
{
    Engine::RENDER_STATE_DESC Default{};
    Engine::RENDER_STATE_DESC Alpha{}; Alpha.isBlendEnable=true; Alpha.isDepthWriteEnable=false;
    if (FAILED(Add_RenderState(0,Default)) || FAILED(Add_RenderState(1,Alpha))) return E_FAIL;
    return S_OK;
}
void CRenderStateManager::Free() { m_RenderStates.clear(); CBase::Free(); }
