#pragma once
#include "CVIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Cell final: public CVIBuffer
{
public:
	explicit CVIBuffer_Cell(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Cell(const CVIBuffer_Cell& Proto);
	virtual ~CVIBuffer_Cell() {}
private:
	HRESULT			Proto_Initialize(const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)]);
	virtual HRESULT	Clone_Initialize(void* pArg) override;




public:
	virtual HRESULT							Bind_Resource() override;
	virtual HRESULT							Draw()override;

	void									Change_Point();
	void									Change_Line();
	void									Change_Tri();


	HRESULT			SubResourceUpdate(const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)]);

public:
	static	CVIBuffer_Cell* Create( const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)], ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free();

public:
	// CVIBuffer을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC* GetInputElementDesc() override;

	// CVIBuffer을(를) 통해 상속됨
	uint32 GetInputElementDescSize() override;

	// CVIBuffer을(를) 통해 상속됨
	CComponent* Clone(void* pArg) override;
};

NS_END


