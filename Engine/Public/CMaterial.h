#pragma once
#include "CResource.h"

namespace Engine
{
	class CTexture;
	class CGameManager;
}

NS_BEGIN(Engine)

class ENGINE_DLL CMaterial final :public CResource
{
private:
	explicit CMaterial(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CMaterial(const CMaterial& Proto);
	virtual ~CMaterial() {}
private:
	HRESULT	Proto_Initialize(const MTL_PATH& MtlPath, const wstring& wstrName);
	HRESULT	Copy_Initialize(void* pArg)override;

	HRESULT	Ready_Diffuse(const wstring& wstrDiffusePath);
	HRESULT	Ready_Ambient(const wstring& wstrAmbient);
	HRESULT	Ready_Normal(const wstring& wstrNormalPath);
	HRESULT	Ready_Specular(const wstring& wstrSpecularPath);
	HRESULT	Ready_Emissive(const wstring& wstrEmissivePath);
	HRESULT	Ready_Metal(const wstring& wstrMetalPath);

	HRESULT	Ready_DefaultWhiteColorTexture();


public:
	CONST wstring&						Get_Name() { return m_wstrName; }
	int32								Get_MateiralIndex(MATERIAL_TYPE eType);
public:

	HRESULT								Set_GPU_Material(class CShader* pShader);
	HRESULT								Set_Material_Index(MATERIAL_TYPE eType, uint32 _iIndex) {
		MAX_ENUM_CHECK_RETURN_EFAIL(MATERIAL_TYPE::END,eType);
		m_iIndex[Cast_Uint(eType)] = _iIndex;
		return S_OK;
	}

private:
	/* 안에있는 ShaderResource View만 Add_Ref 해주면된다. */
	static ShaderResourceView*			m_pSRVs[Cast_Uint(MATERIAL_TYPE::END)];
	ShaderResourceView*					m_pDefaultWhiteSRV;
	uint32										m_iIndex[Cast_Uint(MATERIAL_TYPE::END)];
	array < vector<ShaderResourceView*>,  Cast_Uint(MATERIAL_TYPE::END) > m_arrayMaterialTextures;
	wstring										m_wstrName;
public:
	static	CMaterial*		Create( const MTL_PATH& strMtlPath , const wstring& wstrName,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CResource*				Copy_Resource(void* pArg);
	virtual void			Free();
public:
};


NS_END
