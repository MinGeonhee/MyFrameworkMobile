#include "CScreenShot.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

uint32 CScreenShot::m_iScreenShotNum = 0;

CScreenShot::CScreenShot(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext)
{
}

ComPtr<Texture2D> CScreenShot::Get_ScreenShot_Textre2D()
{
	return m_pTexture2D;
}

HRESULT CScreenShot::Save_ScreenShot(const wstring& wstrFilePath)
{
	NULLPTR_RETURN_EFAIL(m_pTexture2D);

	/* 원본 : Staging 텍스처 + DirectXTex CaptureTexture + SaveToWICFile(JPG) */
	/* 현재 : Staging 텍스처 + Map(READ) + stb_image_write(JPG)                 */
	Diligent::TextureDesc tDesc = m_pTexture2D->GetDesc();
	tDesc.Name = "ScreenShot Staging";
	tDesc.Usage = Diligent::USAGE_STAGING;
	tDesc.BindFlags = Diligent::BIND_NONE;
	tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
	tDesc.MipLevels = 1;

	ComPtr<Texture2D> pSaveTexture2D;
	m_pDevice->CreateTexture(tDesc, nullptr, pSaveTexture2D.ReleaseAndGetAddressOf());
	NULLPTR_RETURN_EFAIL(pSaveTexture2D);

	Diligent::CopyTextureAttribs tCopy{ m_pTexture2D.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		pSaveTexture2D.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION };
	m_pDeviceContext->CopyTexture(tCopy);
	m_pDeviceContext->WaitForIdle();

	Diligent::MappedTextureSubresource tMapped{};
	m_pDeviceContext->MapTextureSubresource(pSaveTexture2D.Get(), 0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT, nullptr, tMapped);
	if (tMapped.pData == nullptr)
		return E_FAIL;

	std::vector<uint8> vecPixel(static_cast<size_t>(tDesc.Width) * tDesc.Height * 4);
	const bool isBGRA = tDesc.Format == Diligent::TEX_FORMAT_BGRA8_UNORM || tDesc.Format == Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB;
	for (uint32 y = 0; y < tDesc.Height; ++y)
	{
		const uint8* pSrc = static_cast<const uint8*>(tMapped.pData) + static_cast<size_t>(tMapped.Stride) * y;
		uint8* pDst = &vecPixel[static_cast<size_t>(y) * tDesc.Width * 4];
		::memcpy(pDst, pSrc, static_cast<size_t>(tDesc.Width) * 4);
		if (isBGRA)
			for (uint32 x = 0; x < tDesc.Width; ++x)
				std::swap(pDst[x * 4], pDst[x * 4 + 2]);
	}
	m_pDeviceContext->UnmapTextureSubresource(pSaveTexture2D.Get(), 0, 0);

	wstring wstr = wstrFilePath + L"ScreenShot" + to_wstring(m_iScreenShotNum++) + L".jpg";
	const std::string strPath = std::filesystem::path(wstr).string();
	if (stbi_write_jpg(strPath.c_str(), static_cast<int>(tDesc.Width), static_cast<int>(tDesc.Height), 4, vecPixel.data(), 95) == 0)
		return E_FAIL;

	return S_OK;
}

HRESULT CScreenShot::Initialize(ComPtr<Texture2D> pTexture2D)
{
	/* 매개변수로 받아온 백버퍼 텍스처를 셰이더에서 읽을 수 있는 텍스처로 복사해둔다 */
	NULLPTR_RETURN_EFAIL(pTexture2D);

	Diligent::TextureDesc tDesc = pTexture2D->GetDesc();
	tDesc.Name = "ScreenShot";
	/* 바인드값은 이대로 두기 */
	tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	tDesc.Usage = Diligent::USAGE_DEFAULT;
	tDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
	tDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;

	m_pDevice->CreateTexture(tDesc, nullptr, m_pTexture2D.ReleaseAndGetAddressOf());
	NULLPTR_RETURN_EFAIL(m_pTexture2D);

	/* 리소스 카피 */
	Diligent::CopyTextureAttribs tCopy{ pTexture2D.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		m_pTexture2D.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION };
	m_pDeviceContext->CopyTexture(tCopy);

	return S_OK;
}

CScreenShot* CScreenShot::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, ComPtr<Texture2D> pTexture2D)
{
	CScreenShot* pScreenShot = new CScreenShot(pDevice, pContext);

	if (FAILED(pScreenShot->Initialize(pTexture2D)))
	{
		Safe_Release(pScreenShot);
		MSG_BOX("ScreenShot is failed to Create");
		return nullptr;
	}

	return pScreenShot;
}

void CScreenShot::Free()
{
	__super::Free();
	/* ScreenShot::Free() */
}
