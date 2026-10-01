#pragma once
#include "CBase.h"

namespace Engine
{
	class CCameraObjectBase;
}

NS_BEGIN(Engine)

class ENGINE_DLL CLight :public CBase
{

public:
	typedef struct Light_Desc
	{
		LIGHT_TYPE				eLightType;
		Light_Buffer_Data	tLightBufferData;
	}LIGHT_DESC;
public:
	explicit		CLight(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext);
	virtual		~CLight() {}
private:
	HRESULT									Light_Initialize(const LIGHT_DESC& tDesc);
public:
	const LIGHT_TYPE&						Get_LightType()const { return m_eLightType; }
	const Light_Buffer_Data&				Get_LightBuffer_Data() { return m_tLightBufferData; }
	const bool&								Get_IsActive()						const { return m_isActive;}
	const Float4&							Get_Position()  const { return  m_tLightBufferData.vPosition; }
	const float&							Get_Range()	 const { return  m_tLightBufferData.fRange; }
public:
	bool									Is_CameraInRange(const Float4& vRenderCamAreaXZ);
public:
	void									Set_Position(const fVector4_1& vPos) { XMStoreFloat4(&m_tLightBufferData.vPosition, vPos); }
	void									Set_Position(const Float3& vPos) { m_tLightBufferData.vPosition = Float4(vPos.x, vPos.y , vPos.z , 1.f); }
	void									Set_Range(float fRange) { m_tLightBufferData.fRange = fRange; }
	void									Set_IsActive(bool isAcitve) { m_isActive = isAcitve; }

public:
	HRESULT									Render(CCameraObjectBase* pRenderCam);
public:
#ifdef _DEBUG
	void									ImGuiUpdate();
#endif // _DEBUG


private:
	bool										m_isActive;
	LIGHT_TYPE									m_eLightType;
	Light_Buffer_Data							m_tLightBufferData;
public:
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>						m_pDeviceContext;
public:
	static CLight* Create(const LIGHT_DESC& tDesc,ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual void Free()override;
};

NS_END
