#pragma once
#include "CVIBuffer.h"


NS_BEGIN(Engine)


class ENGINE_DLL CVIBuffer_Mesh final :public CVIBuffer
{
private:
	explicit CVIBuffer_Mesh(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Mesh(const CVIBuffer_Mesh& ProtoType);
	virtual	 ~CVIBuffer_Mesh() {}
private:
	HRESULT	Proto_Initialize();
			HRESULT	Proto_Initialize_Anim(const VI_INFO& tVIInfo);
			HRESULT	Proto_Initialize_NonAnim(const VI_INFO& tVIInfo, const XMatrix_1& matTransform );
	virtual HRESULT	Clone_Initialize(void* pArg) override;

private:
	HRESULT	Ready_AnimBuffer(const string& VertexPath, const string& IndexPath);
	HRESULT Ready_NonAnimBuffer(const string& VertexPath, const string& IndexPath, const XMatrix_1& matTransform);
public:
	static	CVIBuffer_Mesh* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	static	CVIBuffer_Mesh* Create_Anim(const VI_INFO& tVIInfo,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	static	CVIBuffer_Mesh* Create_NonAnim(const VI_INFO& tVIInfo, const XMatrix_1& matTransform,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);

	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

	// CVIBuffer을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC* GetInputElementDesc() override;

	// CVIBuffer을(를) 통해 상속됨
	uint32 GetInputElementDescSize() override;
};

NS_END