#pragma once
#include "CBase.h"
#include "CLight.h"

namespace Engine
{
	class CLight;
	class CGameManager;
	class CLightInfoWindow;
	class CShader;
	class CCameraObjectBase;
	class CVIBuffer_Rect;
}

NS_BEGIN(Engine)

class CLightManager :public CBase
{

public:
	explicit CLightManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CLightManager() {}
public:
	HRESULT								Initialize();
public:
	const LIGHT_GROUP_BUF*				Get_LightGroupBuffer();
	HRESULT								Add_Light( const CLight::Light_Desc& tDesc);
	HRESULT								Add_Light(CLight* pLight);
	const Light_Buffer_Data&			Get_LightBufferData(LIGHT_TYPE eType , uint32 iIndex);
	CLight*								Get_Light(LIGHT_TYPE eType, uint32 iIndex);
	list<CLight*>						Get_LightList(LIGHT_TYPE eType) { return m_listLight[Cast_Uint(eType)]; }
	HRESULT								Set_GPU_LightGroupBuffer(CCameraObjectBase* pRenderCam);
	HRESULT								Clear_Light();
public:
	HRESULT								Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer,CCameraObjectBase* pRenderCam);
private:
	HRESULT								Render_DirectionalLight(CShader* pShader, CVIBuffer_Rect* pVIBuffer);
	HRESULT								Render_PointLight(CShader* pShader, CVIBuffer_Rect* pVIBuffer);
private:
	CLight*								Find_Light(LIGHT_TYPE eType, uint32 iIndex);
private:
	LIGHT_GROUP_BUF						m_tLightGroupBuffer;
	list<CLight*>						m_listLight[Cast_Uint(LIGHT_TYPE::END)];
private:
	CGameManager*						m_pGameManager;
	ComPtr<Device>						m_pDevice;
	ComPtr<DeviceContext>				m_pDeviceContext;
public:
	static CLightManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void	Free()override;

public:
	friend class CLightInfoWindow;
};

NS_END
