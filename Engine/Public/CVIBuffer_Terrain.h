#pragma once
#include "CVIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final:    public CVIBuffer
{
private:
	explicit CVIBuffer_Terrain(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Terrain(const CVIBuffer_Terrain& ProtoType);
	virtual	 ~CVIBuffer_Terrain() {}
private:
	HRESULT	Proto_Initialize(CONST std::wstring& wstrPath);
	virtual HRESULT	Clone_Initialize(void* pArg) override;

private:
	int64			m_iCountVerticesX;
	int64			m_iCountVerticesZ;
public:
	static	CVIBuffer_Terrain* Create(const std::wstring& wstrPath ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

	// CVIBuffer을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC* GetInputElementDesc() override;

	// CVIBuffer을(를) 통해 상속됨
	uint32 GetInputElementDescSize() override;
};

NS_END