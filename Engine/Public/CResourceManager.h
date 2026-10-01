#pragma once
#include "CBase.h"

namespace Engine
{
	class CGameManager;
	class CResource;
}


NS_BEGIN(Engine)

class ENGINE_DLL CResourceManager : public CBase
{
private:
	explicit CResourceManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CResourceManager() {}

private:
	CResource* Find_Resrouce(const wstring& wstrKey, RESOURCE_TYPE eResourceType);

	HRESULT		Ready_Default_Shader();
	HRESULT		Ready_Default_Texture();
	HRESULT		Ready_Default_Material();
	HRESULT		Ready_Default_Model();

public:
	HRESULT		Ready_DefaultResource();




	CResource* Get_Resource(const wstring& wstrKey, RESOURCE_TYPE eResourceType, bool isCopy, void* pArg = nullptr);
	HRESULT    Add_Resource(const wstring& wstrKey, CResource* pResource);
private:
	array<unordered_map<wstring, CResource*>, Cast_Uint(RESOURCE_TYPE::END)>	m_arrayResourceUMap;
private:
	CGameManager*			m_pGameManager;

	ComPtr<Device>		   m_pDevice;
	ComPtr<DeviceContext>  m_pDeviceContext;
public:
	static	CResourceManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void Free();
};

NS_END

