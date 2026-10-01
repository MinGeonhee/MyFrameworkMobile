#include "CCanvas.h"
#include "CGraphicDevice.h"
#include "CGameManager.h"
#include "CUITransform.h"
#include "CUILayer.h"
#include "CShader.h"

CCanvas::CCanvas(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_umapUILayer{}, m_pShader(nullptr) , m_isActive(true)
    , m_tCanvasViewPort{}, m_matCanvas{} , m_isChangeViewPort{false}
{
}


HRESULT CCanvas::Initialize(const Canvas_Desc* pDesc)
{  
    
    CUITransform::UITransform_Desc tDesc{};

    m_tCanvasViewPort.TopLeftX = pDesc->vCanvasStartPos.x;
    m_tCanvasViewPort.TopLeftY = pDesc->vCanvasStartPos.y;
    m_tCanvasViewPort.Width = pDesc->vCanvasSize.x;
    m_tCanvasViewPort.Height = pDesc->vCanvasSize.y;
    m_tCanvasViewPort.MaxDepth = 1.f;
    m_tCanvasViewPort.MinDepth = 0.f;

    XMStoreFloat4x4(&m_matCanvas, XMMatrixOrthographicLH(m_tCanvasViewPort.Width, m_tCanvasViewPort.Height, 0.f, 1.f));


    m_pShader = static_cast<CShader*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(pDesc->wstrShaderKey,RESOURCE_TYPE::SHADER,false,nullptr));
    NULLPTR_RETURN_EFAIL(m_pShader);

    return S_OK;
}

HRESULT CCanvas::Ready_Component(void* pArg)
{

    return S_OK;
}

HRESULT CCanvas::Add_UILayer(const wstring& wstrAddKey, CUILayer* pUILayer)
{
    CUILayer* pFind = Get_UILayer(wstrAddKey);
    if (pFind != nullptr) return E_FAIL;

    m_umapUILayer.emplace(wstrAddKey,pUILayer);


    return S_OK;
}

CUILayer* CCanvas::Get_UILayer(const wstring& wstrFindKey) const
{
    auto pFind = m_umapUILayer.find(wstrFindKey);
    if (pFind == m_umapUILayer.end()) return nullptr;
    return pFind->second;
}


void CCanvas::Pre_Update(float DT)
{
    for (auto& UILayer : m_umapUILayer)
        if(UILayer.second)
            if(UILayer.second->Get_IsActive())
                UILayer.second->Pre_Update(this,DT);
}

void CCanvas::Update(float DT)
{
    for (auto& UILayer : m_umapUILayer)
        if (UILayer.second)
            if (UILayer.second->Get_IsActive())
                UILayer.second->Update(this,DT);



    if (m_isChangeViewPort == true)
    {
        XMStoreFloat4x4(&m_matCanvas, XMMatrixOrthographicLH(m_tCanvasViewPort.Width, m_tCanvasViewPort.Height, 0.f, 1.f));
        m_isChangeViewPort = false;
    }
}

void CCanvas::Late_Update(float DT)
{
    for (auto& UILayer : m_umapUILayer)
        if (UILayer.second)
            if (UILayer.second->Get_IsActive())
                UILayer.second->Late_Update(this,DT);
}

HRESULT	 CCanvas::Render()
{
    /*  Sort  */
    vector<CUILayer*> vecLayers;
    vecLayers.reserve(m_umapUILayer.size());

    for (auto& UILayer : m_umapUILayer)
        if (UILayer.second)
            if (UILayer.second->Get_IsActive())
                vecLayers.push_back(UILayer.second);

    std::sort(vecLayers.begin(), vecLayers.end(),
        [](const CUILayer* a, const CUILayer* b) {
            return a->Get_UILayerOrder() > b->Get_UILayerOrder();
        });


    /* 직교투영 세팅 */
    NULLPTR_RETURN_EFAIL(m_pShader);
    CameraBufferData tCameraBuffer;

    CGraphicDevice::Get_Instance()->RS_SetViewports(1,&m_tCanvasViewPort);
    //XMStoreFloat4x4(&m_matCanvas,XMMatrixOrthographicLH(1280.f,720.f,0.f,1.f));


    m_pShader->Set_GPU_Canvas(m_matCanvas);

        for (auto& UIObject : vecLayers)
        if (UIObject)
            if (UIObject->Get_IsActive())
                if(FAILED(UIObject->Render(this))) return S_OK;

    return S_OK;
}

CCanvas* CCanvas::Create(const Canvas_Desc* pDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CCanvas* pCanvas = new CCanvas(pDevice, pContext);

    if (FAILED(pCanvas->CCanvas::Initialize(pDesc)))
    {
        Safe_Release(pCanvas);
        MSG_BOX("Canvas is failed to Create");
        return nullptr;
    }

    return pCanvas;
}

void CCanvas::Free()
{
    /* CGmaeObject::Free() */
    __super::Free();

    /* CCanvas::Free() */

    for (auto& UILayer : m_umapUILayer)
        Safe_Release(UILayer.second);


    Safe_Release(m_pShader);
}
