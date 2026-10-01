#pragma once
#include "CBase.h"


/* Resource 에 어떻게 특별하게해서 복사해서 생성하고싶다? */
/* 모델이 바뀔 이유가 있을까? */
/* Resource를 복사생성해서 들고있기보단 리소스는 하나만 존재하고 */
/* 그걸 복사하는 형태가 가장 이상적인 느낌인거같은데.. */
/* Model은 그런면에서 엄청 애매한 그 위치에 있는 느낌... */


/* Resource를 복제해서 가져갈지 아니면 그냥 원본으로 가져갈지 선택이 가능하게만들어주면 좋을 것 같다 */

namespace Engine
{
	class CGameManager;
}

NS_BEGIN(Engine)

class ENGINE_DLL CResource abstract :public CBase
{
protected:
	explicit CResource(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CResource(const CResource& Proto);
	virtual ~CResource();
protected:
	virtual HRESULT					Proto_Initialize();
	virtual HRESULT					Copy_Initialize(void* pArg);
public:
	const MAKE_TYPE&		Get_MakeType() const  { return  m_eMakeType; }
	const RESOURCE_TYPE&	Get_ResourceType() const { return m_eResourceType; }

#ifdef _DEBUG
public:

	virtual void			ImGui_Update() { return; }

#endif // _DEBUG
public:
	MAKE_TYPE				m_eMakeType;
	RESOURCE_TYPE			m_eResourceType;
	
	char					m_szPath[MAX_PATH];
protected:
	CGameManager*			m_pGameManager;
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	virtual CResource* Copy_Resource(void* pArg) = 0;
	virtual void Free()override;
};

NS_END