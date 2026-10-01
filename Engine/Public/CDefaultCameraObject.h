#pragma once
#include "CCameraObjectBase.h"

namespace Engine
{
	class CCameraComponent;
	class CCameraObjectInfoWindow;
}


NS_BEGIN(Engine)

/* 가볍게 Debug 용 Default Camera 만들기 */

class ENGINE_DLL CDefaultCameraObject final: public CCameraObjectBase
{
	friend CCameraObjectInfoWindow;
public:
	/* final인 Desc 에는 이름 소속을 붙일 필요가 없어보이긴해 */
	struct DefaultCameraObject_Dest : public CCameraObjectBase::CameraObject_Desc
	{
		/* 움직임 타입 , 카메라 타입  */
		MOVE_TYPE					eMoveType = MOVE_TYPE::END;
		CAMERA_TYPE					eCameraType = CAMERA_TYPE::END;
		UPDATE_MODE					eUpdateMode = UPDATE_MODE::DYNAMIC;

		Float3						tOffsetFromTarget{};
		CGameObject*				pActor{ nullptr };
	};
protected:
	explicit						CDefaultCameraObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual							~CDefaultCameraObject() {}
protected:
	virtual HRESULT					Camera_Initialize(void* pArg);
	HRESULT							Ready_Component(void* pArg);
#ifdef _DEBUG
	virtual void					ImGuiUpdate()override;
#endif
public:

	const CAMERA_TYPE&				Get_CameraType() const { return m_eCameraType;}	
	const MOVE_TYPE&				Get_MoveType() const { return m_eMoveType; }

public:
	void							Set_CameraType(CAMERA_TYPE eType);
	void							Set_DefaultLookVec();
private:
	void							FirstPerson_Update(float DT);
	void							ThirdPerson_Update(float DT);
	void							Free_Update(float DT);
	void							Fixed_Update(float DT);
private:
	virtual HRESULT					Set_PipeLineTransform();
	HRESULT							Setting_FirstPersonType();
	HRESULT							Setting_ThirdPersonType();
	HRESULT							Setting_FreeType();
	HRESULT							Setting_FixedType();
public:
	virtual void					Pre_Update(float DT)	override;
	virtual void					Update(float DT)			override;
	virtual void					Late_Update(float DT)	override;
	virtual HRESULT					Add_RenderGroup()		override;
public:
	void							Free_KeyUpdaate(float DT);
protected:
	MOVE_TYPE						m_eMoveType;
	CAMERA_TYPE						m_eCameraType;
	Float3							m_tDefualtLookVec;
	Float3							m_tOffsetFromTarget;
	bool							m_isFixedMouse;

	/* 카메라가 3인칭 모드일떄 카메라가 바라보는 방향을 고정시켜줘야한다 */
	Float4							m_tDefaultLook;

protected:
	vector<uint32>					m_vecRenderGroupID;
public:
	static		CDefaultCameraObject*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext , void* pArg);
	virtual		CDefaultCameraObject*	Clone(void* pArg);
	virtual	void				Free() override;

	// CCameraObjectBase을(를) 통해 상속됨
	void Add_ColliderGroup() override;
};

NS_END