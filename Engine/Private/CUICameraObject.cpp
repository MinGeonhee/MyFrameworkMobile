#include "CUICameraObject.h"
#include "CCameraComponent.h"
#include "CGameManager.h"

/* CShader */

CUICameraObject::CUICameraObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CCameraObjectBase(pDevice,pContext)
{

}



HRESULT	CUICameraObject::Camera_Initialize(void* pArg)
{
    NULLPTR_RETURN_EFAIL(pArg);

    /* GameObject 초기화 및 Transform 생성할꺼다 */
    if (FAILED(__super::Camera_Initialize(pArg)))
        return E_FAIL;

    /* 내가 필요한 컴포넌트 추가 */
    if (FAILED(Add_Component(pArg)))
        return E_FAIL;

    UICAM_DESC* tUIDesc = static_cast<UICAM_DESC*>(pArg);
    NULLPTR_RETURN_EFAIL(tUIDesc);
  

    m_vecRenderGroupID = tUIDesc->vecRenderGroupID;


    /* 미리 UI 용 행렬 만들어서 PipeLine에 올려버리기 */
    m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW , XMMatrixIdentity());
    m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixOrthographicLH(m_tInfo.tViewPortDesc.Width, m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));

    return S_OK;
}

HRESULT	CUICameraObject::Add_Component(void* pArg)
{
    /* Add_Component */


    return S_OK;
}


void CUICameraObject::Setting_UICamera()
{
}



void			CUICameraObject::Pre_Update(float DT)
{

}
void			CUICameraObject::Update(float DT)
{

}
void			CUICameraObject::Late_Update(float DT)
{
}

CUICameraObject* CUICameraObject::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, void* pArg)
{
    CUICameraObject* pUICameraObject = new CUICameraObject(pDevice, pContext);

    if (FAILED(pUICameraObject->Camera_Initialize(pArg)))
    {
        Safe_Release(pUICameraObject);
        MSG_BOX("CameraObject is failed to Create");
        return nullptr;
    }


    return pUICameraObject;
}

void CUICameraObject::Free()
{
    __super::Free();

    /* CCameraObject::Free() */
}

void CUICameraObject::Add_ColliderGroup()
{
    return;
}

CUICameraObject* CUICameraObject::Clone(void* pArg)
{
    return nullptr;
}
