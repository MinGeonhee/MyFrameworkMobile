#include "pch.h"
#include "CTestObject.h"

#include "CGameManager.h"
#include "CShader.h"
#include "CModel.h"
#include "CTransform.h"

CTestObject::CTestObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CGameObject(pDevice, pContext)
{
}

CTestObject::CTestObject(const CTestObject& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CTestObject::Proto_Initialize()
{
	if (FAILED(CGameObject::Proto_Initialize()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTestObject::Clone_Initialize(void* pArg)
{
	if (FAILED(CGameObject::Clone_Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTestObject::Ready_Component(void* pArg)
{
	TESTOBJECT_DESC* pDesc = static_cast<TESTOBJECT_DESC*>(pArg);
	NULLPTR_RETURN_EFAIL(pDesc);

	m_strPass = pDesc->strPass;
	m_isAnim = pDesc->isAnim;
	m_iAnimIndex = pDesc->iAnimIndex;
	m_fAnimSpeed = pDesc->fAnimSpeed;
	m_fTurnSpeed = pDesc->fTurnSpeed;

	/* 애니메이션 모델은 뼈 행렬을 오브젝트마다 따로 가져야 하므로 복사본을 받는다 (원본 CObject 와 동일) */
	m_pModel = static_cast<CModel*>(m_pGameManager->ResourceManager_Get_Resource(pDesc->wstrModelKey, RESOURCE_TYPE::MODEL, true));
	m_pShader = static_cast<CShader*>(m_pGameManager->ResourceManager_Get_Resource(pDesc->wstrShaderKey, RESOURCE_TYPE::SHADER, false));
	NULLPTR_RETURN_EFAIL(m_pModel);
	NULLPTR_RETURN_EFAIL(m_pShader);

	if (m_isAnim)
	{
		m_pModel->Set_Owner(this);
		m_pModel->Change_Animation(m_iAnimIndex, ANIMATION_PLAYTYPE::LOOP);
	}

	return S_OK;
}

void CTestObject::Update(float DT)
{
	if (m_fTurnSpeed != 0.f)
		m_pTransformCom->Rotation_AccRadian(0.f, m_fTurnSpeed * DT, 0.f);

	if (m_isAnim && m_pModel != nullptr)
		m_pModel->Play_Animation(m_fAnimSpeed, DT);
}

void CTestObject::Late_Update(float DT)
{
}

HRESULT CTestObject::Add_RenderGroup()
{
	return m_pGameManager->RenderManager_Add_RenderGroup(RENDER_GROUP::NONBLEND, this);
}

HRESULT CTestObject::Render(CCameraObjectBase* pRenderCam)
{
	NULLPTR_RETURN_EFAIL(m_pShader);
	NULLPTR_RETURN_EFAIL(m_pModel);

	m_pShader->Set_GPU_Transform(m_pTransformCom->Get_matWorld());

	const uint32 iMeshCount = m_pModel->Get_MeshCount();
	for (uint32 i = 0; i < iMeshCount; ++i)
	{
		if (FAILED(m_pModel->Set_GPU_Material(m_pShader, i)))
			continue;

		if (m_isAnim)
			m_pModel->Set_GPU_ConnectBoneMatrix(m_pShader, Shader_ArrayBoneMatrix, i);

		if (FAILED(m_pModel->Draw(m_pShader, m_strPass, i)))
			return E_FAIL;
	}

	return S_OK;
}

void CTestObject::Change_Animation(uint32 iAnimIndex)
{
	if (m_pModel == nullptr || !m_isAnim)
		return;

	m_iAnimIndex = iAnimIndex;
	m_pModel->Change_Animation(iAnimIndex, ANIMATION_PLAYTYPE::LOOP);
}

uint32 CTestObject::Get_AnimationCount() const
{
	return m_pModel != nullptr ? m_pModel->Get_AnimationCount() : 0;
}

CTestObject* CTestObject::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CTestObject* pInstance = new CTestObject(pDevice, pContext);

	if (FAILED(pInstance->Proto_Initialize()))
	{
		Safe_Release(pInstance);
		MSG_BOX("TestObject is failed to Create");
		return nullptr;
	}

	return pInstance;
}

CGameObject* CTestObject::Clone(void* pArg)
{
	CTestObject* pInstance = new CTestObject(*this);

	if (FAILED(pInstance->Clone_Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("TestObject is failed to Clone");
		return nullptr;
	}

	return pInstance;
}

void CTestObject::Free()
{
	__super::Free();

	Safe_Release(m_pModel);
	Safe_Release(m_pShader);
}
