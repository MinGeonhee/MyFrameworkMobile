#include "CPipeLine.h"
#include "CShader.h"
#include "CGameManager.h"
#include "CConstantBuffer.h"


CPipeLine::CPipeLine(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_umapTransformMatrices{}, m_umapTransformInverseMatrices{} , m_pDevice(pDevice) , m_pDeviceContext(pContext)
    , m_pGameManager(CGameManager::GetInstance()) ,m_pLightGroupCB(nullptr), m_pCameraCB(nullptr)
    , m_pMouseCB(nullptr), m_tMouseCBData{}
{
    m_umapTransformMatrices.clear();
    m_umapTransformInverseMatrices.clear();
    m_umapCameraPosition.clear();

    Safe_AddRef(m_pGameManager);
}

HRESULT CPipeLine::Initialize()
{
    /* Cam Constant Buffer */
    m_pCameraCB = CConstantBuffer<CAM_BUF_DATA>::Create(m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(m_pCameraCB);


    m_pLightGroupCB = CConstantBuffer<LIGHT_GROUP_BUF>::Create(m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(m_pLightGroupCB);

    m_pMouseCB = CConstantBuffer<MOUSE_BUF_DATA>::Create(m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(m_pMouseCB);


    return S_OK;
}

HRESULT CPipeLine::Set_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, const XMatrix_1& TransformMatrix)
{
    auto& InputMatrix = m_umapTransformMatrices[wstrFindKey][Cast_Uint(eTransformMatrix)];
    auto& InputInverseMatrix = m_umapTransformInverseMatrices[wstrFindKey][Cast_Uint(eTransformMatrix)];
    auto& InputCameraPos = m_umapCameraPosition[wstrFindKey];

    /* 키값이 없을때는? */
   XMStoreFloat4x4(&InputMatrix,TransformMatrix);

   /* 역행렬까지 구해서 넣어두기 */
   XMStoreFloat4x4(&InputInverseMatrix, XMMatrixInverse(nullptr, TransformMatrix));

   /* 카메라 포지션 까지*/
   if (eTransformMatrix == D3DTS::VIEW)
   {
       InputCameraPos.x = InputInverseMatrix._41;
       InputCameraPos.y = InputInverseMatrix._42;
       InputCameraPos.z = InputInverseMatrix._43;
       InputCameraPos.w = 1.f;
   }

   /* Set 에서 해주는 이유는 카메라 업데이트가 도는애들만 이 Set 함수가 불리게될텐데 이떄만 해주기 위해서 */

   return S_OK;
}

HRESULT CPipeLine::Set_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, const Float4x4& TransformMatrix)
{
    auto& InputMatrix = m_umapTransformMatrices[wstrFindKey][Cast_Uint(eTransformMatrix)];
    auto& InputInverseMatrix = m_umapTransformInverseMatrices[wstrFindKey][Cast_Uint(eTransformMatrix)];
    auto& InputCameraPos = m_umapCameraPosition[wstrFindKey];

    /* 키값이 없을때는? */
    XMStoreFloat4x4(&InputMatrix, XMLoadFloat4x4(&TransformMatrix));

    /* 역행렬까지 구해서 넣어두기 */
    XMStoreFloat4x4(&InputInverseMatrix, XMMatrixInverse(nullptr, XMLoadFloat4x4(&TransformMatrix)));


    /* 카메라 포지션 까지*/
    if (eTransformMatrix == D3DTS::VIEW)
    {
        InputCameraPos.x = InputInverseMatrix._41;
        InputCameraPos.y = InputInverseMatrix._42;
        InputCameraPos.z = InputInverseMatrix._43;
        InputCameraPos.w = 1.f;
    }

    /* Set 에서 해주는 이유는 카메라 업데이트가 도는애들만 이 Set 함수가 불리게될텐데 이떄만 해주기 위해서 */

    return S_OK;
}

Float4x4 CPipeLine::Get_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) CONST
{

    if (isInverse == true)
    {
        if (m_umapTransformInverseMatrices.empty()) return Float4x4();


        return m_umapTransformInverseMatrices.at(wstrFindKey)[Cast_Uint(eTransformMatrix)];
    }
    if (isInverse == false)
    {
        if (m_umapTransformMatrices.empty()) return Float4x4();

        return m_umapTransformMatrices.at(wstrFindKey)[Cast_Uint(eTransformMatrix)];
    }

    return Float4x4();
}

const Float4x4* CPipeLine::Get_PipeLineMatrixPtr(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) const
{
    if (isInverse == true)
    {
        if (m_umapTransformInverseMatrices.empty()) return nullptr;


        return &m_umapTransformInverseMatrices.at(wstrFindKey)[Cast_Uint(eTransformMatrix)];
    }
    if (isInverse == false)
    {
        if (m_umapTransformMatrices.empty()) return nullptr;

        return &m_umapTransformMatrices.at(wstrFindKey)[Cast_Uint(eTransformMatrix)];
    }

    return nullptr;
}


HRESULT CPipeLine::Link_LightGroupBuffer(CShader* pShader)
{
    NULLPTR_RETURN_EFAIL(pShader);
    NULLPTR_RETURN_EFAIL(m_pLightGroupCB);

    /* 원본 : EffectConstantBuffer->SetConstantBuffer(공용 버퍼) */
    m_pLightGroupCB->CopyData(m_tLightGroupData);

    return pShader->Bind_ConstantBuffer(GB_Shader_LightGroupBuffer, m_pLightGroupCB);
}

HRESULT CPipeLine::Link_CameraBuffer(CShader* pShader)
{
    NULLPTR_RETURN_EFAIL(pShader);
    NULLPTR_RETURN_EFAIL(m_pCameraCB);

    /* 원본 : EffectConstantBuffer->SetConstantBuffer(공용 버퍼) */
    m_pCameraCB->CopyData(m_tCameraCBData);

    return pShader->Bind_ConstantBuffer(GB_Shader_CameraBuffer, m_pCameraCB);
}

HRESULT CPipeLine::Link_MouseBuffer(CShader* pShader)
{
    NULLPTR_RETURN_EFAIL(pShader);
    NULLPTR_RETURN_EFAIL(m_pMouseCB);

    /* 원본 : EffectConstantBuffer->SetConstantBuffer(공용 버퍼) */
    m_pMouseCB->CopyData(m_tMouseCBData);

    return pShader->Bind_ConstantBuffer(GB_Shader_MouseBuffer, m_pMouseCB);
}

Float4 CPipeLine::Get_CameraPosition(const std::wstring& wstrFindKey) CONST
{
    auto iter = m_umapCameraPosition.find(wstrFindKey);

    if (iter == m_umapCameraPosition.end())
        return Float4();

    return m_umapCameraPosition.at(wstrFindKey);
}

const void CPipeLine::Clear_PipeLine()
{
    m_umapCameraPosition.clear();
    m_umapTransformInverseMatrices.clear();
    m_umapTransformMatrices.clear();
}

HRESULT CPipeLine::Bind_PipeLine()
{
    HRESULT hr{ E_FAIL };

    hr = Set_GPU_LightGroupBuffer();

    return hr;
}

HRESULT CPipeLine::Link_EffectConstantBuffer(GLOBAL_CB_TYPE eCBType, CShader* pShader)
{
    HRESULT hr{ E_FAIL };

    switch (eCBType)
    {
    case Engine::GLOBAL_CB_TYPE::CAMERA:
        hr = CPipeLine::Link_CameraBuffer(pShader);
        break;
    case Engine::GLOBAL_CB_TYPE::LIGHTGROUP:
        hr = CPipeLine::Link_LightGroupBuffer(pShader);
        break;
    case Engine::GLOBAL_CB_TYPE::MOUSE:
        hr = CPipeLine::Link_MouseBuffer(pShader);
        break;
    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}


HRESULT CPipeLine::Set_GPU_CameraBuffer(const wstring& wstrFindKey, const CAM_SHADER_INFO& tCamShaderInfo)
{
    HRESULT hr{ E_FAIL };

    /* CB로 바꿔서 건내주기 한번에 복사 할 수 있게 */

    /* Cam Buffer */
    m_tCameraCBData.matView = m_umapTransformMatrices[wstrFindKey][Cast_Uint(D3DTS::VIEW)];
    m_tCameraCBData.matProj = m_umapTransformMatrices[wstrFindKey][Cast_Uint(D3DTS::PROJ)];;
    XMStoreFloat4x4(&m_tCameraCBData.matViewProj, XMLoadFloat4x4(&m_tCameraCBData.matView) * XMLoadFloat4x4(&m_tCameraCBData.matProj));

    /* Inv Cam Buffer */
    m_tCameraCBData.matViewInv = m_umapTransformInverseMatrices[wstrFindKey][Cast_Uint(D3DTS::VIEW)];
    m_tCameraCBData.matProjInv = m_umapTransformInverseMatrices[wstrFindKey][Cast_Uint(D3DTS::PROJ)];

    /* CamPos */
    m_tCameraCBData.tCamPosition = m_umapCameraPosition[wstrFindKey];

    /* CamInfo */
    m_tCameraCBData.tCamInfo = tCamShaderInfo;

    hr = m_pCameraCB->CopyData(m_tCameraCBData);

    return S_OK;
}

HRESULT CPipeLine::Set_GPU_LightGroupBuffer(const LIGHT_GROUP_BUF& tLightGroupBuf)
{
    m_tLightGroupData = tLightGroupBuf;
    return m_pLightGroupCB->CopyData(m_tLightGroupData);
}

HRESULT CPipeLine::Set_GPU_LightGroupBuffer()
{
    HRESULT hr{ E_FAIL };

    CONST LIGHT_GROUP_BUF* pBuffer = m_pGameManager->LightManager_Get_LightGroupBuffer();
    NULLPTR_RETURN_EFAIL(pBuffer);
    NULLPTR_RETURN_EFAIL(m_pLightGroupCB);

    hr = m_pLightGroupCB->CopyData(*pBuffer);

    return hr;
}

HRESULT CPipeLine::Set_GPU_MouseBuffer(const MOUSE_BUF_DATA& tBuffer)
{
    NULLPTR_RETURN_EFAIL(m_pMouseCB);

    return m_pMouseCB->CopyData(tBuffer);
}


void CPipeLine::Update(float DT)
{
    /* 아직 Update 할게없음 */
}
void CPipeLine::Late_Update(float DT)
{
    /* 아직 Update 할게없음 */
}


CPipeLine* CPipeLine::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CPipeLine* pPipeLine = new CPipeLine(pDevice ,pContext);

    INIT_FAILED_CEHCK(pPipeLine);

    return pPipeLine;
}

void CPipeLine::Free()
{
    __super::Free();
    
    /* CPipeLine::Free() */

    Safe_Release(m_pGameManager);
    Safe_Release(m_pCameraCB);
    Safe_Release(m_pLightGroupCB);
    Safe_Release(m_pMouseCB);
}
