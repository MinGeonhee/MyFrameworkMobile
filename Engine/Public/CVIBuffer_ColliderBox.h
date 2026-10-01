#pragma once
#include "CVIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_ColliderBox :public CVIBuffer
{
public:
	explicit		CVIBuffer_ColliderBox(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual			~CVIBuffer_ColliderBox();
private:
	virtual HRESULT	Proto_Initialize() override;
	virtual HRESULT	Clone_Initialize(void* pArg) override;
public:
	static	CVIBuffer_ColliderBox* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual	void Free();

	// CVIBuffer을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC* GetInputElementDesc() override;
	uint32 GetInputElementDescSize() override;
};

NS_END

