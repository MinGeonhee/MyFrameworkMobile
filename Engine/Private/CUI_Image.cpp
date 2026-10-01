#include "CUI_Image.h"
#include "CVIBuffer_Rect.h"
#include "CCanvas.h"
#include "CGameManager.h"
#include "CTexture.h"
#include "CShader.h"
#include "CUIObject.h"

CUI_Image::CUI_Image(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CUIComponent(pDevice, pContext), m_pTexture(nullptr), m_pBuffer(nullptr), m_strPass("Default"), m_fAlpha(1.f)
	, m_iTextureIndex(0)
{
}

CUI_Image::CUI_Image(const CUI_Image& Proto)
	: CUIComponent(Proto), m_pTexture(Proto.m_pTexture), m_pBuffer(Proto.m_pBuffer), m_fAlpha(Proto.m_fAlpha)
	, m_strPass(Proto.m_strPass), m_iTextureIndex(Proto.m_iTextureIndex)
{
}

CUI_Image::~CUI_Image()
{
}

HRESULT	CUI_Image::Proto_Initialize()
{
	if (FAILED(__super::Proto_Initialize()))
		return E_FAIL;



	if (FAILED(Ready_Buffer()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_Image::Clone_Initialize(void* pArg)
{
	if (FAILED(__super::Clone_Initialize(pArg)))
		return E_FAIL;

	UI_IMAGE_DESC* pDesc = static_cast<UI_IMAGE_DESC*>(pArg);


	m_iTextureIndex = pDesc->iTextureIndex;


	if (FAILED(Ready_Resource(pDesc->wstrFindTextureKey)))
		return E_FAIL;


	return S_OK;
}


uint32 CUI_Image::Get_TexturCount() const
{
	if (!m_pTexture) return -1;


	return m_pTexture->Get_TextureCnt();
}

HRESULT CUI_Image::Ready_Buffer()
{
	m_pBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pBuffer);

	return S_OK;
}

HRESULT CUI_Image::Ready_Resource(const wstring& wstrTextureKey)
{
	m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(wstrTextureKey, RESOURCE_TYPE::TEXTURE, false, nullptr));
	NULLPTR_RETURN_EFAIL(m_pTexture);

	return S_OK;
}

void CUI_Image::Pre_Update(float DT)
{
	__super::Pre_Update(DT);
}

void CUI_Image::Update(float DT)
{
	__super::Update(DT);
}

void CUI_Image::Late_Update(float DT)
{
	__super::Late_Update(DT);
}

HRESULT CUI_Image::Render(CCanvas* pCanvas)
{
	NULLPTR_RETURN_EFAIL(m_pBuffer);
	NULLPTR_RETURN_EFAIL(m_pTexture);

	/* World Transform 가져오기 */

	CShader* pShader = pCanvas->Get_Shader();

	const Float4x4 matWorld = static_cast<CUIObject*>(m_pOwner)->Get_UITransform()->Get_World();

	pShader->Set_GPU_UIAplha(m_fAlpha);

	pShader->Set_GPU_Transform(matWorld);
	pShader->Set_GPU_Texture(m_pTexture->GetShaderResourceView(m_iTextureIndex));

	m_pBuffer->Bind_Resource();
	pShader->Apply_Pass(m_strPass);
	m_pBuffer->Draw();

	return S_OK;
}


CUI_Image* CUI_Image::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CUI_Image* pImage = new CUI_Image(pDevice, pContext);

	if (FAILED(pImage->Proto_Initialize()))
	{
		Safe_Release(pImage);
		MSG_BOX("UI_Image is failed to Create");
		return nullptr;
	}

	return pImage;
}


CComponent* CUI_Image::Clone(void* pArg)
{
	CUI_Image* pImage = new CUI_Image(*this);

	if (FAILED(pImage->Clone_Initialize(pArg)))
	{
		Safe_Release(pImage);
		MSG_BOX("UI_Image is failed to Clone");
		return nullptr;
	}

	return pImage;
}

void CUI_Image::Free()
{
	/* CUIComponent::Free() */
	__super::Free();


	/* UI_Image::Free() */
	if (m_eMakeType == MAKE_TYPE::PROTOTYPE)
		Safe_Release(m_pBuffer); // PrototType일떄만 버퍼지우기 버퍼는 하나를 통일해서 사용해주기 나중에문제생기면 깊복

	Safe_Release(m_pTexture);

}