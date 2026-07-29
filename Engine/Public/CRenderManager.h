#pragma once

#include "CBase.h"

class CGameObject;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CRenderManager final : public CBase
{
private:
    CRenderManager() = default;

public:
    ~CRenderManager() override = default;
    HRESULT Ready_Default();
    HRESULT Add_RenderGroup(RENDER_GROUP eRenderGroup, CGameObject* pGameObject);
    HRESULT RenderGroup();
    HRESULT Clean_RenderGroup();
    size_t Get_RenderGroupSize(RENDER_GROUP eRenderGroup) const;

    static CRenderManager* Create();
    void Free() override;

private:
    std::array<std::vector<CGameObject*>, static_cast<size_t>(RENDER_GROUP::END)> m_RenderGroups;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
