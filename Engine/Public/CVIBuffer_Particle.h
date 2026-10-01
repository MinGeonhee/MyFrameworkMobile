#pragma once
#include "CVIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Particle abstract : public CVIBuffer
{
protected:
	typedef struct VIBuffer_Particle_Desc
	{
		uint32		iNumInstance		= {};
		Float2		vSize				= {};
		Float3		vCenter				= {};
		Float3		vPivot				= {};
		Float3		vRange				= {};
		Float2		vLifeTime			= {};
		Float2		vSpeed				= {};
		bool		isLoop				= {};

		uint32		iTotalFrameCount    = {NONE_USE_TEXTURE_ANIM};
		bool		isTextureAnimLoop	= { false };
		Float2		vTextureAnimSpeed   = {};

	}VIBUFFER_PARTICLE_DESC;
protected:
	explicit CVIBuffer_Particle(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CVIBuffer_Particle(const CVIBuffer_Particle& Proto);
	virtual ~CVIBuffer_Particle(){}
public:
	virtual HRESULT	Bind_Resource();
	virtual HRESULT	Draw();
protected:
	HRESULT			 Proto_Initialize();
	virtual  HRESULT Clone_Initialize(void* pArg)override;
public:
	/* Particle 관련 함수 */

	void			 Drop(float DT);
	void			 Spread(float DT);
	void			 Hover(float DT);
	void			 Gravity(float DT);
public:
	bool				Get_IsEnd() const { return m_isEnd; }
	void				RePlace_Particle();
protected:
	bool				 m_isEnd;
	bool				 m_isLoop;
	float*				 m_fSpeeds;

	Float3				 m_vPivot;
	Float2				 m_vSize;
	Float3				 m_vCenter;
	Float3				 m_vRange;
	Float2				 m_vSpeed;
	Float2				 m_vLifeTime;
	Float2				 m_vTextureAnimSpeed;

	uint32				 m_iTotalFrameCount;
	bool				 m_isTextureAnimLoop;
	float*				 m_fTextureAnimSpeeds;

protected:
	/* 인스턴스 버퍼를 위한 Buffer Com객체 */
	ComPtr<VertexBuffer> m_pVBInstanceParticle;
	/* 인스턴스 버퍼 구조체를 담고이을 구조체 */
	BUFFER_DESC    m_tInstanceParticleVerticleBufferDesc;
protected:
	/* Particle Instance 데이터 구조체 */
	VTX_INSTANCE_PARTICLE* m_pInstanceParticleVertices;
	uint32				   m_iIndexCountPerInstanceParticle;
	uint32				   m_iNumInstanceParticle;
	uint32				   m_iInstanceParticleVertexStride;
public:
	virtual void Free()override;
};

NS_END