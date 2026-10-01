#pragma once
#include "CVIBuffer_Particle.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Particle_Point final : public CVIBuffer_Particle
{
public:
	typedef struct VIBuffer_Particle_Point_Desc : public CVIBuffer_Particle::VIBUFFER_PARTICLE_DESC
	{

	}VIBUFFER_PARTICLE_POINT_DESC;
private:
	explicit CVIBuffer_Particle_Point(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Particle_Point(const CVIBuffer_Particle_Point& Proto);
	virtual ~CVIBuffer_Particle_Point() {}
private:
	HRESULT			Proto_Initialize(const VIBUFFER_PARTICLE_POINT_DESC* pInitialDesc);
	virtual HRESULT	Clone_Initialize(void* pArg);
public:
	virtual HRESULT	Bind_Resource()override;
	virtual HRESULT Draw()override;
public:
	static	CVIBuffer_Particle_Point* Create(const VIBUFFER_PARTICLE_POINT_DESC* pInitialDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CComponent*				  Clone(void* pArg) override;
	virtual void					  Free()override;

	// CVIBuffer_Particle을(를) 통해 상속됨
	const INPUT_ELEMENT_DESC* GetInputElementDesc() override;
	uint32 GetInputElementDescSize() override;
};

NS_END

