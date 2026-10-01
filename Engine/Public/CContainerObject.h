#pragma once
#include "CGameObject.h"

namespace Engine
{
	class CPartObject;
}

NS_BEGIN(Engine)

class ENGINE_DLL CContainerObject abstract : public CGameObject
{
protected:
	typedef struct ContainerObject_Desc : public GAMEOBJECT_DESC
	{
	}CONTAINEROBJECT_DESC;
protected:
	explicit CContainerObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CContainerObject(const CContainerObject& Proto);
	virtual ~CContainerObject() {}
protected:
	HRESULT				Proto_Initialize();
	HRESULT				Clone_Initialize(void* pArg)override;
	HRESULT				Ready_Component(void* pArg)override;
	virtual HRESULT		Ready_PartObject();

public:
	virtual void		Pre_Update(float DT)						override;
	virtual void		Update(float DT)							override;
	virtual void		Late_Update(float DT)						override;
	virtual HRESULT		Add_RenderGroup()							override;
	virtual HRESULT		Render(FD CCameraObjectBase* pRenderCam)	override;
	virtual void		Set_Dead()									override;

#ifdef _DEBUG

	virtual void		ImGuiUpdate()override;

#endif 

public:
	HRESULT				Add_PartObject( uint32 iFindSceneID, const wstring& wstrFindProtoKey,const wstring& wstrAddKey , void* pArg  ,CPartObject** ppPartObject );
	CPartObject*		Find_PartObject(const wstring& wstrFindKey);
protected:
	map< const wstring, CPartObject*>			m_mapPartObject;

public:
	virtual void Free();
};

NS_END

