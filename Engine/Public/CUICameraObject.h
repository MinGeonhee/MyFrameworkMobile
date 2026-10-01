#pragma once
#include "CCameraObjectBase.h"

NS_BEGIN(Engine)

class ENGINE_DLL CUICameraObject final:    public CCameraObjectBase
{
public:
	typedef struct UICameraObject_Desc final : public CCameraObjectBase::CameraObject_Desc
	{
		/* UI는 직교투영을 사용해아하는 렌더그룹만 렌더하게 된다.. */
		vector<uint32>					vecRenderGroupID;
		UPDATE_MODE						eUpdateMode = UPDATE_MODE::STATIC;
	} UICAM_DESC;

protected:
	explicit				CUICameraObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual					~CUICameraObject() {}
protected:
	virtual HRESULT			Camera_Initialize(void* pArg);
	HRESULT					Add_Component(void* pArg);
public:
public:
	void						Setting_UICamera();
public:
	virtual void				Pre_Update(float DT)	override;
	virtual void				Update(float DT)		override;
	virtual void				Late_Update(float DT)	override;
public:
	vector<uint32>				m_vecRenderGroupID;
public:
	static	CUICameraObject*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, void* pArg);
	virtual CUICameraObject*	Clone(void* pArg);
	virtual	void				Free() override;


	// CCameraObjectBase을(를) 통해 상속됨
	void Add_ColliderGroup() override;

	// CCameraObjectBase을(를) 통해 상속됨
};


NS_END