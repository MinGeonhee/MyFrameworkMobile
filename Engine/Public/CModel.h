#pragma once

#include "CResource.h"

class CGameObject;
class CMesh;
class CShader;
class CBone;
class CAnimation;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CModel final : public CResource
{
private:
    explicit CModel(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);
    explicit CModel(const CModel& Prototype);
    ~CModel() override = default;
    HRESULT Initialize(const std::wstring& strModelName, MODEL_TYPE eModelType);
    HRESULT Copy_Initialize(void* pArg) override;

public:
    HRESULT Add_Mesh(CMesh* pMesh);
    HRESULT Add_Bone(CBone* pBone);
    HRESULT Add_Animation(CAnimation* pAnimation);
    CMesh* Find_Mesh(const std::string& strName) const;
    CMesh* Get_Mesh(uint32 iIndex) const;
    uint32 Get_MeshCount() const { return static_cast<uint32>(m_vecMesh.size()); }
    const std::wstring& Get_ModelName() const { return m_strModelName; }
    MODEL_TYPE Get_ModelType() const { return m_eModelType; }
    void Set_Owner(CGameObject* pOwner) { m_pOwner = pOwner; }
    CGameObject* Get_Owner() const { return m_pOwner; }
    void Set_MeshActive(const std::string& strMeshName, bool isActive);
    void Set_MeshActive(uint32 iMeshIndex, bool isActive);
    HRESULT Draw(CShader* pShader, const std::string& strPassKey = "Default");
    bool Play_Animation(float fSpeed, float fTimeDelta);
    void Change_Animation(uint32 iAnimationIndex, ANIMATION_PLAYTYPE ePlayType);
    uint32 Get_AnimationCount() const { return static_cast<uint32>(m_vecAnimation.size()); }

    static CModel* Create(const std::wstring& strModelName,
                          MODEL_TYPE eModelType,
                          Diligent::IRenderDevice* pDevice,
                          Diligent::IDeviceContext* pDeviceContext);
    CResource* Copy_Resource(void* pArg = nullptr) override;
    void Free() override;

private:
    CGameObject* m_pOwner = nullptr;
    std::wstring m_strModelName;
    MODEL_TYPE m_eModelType = MODEL_TYPE::NONANIM;
    std::vector<CMesh*> m_vecMesh;
    std::unordered_map<std::string, CMesh*> m_mapMesh;
    std::vector<CBone*> m_vecBone;
    std::vector<CAnimation*> m_vecAnimation;
    uint32 m_iCurrentAnimationIndex = 0;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
