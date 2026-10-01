#pragma once
#include "CBase.h"
#include <DirectXMath.h>

namespace Engine
{
	template<class T>
	class CConstantBuffer;

	class CShader;
	class CGameManager;
}

NS_BEGIN(Engine)

class CPipeLine :    public CBase
{
private:
	explicit CPipeLine(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual ~CPipeLine() {}
private:
	HRESULT			Initialize();
	HRESULT			Link_LightGroupBuffer		(CShader* pShader);
	HRESULT			Link_CameraBuffer			(CShader* pShader);
	HRESULT			Link_MouseBuffer			(CShader* pShader);
public:
	/* 찾을 카메라 번호라고 생각하면된다 */
	HRESULT			Set_PipeLineMatrix(CONST std::wstring& wstrFindKey , D3DTS eTransformMatrix , const XMatrix_1& TransformMatrix);
	HRESULT			Set_PipeLineMatrix(CONST std::wstring& wstrFindKey , D3DTS eTransformMatrix , const Float4x4& TransformMatrix);

	Float4x4 		Get_PipeLineMatrix(CONST std::wstring& wstrFindKey, D3DTS eTransformMatrix , bool isInverse) CONST;
	const Float4x4* Get_PipeLineMatrixPtr(CONST std::wstring& wstrFindKey, D3DTS eTransformMatrix , bool isInverse) CONST;
	Float4			Get_CameraPosition(CONST std::wstring& wstrFindKey) CONST;

	CONST	void	Clear_PipeLine();
public:
	HRESULT			Bind_PipeLine();

	HRESULT			Link_EffectConstantBuffer(GLOBAL_CB_TYPE eCBType, CShader* pShader);
public:
	HRESULT			Set_GPU_CameraBuffer(const wstring& wstrFindKey , const CAM_SHADER_INFO& tCamInfo);
	HRESULT			Set_GPU_LightGroupBuffer( const LIGHT_GROUP_BUF& tLightGroupBuf);
	HRESULT			Set_GPU_LightGroupBuffer();
	HRESULT			Set_GPU_MouseBuffer(const MOUSE_BUF_DATA& tBuffer);
public:
	void	Update(float DT);
	void	Late_Update(float DT);
private:

	/* 카메라가 업데이트 돌고나서의 뷰 , 투영 , 카메라 위치값에 대한 정보를 저장시켜 놓는다. */
	unordered_map<wstring, array <Float4x4 , Cast_Uint(D3DTS::END)>>		m_umapTransformMatrices;
	/* 들고나서 나온 행렬의 역행렬들을 미리 구해놓는다 */
	unordered_map<wstring, array <Float4x4, Cast_Uint(D3DTS::END)>>			m_umapTransformInverseMatrices;
	unordered_map<wstring, Float4 >											m_umapCameraPosition;
private:
	/* Camera_Buffer_Data */
	CConstantBuffer<CAM_BUF_DATA>*			m_pCameraCB;
	CameraBufferData						m_tCameraCBData;

	/* Light Group Buffer Data */
	CConstantBuffer<LIGHT_GROUP_BUF>*		m_pLightGroupCB;
	LIGHT_GROUP_BUF							m_tLightGroupData;

	/* Mouse Buffer Data */
	CConstantBuffer<MOUSE_BUF_DATA>*		m_pMouseCB;
	MOUSE_BUF_DATA							m_tMouseCBData;
public:
	CGameManager*							m_pGameManager;

	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;

public:
	static CPipeLine* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual  void Free();
};

NS_END