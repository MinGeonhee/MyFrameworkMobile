#pragma once
#include "CVIBuffer.h"

NS_BEGIN(Engine)


class ENGINE_DLL CVIBuffer_Rect final :  public CVIBuffer
{
private:
	explicit CVIBuffer_Rect(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Rect(const CVIBuffer_Rect& ProtoType);
	virtual	 ~CVIBuffer_Rect() {}
private:
	HRESULT	Proto_Initialize();
	virtual HRESULT	Clone_Initialize(void* pArg) override;

public:
	static	CVIBuffer_Rect*	Create(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void Free() override;

	// CVIBuffer을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC*  GetInputElementDesc() override;

	// CVIBuffer을(를) 통해 상속됨
	uint32 GetInputElementDescSize() override;
};

NS_END