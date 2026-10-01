#include "CLightManager.h"
#include "CLight.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CTransform.h"
#include "CVIBuffer_Rect.h"

CLightManager::CLightManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_tLightGroupBuffer() , m_pGameManager(CGameManager::GetInstance())
{
    /* 혹시모르니 확실하게  */
    m_tLightGroupBuffer.tGlobalLightBuffer = Light_Buffer_Data();
}

HRESULT CLightManager::Initialize()
{
    return S_OK;
}



const LIGHT_BUF_DATA& CLightManager::Get_LightBufferData(LIGHT_TYPE eType, uint32 iIndex)
{
    CLight* pFind_Light = Find_Light(eType, iIndex);

    return pFind_Light->Get_LightBuffer_Data();
}

CLight* CLightManager::Get_Light(LIGHT_TYPE eType, uint32 iIndex)
{
    return Find_Light(eType,iIndex);
}

const LIGHT_GROUP_BUF* CLightManager::Get_LightGroupBuffer()
{
    /*  전역 Light 객체는 Dirrectinal 항상 최상단에 존재한다 */
    /*  하나만 쓰라는 뜻 */

    auto& GlobalLightList = m_listLight[Cast_Uint(LIGHT_TYPE::DIRECTIONAL)];
    const auto& PointLightList = m_listLight[Cast_Uint(LIGHT_TYPE::POINT)];

    if (!GlobalLightList.empty())
        m_tLightGroupBuffer.tGlobalLightBuffer = m_listLight[Cast_Uint(LIGHT_TYPE::DIRECTIONAL)].front()->Get_LightBuffer_Data();
    

    m_tLightGroupBuffer.iPointLightNum = 0;
    for (const auto& Light : m_listLight[Cast_Uint(LIGHT_TYPE::POINT)])
        if (Light->Get_IsActive() == true)
        {
            m_tLightGroupBuffer.arrPointLightBuffer[m_tLightGroupBuffer.iPointLightNum++] = Light->Get_LightBuffer_Data();
        }

    return &m_tLightGroupBuffer;
}


HRESULT CLightManager::Set_GPU_LightGroupBuffer(CCameraObjectBase* pRenderCam)
{
    Float4 CamAreaXY = pRenderCam->Get_CameraRenderAreaXY_MinMax();


    auto& GlobalLightList = m_listLight[Cast_Uint(LIGHT_TYPE::DIRECTIONAL)];
    const auto& PointLightList = m_listLight[Cast_Uint(LIGHT_TYPE::POINT)];

    if (!GlobalLightList.empty())
        m_tLightGroupBuffer.tGlobalLightBuffer = m_listLight[Cast_Uint(LIGHT_TYPE::DIRECTIONAL)].front()->Get_LightBuffer_Data();


    m_tLightGroupBuffer.iPointLightNum = 0;
    for (const auto& Light : m_listLight[Cast_Uint(LIGHT_TYPE::POINT)])
    {
        if (Light)
        {
            if (Light->Get_IsActive() == true)
            {
                if(Light->Is_CameraInRange(CamAreaXY))
                {
                    /* the buffer holds MAX_LIGHTS entries; writing past it corrupts
                       iPointLightNum itself and crashes on the next assignment. */
                    if (m_tLightGroupBuffer.iPointLightNum >= MAX_LIGHTS)
                        break;
                    m_tLightGroupBuffer.arrPointLightBuffer[m_tLightGroupBuffer.iPointLightNum++] = Light->Get_LightBuffer_Data();
                }
            }
        }
    }

    if(FAILED(m_pGameManager->PipeLine_Set_GPU_LightGroupBuffer(m_tLightGroupBuffer)))
        return E_FAIL;

    return S_OK;
}


HRESULT CLightManager::Add_Light(const CLight::Light_Desc& tDesc)
{
    CLight* pLight = CLight::Create(tDesc, m_pDevice, m_pDeviceContext);

    NULLPTR_RETURN_EFAIL(pLight);

    m_listLight[Cast_Uint(tDesc.eLightType)].push_back(pLight);
    return S_OK;
}

HRESULT CLightManager::Add_Light(CLight* pLight)
{
    NULLPTR_RETURN_EFAIL(pLight);

    LIGHT_TYPE eType = pLight->Get_LightType();


    m_listLight[Cast_Uint(eType)].push_back(pLight);
   
    return S_OK;
}

HRESULT CLightManager::Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer, CCameraObjectBase* pRenderCam)
{
    NULLPTR_RETURN_EFAIL(pShader);
    NULLPTR_RETURN_EFAIL(pVIBuffer);
    NULLPTR_RETURN_EFAIL(pRenderCam);

    /* Render 하기전 카메라 위치값으로 점광원들어가는 개수 최적화할 에정 */
    this->Set_GPU_LightGroupBuffer(pRenderCam);

    if (FAILED(Render_DirectionalLight(pShader, pVIBuffer)))
        return E_FAIL;
    

    /* Point 개수가 0개면 진입 X */
    if (m_tLightGroupBuffer.iPointLightNum != 0)
    {
        if (FAILED(Render_PointLight(pShader, pVIBuffer)))
            return E_FAIL;
    }


    return S_OK;
}

HRESULT CLightManager::Render_DirectionalLight(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
    HRESULT hr{ E_FAIL };

    hr = pVIBuffer->Bind_Resource();

    hr = pShader->Apply_Pass("Directional_Light");

    hr = pVIBuffer->Draw();

    return hr;
}

HRESULT CLightManager::Render_PointLight(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
    HRESULT hr{ E_FAIL };

    hr = pVIBuffer->Bind_Resource();

    hr = pShader->Apply_Pass("Point_Light");

    hr = pVIBuffer->Draw();

    return hr;
}

HRESULT CLightManager::Clear_Light()
{
    for (uint32 i = 0; i < Cast_Uint(LIGHT_TYPE::END); ++i)
    {
        for (auto& Light : m_listLight[i])
            Safe_Release(Light);
        m_listLight[i].clear();
    }

    m_listLight->clear();

    return S_OK;
}

CLight* CLightManager::Find_Light(LIGHT_TYPE eType, uint32 iIndex)
{
    MAX_ENUM_CHECK_RETURN_NULLPTR( LIGHT_TYPE::END , eType);
    if (iIndex >= m_listLight[Cast_Uint(eType)].size())
        return nullptr;

    list<CLight*>::iterator iter = m_listLight[Cast_Uint(eType)].begin();

    std::advance(iter, iIndex);

    return *iter;
}

CLightManager* CLightManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CLightManager* pLightManager = new CLightManager(pDevice,pContext);

    if (FAILED(pLightManager->Initialize()))
    {
        Safe_Release(pLightManager);
        MSG_BOX("Light Manager is failed to Create");
        return nullptr;
    }

    return pLightManager;
}



void CLightManager::Free()
{

    for (uint32 i = 0; i < Cast_Uint(LIGHT_TYPE::END);  ++i)
    {
        for (auto& Light : m_listLight[i])
            Safe_Release(Light);
    }
}
