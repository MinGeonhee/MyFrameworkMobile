#pragma once
#include "CBase.h"

class ENGINE_DLL CPrototypeManager final : public CBase
{
private:
    explicit CPrototypeManager(Device* pDevice, Context* pDeviceContext);
    HRESULT Initialize(const Engine::PrototypeManager_Desc& Desc);
    CBase* Find_Prototype(PROTOTYPE_TYPE ePrototypeType,
                          uint32 iLevelID,
                          const std::wstring& strPrototypeTag) const;
public:
    HRESULT Ready_DefaultPrototype();
    HRESULT Add_Prototype(PROTOTYPE_TYPE ePrototypeType,
                          uint32 iLevelID,
                          const std::wstring& strPrototypeTag,
                          CBase* pPrototype);
    CBase* Clone_Prototype(PROTOTYPE_TYPE ePrototypeType,
                           uint32 iLevelID,
                           const std::wstring& strPrototypeTag,
                           void* pArg = nullptr) const;

    void Clear_Prototype(uint32 iLevelID);

    const std::map<std::wstring, CBase*>* Get_PrototypeList(
        PROTOTYPE_TYPE ePrototypeType,
        uint32 iLevelID) const;
private:
    uint32 m_iTotalLevelCount = 0;
    std::array<std::vector<std::map<std::wstring, CBase*>>,
               static_cast<size_t>(PROTOTYPE_TYPE::END)> m_PrototypeMaps;
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
public:
    ~CPrototypeManager() override;
    static CPrototypeManager* Create(const Engine::PrototypeManager_Desc& Desc,
                                     Device* pDevice,
                                     Context* pDeviceContext);
    void Free() override;
};
