#pragma once
#include "CResource.h"

namespace Engine
{
	class CShader;
}

NS_BEGIN(Engine)

class ENGINE_DLL  CTexture : public CResource
{
private:
	explicit CTexture(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CTexture(const CTexture& Prototype);
	virtual ~CTexture() {}
private:
	HRESULT	Initialize(const wstring& Path, uint32 iTextureCnt);
	HRESULT	Initialize(Texture2D* pTexture2DCom);
	virtual HRESULT		Copy_Initialize(void* pArg);
public:
	ShaderResourceView*							GetShaderResourceView(uint32 iIndex= 0 ) { MAX_ENUM_CHECK_RETURN_NULLPTR(m_iTextureCnt, iIndex); return m_vecTexture[iIndex]; }
	ShaderResourceView**						GetShaderResourceViewPP(uint32 iIndex= 0 ) { MAX_ENUM_CHECK_RETURN_NULLPTR(m_iTextureCnt, iIndex); return &m_vecTexture[iIndex]; }
	const uint32&								Get_TextureCnt() const { return m_iTextureCnt; }

	/* Defualt Texture */
	HRESULT										Set_GPU_Texture(CShader* pShader);
	HRESULT										Set_GPU_Textures(CShader* pShader, uint32 iCountTex , uint32 iOffset = 0);

	/* Mix Textures */
	HRESULT										Set_GPU_MixTextureArr(CShader* pShader,uint32 iCountTex, uint32 iOffset = 0);
	HRESULT										Set_GPU_MaskTexture(CShader* pShader);

private:
	wstring										m_wstrFileName;
private:
	vector<ShaderResourceView*>					m_vecTexture;
	uint32										m_iTextureCnt;
public:
	static CTexture* Create(const wstring& Path, uint32 iTextureCnt,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	static CTexture* Create( Texture2D* pTexture2DCom ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);

	CResource*				Copy_Resource(void* pArg);

	virtual void			Free();
};

/* 파일 → Diligent Texture (png / jpg / tga / bmp : stb_image , dds : 엔진 DDS 로더) */
ENGINE_DLL HRESULT	Load_TextureFromFile(Device* pDevice, const wstring& wstrPath, Diligent::ITexture** ppTexture);
/* RGBA8 메모리 → Diligent Texture */
ENGINE_DLL HRESULT	Create_TextureRGBA8(Device* pDevice, uint32 iWidth, uint32 iHeight, const void* pPixels, const char* pName, Diligent::ITexture** ppTexture);

NS_END
