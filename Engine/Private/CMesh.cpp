#include "CMesh.h"
#include "CVIBuffer_Mesh.h"
#include "CTexture.h"
#include "CShader.h"
#include "CGameManager.h"
#include "CMaterial.h"
#include "CBone.h"

/* 아.. 그래서 쉐이더를 던져주는 느낌으로 한 이유가 매개변수로 Shader를 받고? 처리해줄려고 */
/* 쉐이더를 미리 Get으로받아와서 매개변수로 넣어주면 될거같은데? */

CMesh::CMesh(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_isActive(true)
    , m_strName{}, m_pMaterial{ nullptr }, m_iCountConnectedBone{0}
    , m_pBuffer(nullptr), m_pOwner(nullptr),m_vecConnectedBoneIndex{}, m_vecConnectedBoneOffsetMatrix{}
{
}

CMesh::CMesh(const CMesh& Proto)
    : m_pDevice(Proto.m_pDevice) , m_pDeviceContext(nullptr) , m_pBuffer(Proto.m_pBuffer),m_pOwner(nullptr) ,m_pMaterial(Proto.m_pMaterial)
    , m_iCountConnectedBone{Proto.m_iCountConnectedBone}, m_isActive(Proto.m_isActive)
{
    Safe_AddRef(m_pBuffer);
    /* 안에 있는 디퓨즈 ID값들을 어케해야하지...*/
    Safe_AddRef(m_pMaterial);
}


HRESULT CMesh::Initialilze_Anim(const string& strMeshName, const MeshConnectedBoneInfo& tMeshConnectedBoneInfo, const wstring& strMaterialName, const MTL_PATH& strMtlPath, const VI_INFO& tVIInfo)
{
    /* Mesh Name Setting*/
    m_strName = strMeshName;

    /* 이매쉬와 연결된 뼈의 개수.. */
    m_iCountConnectedBone = tMeshConnectedBoneInfo.iCountConnectedBone;
    m_vecConnectedBoneIndex = tMeshConnectedBoneInfo.vecConnectedBoneID;
    m_vecConnectedBoneOffsetMatrix = tMeshConnectedBoneInfo.vecConnectedOffsetMatrix;

    /* Material Setting */
    if (FAILED(Ready_Material(strMaterialName, strMtlPath)))
        return E_FAIL;


    /* Buffer Setting */
    if (FAILED(Ready_Buffer(MODEL_TYPE::ANIM, tVIInfo)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Initialilze_NonAnim(const string& strMeshName, const wstring& strMaterialName, const MTL_PATH& strMtlPath, const VI_INFO& tVIInfo, const XMatrix_1& matTransform)
{
    /* Mesh Name Setting*/
    m_strName = strMeshName;

    /* NonAnim은 뼈의개수 0개*/
    m_iCountConnectedBone = 0;

    /* Material  */
    if (FAILED(Ready_Material(strMaterialName, strMtlPath)))
        return E_FAIL;


    /* Buffer Setting */
    if (FAILED(Ready_Buffer( MODEL_TYPE::NONANIM , tVIInfo, matTransform)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Ready_Buffer(MODEL_TYPE _eModelType, VI_INFO tVIInfo , const XMatrix_1& matTransform)
{

    m_pBuffer = _eModelType == MODEL_TYPE::ANIM ? CVIBuffer_Mesh::Create_Anim(tVIInfo, m_pDevice, m_pDeviceContext) : CVIBuffer_Mesh::Create_NonAnim(tVIInfo, matTransform, m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(m_pBuffer);


    return S_OK;
}


HRESULT CMesh::Ready_Material(const wstring& _wstrMaterialName,const MTL_PATH& _MtlPath)
{
    m_pMaterial =  static_cast<CMaterial*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(_wstrMaterialName,RESOURCE_TYPE::MATERIAL,false));
    
    /* 등록된 메테리얼이 없다는뜻 */
    if (m_pMaterial == nullptr)
    {
        m_pMaterial = CMaterial::Create(_MtlPath,_wstrMaterialName,m_pDevice,m_pDeviceContext);
        if (FAILED(CGameManager::GetInstance()->ResourceManager_Add_Resource(_wstrMaterialName, m_pMaterial)))
        {
            Safe_Release(m_pMaterial);
            return E_FAIL;
        }
        Safe_AddRef(m_pMaterial);
    }

    return S_OK;
}

HRESULT CMesh::Set_GPU_Material(CShader* pShader)
{
    NULLPTR_RETURN_EFAIL(pShader);

    return m_pMaterial->Set_GPU_Material(pShader);;
}

HRESULT CMesh::Set_GPU_ConnectedBoneMatrix(CShader* pShader, const string& strName,const vector<CBone*>& vecBone)
{
    NULLPTR_RETURN_EFAIL(pShader);

    for (uint32 i = 0; i < m_iCountConnectedBone; ++i)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_vecConnectedBoneOffsetMatrix[i]) * vecBone[m_vecConnectedBoneIndex[i]]->Get_CombinedTransformationMatrix_Cal());
    }

    pShader->Set_GPU_Matrices(strName , m_BoneMatrices, g_iMaxNumBones);

    return S_OK;
}

HRESULT CMesh::Bind_Resource()
{
    return m_pBuffer->Bind_Resource();
}

HRESULT CMesh::Draw(CShader* pShader, const string& strPathKey , uint32 iIndex)
{
    NULLPTR_RETURN_EFAIL(pShader);
    if (!this->m_isActive) return S_OK;

    m_pBuffer->Bind_Resource();
    pShader->Apply_Pass(strPathKey, iIndex);
    m_pBuffer->Draw();

    return S_OK;
}


HRESULT CMesh::Set_Material_Index(MATERIAL_TYPE _eType, uint32 _iIndex)
{
    return m_pMaterial->Set_Material_Index(_eType,_iIndex);
}

void CMesh::Set_Owner(CGameObject* pOwner)
{
    m_pBuffer->Set_Owner(pOwner);

    m_pOwner = pOwner;
}



wstring CMesh::Get_MaterialName() const
{
    if (m_pMaterial == nullptr)  return L"Empty";

    return m_pMaterial->Get_Name();
}



CMesh* CMesh::Create_Anim(const string& strMeshName, const MeshConnectedBoneInfo& tMeshConnectedBoneInfo, const wstring& wstrMaterialName, const MTL_PATH& strMtlPath, VI_INFO tVIInfo, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CMesh* pMesh = new CMesh(pDevice, pContext);

    if (FAILED(pMesh->Initialilze_Anim(strMeshName, tMeshConnectedBoneInfo, wstrMaterialName, strMtlPath,tVIInfo)))
    {
        Safe_Release(pMesh);
        MSG_BOX("Mehs is failed To Create");
        return nullptr;
    }

    return pMesh;
}

CMesh* CMesh::Create_NonAnim(const string& strMeshName, const wstring& wstrMaterialName, const MTL_PATH& strMtlPath, VI_INFO tVIInfo,const XMatrix_1& matTransform ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CMesh* pMesh = new CMesh(pDevice, pContext);

    if (FAILED(pMesh->Initialilze_NonAnim(strMeshName, wstrMaterialName, strMtlPath, tVIInfo, matTransform)))
    {
        Safe_Release(pMesh);
        MSG_BOX("Mehs is failed To Create");
        return nullptr;
    }

    return pMesh;
}


CMesh* CMesh::Clone()
{
    CMesh* pCopyMesh = new CMesh(*this);

    if (pCopyMesh == nullptr) return nullptr;
    return pCopyMesh;
}



void CMesh::Free()
{
    __super::Free();

    /* CMesh::Free() */

    Safe_Release(m_pBuffer);
    Safe_Release(m_pMaterial);

    if (m_pMaterial->GetRef() != 0)
        int a = 0;
}
