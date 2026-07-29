#pragma once

#include "CBase.h"

class CMaterial;
class CShader;
class CVIBuffer_Mesh;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CMesh final : public CBase
{
private:
    CMesh() = default;
    explicit CMesh(const CMesh& Prototype);
    ~CMesh() override = default;

public:
    HRESULT Initialize(const std::string& strName, CVIBuffer_Mesh* pBuffer, CMaterial* pMaterial);
    HRESULT Bind_Resource();
    HRESULT Draw(CShader* pShader, const std::string& strPassKey = "Default");
    void Set_Active(bool isActive) { m_isActive = isActive; }
    bool Get_IsActive() const { return m_isActive; }
    const std::string& Get_Name() const { return m_strName; }
    CMaterial* Get_Material() const { return m_pMaterial; }
    CVIBuffer_Mesh* Get_Buffer() const { return m_pBuffer; }

    static CMesh* Create(const std::string& strName, CVIBuffer_Mesh* pBuffer, CMaterial* pMaterial = nullptr);
    CMesh* Clone();
    void Free() override;

private:
    bool m_isActive = true;
    std::string m_strName;
    CVIBuffer_Mesh* m_pBuffer = nullptr;
    CMaterial* m_pMaterial = nullptr;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
