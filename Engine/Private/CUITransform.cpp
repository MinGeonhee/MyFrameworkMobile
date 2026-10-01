#include "CUITransform.h"
#include "CCanvas.h"

CUITransform::CUITransform(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CUIComponent(pDevice,pContext), m_vSize{0.f,0.f}, m_vPosition{0.f,0.f}, m_vAnchor{0.5f,0.5f}, m_isChange(true)
	, m_pParentUITransform(nullptr), m_isMoveEvent(true), m_vFirstAnchor{ 0.5f,0.5f }, m_vFirstPosition{0.f,0.f}
	, m_vFirstSize{1.f,1.f}
{
	XMStoreFloat4x4(&m_matLocal,XMMatrixIdentity());
	XMStoreFloat4x4(&m_matWorld,XMMatrixIdentity());
	m_matWorld._43 = 0.5f;
	m_matWorld._33 = 1.f;
}

CUITransform::CUITransform(const CUITransform& Proto)
	: CUIComponent(Proto) , m_vSize{ Proto.m_vSize }, m_vPosition{ Proto.m_vPosition }, m_vAnchor{ Proto.m_vAnchor }, m_matLocal(Proto.m_matLocal), m_matWorld(Proto.m_matWorld)\
	, m_isChange(Proto.m_isChange)
	, m_pParentUITransform(Proto.m_pParentUITransform), m_isMoveEvent(Proto.m_isMoveEvent)
	, m_vFirstAnchor{ Proto.m_vFirstAnchor }, m_vFirstPosition{ Proto.m_vFirstPosition }
	, m_vFirstSize{ Proto.m_vFirstSize }
{
}

HRESULT	CUITransform::Proto_Initialize()
{
	return S_OK;
}
HRESULT	CUITransform::Clone_Initialize(void* pArg)
{
	CUITransform::UITRANSFORM_DESC* pDesc =static_cast<CUITransform::UITRANSFORM_DESC*>(pArg);
	m_vAnchor	= pDesc->vAnchor;
	m_vPosition	= pDesc->vPosition;
	m_vSize		= pDesc->vSize;


	m_vFirstAnchor		= m_vAnchor;
	m_vFirstPosition	= m_vPosition;
	m_vFirstSize		= m_vSize;


	m_pParentUITransform = pDesc->pParentUITransform;
	
	return S_OK;
}

Vector4 CUITransform::Get_State_ToCal(STATE eState) CONST
{
	/*  저장용으로 들고있던 World Matrix 를 계산용으로 return 해준다 */
	return XMLoadFloat4x4(&m_matWorld).r[Cast_Uint(eState)];
}

Float4 CUITransform::Get_State_ToStore(STATE eState) CONST
{
	Float4 newFloat = {};
	memcpy(&newFloat, &m_matWorld.m[Cast_Uint(eState)], sizeof(Float4));
	return  newFloat;
}


void CUITransform::Set_State(STATE eState, const Float4& vFloat4)
{
	memcpy(&m_matWorld.m[Cast_Uint(eState)], &vFloat4, sizeof(Float4));
}
void CUITransform::Set_State(STATE eState, const fVector4_1& vVector4)
{
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m_matWorld.m[Cast_Uint(eState)]), vVector4);
}


void CUITransform::Update_World(const CCanvas* pCanvas)
{
	NULLPTR_RETURN_NULL(pCanvas);

	if (!m_isMoveEvent) return;

	if (m_pParentUITransform == nullptr)
	{
		VIEWPORT_DESC tCanvasViewPort = pCanvas->Get_CanvasViewPort();

		/* 앵커좌표 = 내가 캔버스에서 어느위치에 존재할것인가에대한값 */
		Float2 vAnchor = { -tCanvasViewPort.TopLeftX + (tCanvasViewPort.Width) * m_vAnchor.x ,tCanvasViewPort.TopLeftY + (tCanvasViewPort.Height) * m_vAnchor.y };

		/* 마지막 파이널 포즈 =  캔버스 위치좌표를 토대로 이동시키고 내 오프셋좌표를 더해준다. */
		Vector4 vFinalPos = XMLoadFloat2(&vAnchor) + XMLoadFloat2(&m_vPosition);

		XMStoreFloat4x4(&m_matWorld, XMMatrixScalingFromVector(XMLoadFloat2(&m_vSize))
			* XMMatrixTranslationFromVector(vFinalPos));
		m_matWorld._41 -= tCanvasViewPort.Width * 0.5f - tCanvasViewPort.TopLeftX;
		m_matWorld._42 = -m_matWorld._42 + tCanvasViewPort.Height * 0.5f + tCanvasViewPort.TopLeftY;
	}
	else
	{
		XMMATRIX ParentMatrix = XMLoadFloat4x4(m_pParentUITransform->Get_WorldPtr());
		XMVECTOR ParetnTrans = ParentMatrix.r[3];
		/* 부모의 Size정보를 가져온다 */
		Float2 vParentSize = m_pParentUITransform->Get_Size();

		/* 월드 공간상에서의 TopLeft  X Y 좌표 */
		float TopLeftX = XMVectorGetX(ParetnTrans) - vParentSize.x * 0.5f;
		float TopLeftY = XMVectorGetY(ParetnTrans) + vParentSize.y * 0.5f;

		float fAnchorX = vParentSize.x * m_vAnchor.x;
		float fAnchorY = vParentSize.y * m_vAnchor.y;

		Float2 vFinalPos = { TopLeftX + fAnchorX + m_vPosition.x
							,TopLeftY - fAnchorY + m_vPosition.y };

		XMStoreFloat4x4(&m_matWorld, XMMatrixScalingFromVector(XMLoadFloat2(&m_vSize))
			* XMMatrixTranslationFromVector(XMLoadFloat2(&vFinalPos)));
		m_matWorld._43 = 0.5f;
	}

	return;
}

void CUITransform::Update_World_Font(const CCanvas* pCanvas)
{

}

bool CUITransform::Move(const fVector4_1& vDir, float fDelta)
{
	return true;
}


CUITransform* CUITransform::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CUITransform* pTransform = new CUITransform(pDevice, pContext);
	if (FAILED(pTransform->CUITransform::Proto_Initialize()))
	{
		Safe_Release(pTransform);
		MSG_BOX(" UI Transform is failed to Create ");
		return nullptr;
	}
	return pTransform;
}

CComponent* CUITransform::Clone(void* pArg)
{
	CUITransform* pTransform = new CUITransform(*this);
	if (FAILED(pTransform->CUITransform::Clone_Initialize(pArg)))
	{
		Safe_Release(pTransform);
		MSG_BOX(" UI Transform is failed to Clone ");
		return nullptr;
	}
	return pTransform;
}

void	CUITransform::Free()
{
	/*  CComponent::Free() */
	__super::Free();


	/*  CUITrasnform::Free() */
}


