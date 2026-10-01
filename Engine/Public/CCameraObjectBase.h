#pragma once
#include "CGameObject.h"

namespace Engine
{
	class CCameraComponent;
	class CCameraObjectInfoWindow;
}


NS_BEGIN(Engine)


class ENGINE_DLL CCameraObjectBase abstract :   public CGameObject
{
	friend CCameraObjectInfoWindow;
public:
	/* 아마 기본적으로 Render Target View / Depth Stencil View  는 Graphic Device에서 가져오게 될 예정 */
	/* 흠... */

	struct CameraObject_Desc : public GameObject_Desc
	{
		/* 카메라를 사용해서 어디에다가 그릴지는 카메라 감독의 마음 */
		ComPtr<RTV>								CameraObjectBase_pRTVCom{nullptr};
		ComPtr<DepthStencilView>			CameraObjectBase_pDepthStencilViewCom{ nullptr };

		/* 카메라 실행 촬영 순서, 카메라 Update 순서 */
		CAMERA_UPDATE_DEPTH						CameraObjectBase_eCameraUpdateDepth = CAMERA_UPDATE_DEPTH::END;
		CAMERA_RENDER_DEPTH						CameraObjectBase_eCameraRenderDepth = CAMERA_RENDER_DEPTH::END;
		/* Update / Render 설정 */
		bool									CameraObjectBase_isUpdateActive = true;
		bool									CameraObjectBase_isRenderActive = true;

		/* 원래 컴포넌트에 있었던거 */
		PROJECTION_TYPE							CameraObjectBase_eProjectonType = PROJECTION_TYPE::ORTHOGRAPHIC;

		/* 카메라 시작 위치 */
		float									CameraObjectBase_fScreenStartX{ 0.f };
		float									CameraObjectBase_fScreenStartY{ 0.f };

		/* 카메라의 위치롭 부터 촬영할 크기 */
		float									CameraObjectBase_fFilmingSizeY{ 0.f };
		float									CameraObjectBase_fFilmingSizeX{ 0.f };

		/* 카메라 각도 찍힐 범위 */
		float									CameraObjectBase_fFov{ 0.f };
		float									CameraObjectBase_fNear{ 0.f };
		float									CameraObjectBase_fFar{ 0.f };

		Float2									CameraObjectBase_RenderAreaXZ{ 0.f,0.f };
	};

public:
	CONST VIEWPORT_DESC&						Get_CamViewPort() { return m_tInfo.tViewPortDesc; }
protected:
	explicit									CCameraObjectBase(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual										~CCameraObjectBase() {}
protected:

	/* 카메라 초기화 진행 함수 */
	virtual HRESULT								Camera_Initialize(void* pArg);
	HRESULT										Ready_Component(void* pArg) override;

public:
	/* GPU 에 세팅할 RTV / ViewPort 함수  */
	HRESULT										Set_GPU_ViewPort();
	HRESULT										Set_GPU_RTV();
	void										Set_CameraInfo(const CAM_INFO& tInfo) { m_tInfo = tInfo; }
	HRESULT										Set_GPU_Camera();
public:
	HRESULT										Set_ScissorRects();
	virtual HRESULT								Set_GPU_BeforeRender();
	virtual HRESULT								Set_PipeLineTransform();
public:
	/* Get */
	CONST CAMERA_UPDATE_DEPTH&					Get_CameraUpdateDepth()		const { return m_eCameraUpdateDepth;}
	CONST CAMERA_RENDER_DEPTH&					Get_CameraRenderDepth()		const { return m_eCameraRenderDepth;}
	CONST bool&									Get_IsDefaultCamera()		const { return m_isDefaultCamera; }

	Float4									    Get_CameraRenderAreaXY_MinMax();
	Float2										Get_CameraRenderAreaXZ()	const { return m_vRenderAreaXZ; }
	const CAM_INFO&								Get_CameraInfo()			const { return m_tInfo; }

	HRESULT										Update_CameraMatrix();

	/* A-plan : index written into the G-Buffer so the light pass can tell owners apart */
	void										Set_CamIndex(uint32 iIndex) { m_iCamIndex = iIndex; }
	uint32									Get_CamIndex() const { return m_iCamIndex; }

public:
	/* Set */
	void										Set_CameraUpdateDepth(CAMERA_UPDATE_DEPTH eUpdateDepth) { m_eCameraUpdateDepth = eUpdateDepth; }
	void										Set_CameraRenderDepth(CAMERA_RENDER_DEPTH eRenderDepth) { m_eCameraRenderDepth = eRenderDepth; }
	void										Set_Camera_UpdateRender_Depth(CAMERA_UPDATE_DEPTH eUpdateDepth, CAMERA_RENDER_DEPTH eRenderDepth) { m_eCameraUpdateDepth = eUpdateDepth, m_eCameraRenderDepth = eRenderDepth; }

public:
	virtual HRESULT								Add_RenderGroup()override;
	virtual HRESULT								Render();
public:
#ifdef _DEBUG
	void										ImGuiUpdate() override;
	void										ImGuiDebugRender();
#endif

protected:
	Float2										m_vRenderAreaXZ;
protected:
	bool										m_isDefaultCamera;
	PROJECTION_TYPE								m_eProjectionType;
protected:
	/* 카메라 각자의 RTV를 가지고 있다 Default 값으론 Device에서 가져온 RTV를 넣어주면된다 */
	ComPtr<RTV>									m_pRTVCom;
	ComPtr<DepthStencilView>					m_pDepthStencilViewCom;

protected:
	uint32										m_iCamIndex = 0;
	CAM_INFO									m_tInfo;
protected:
	CAMERA_UPDATE_DEPTH							m_eCameraUpdateDepth;
	CAMERA_RENDER_DEPTH							m_eCameraRenderDepth;
public:
	virtual	void		    Free() override;
};

NS_END
