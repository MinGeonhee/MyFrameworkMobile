#include "CVIBuffer_Particle.h"
#include "CGameManager.h"

CVIBuffer_Particle::CVIBuffer_Particle(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CVIBuffer(pDevice,pContext), m_pVBInstanceParticle(nullptr), m_tInstanceParticleVerticleBufferDesc{}, m_pInstanceParticleVertices(nullptr)
	, m_iIndexCountPerInstanceParticle(0),m_iNumInstanceParticle(0), m_iInstanceParticleVertexStride(0)
	, m_fSpeeds(nullptr), m_isLoop(false), m_vPivot{}
	, m_isTextureAnimLoop{false}
	, m_iTotalFrameCount{ NONE_USE_TEXTURE_ANIM }
	, m_fTextureAnimSpeeds(nullptr)
	, m_isEnd(false)
	, m_vSize{0.f,0.f}
	, m_vCenter{0.f,0.f,0.f}
	, m_vRange{0.f,0.f,0.f}
	, m_vLifeTime{0.f,0.f}
	, m_vSpeed{0.f,0.f}
	, m_vTextureAnimSpeed{0.f,0.f}
{ 

}	 

CVIBuffer_Particle::CVIBuffer_Particle(const CVIBuffer_Particle& Proto)
	: CVIBuffer(Proto), m_pVBInstanceParticle(Proto.m_pVBInstanceParticle), m_tInstanceParticleVerticleBufferDesc(Proto.m_tInstanceParticleVerticleBufferDesc)
	,m_pInstanceParticleVertices(Proto.m_pInstanceParticleVertices),m_iIndexCountPerInstanceParticle(Proto.m_iIndexCountPerInstanceParticle)
	, m_iNumInstanceParticle(Proto.m_iNumInstanceParticle), m_iInstanceParticleVertexStride(Proto.m_iInstanceParticleVertexStride), m_vPivot{Proto.m_vPivot}
	, m_fSpeeds(Proto.m_fSpeeds), m_isLoop(Proto.m_isLoop)
	, m_isTextureAnimLoop{   Proto.m_isTextureAnimLoop }
	, m_iTotalFrameCount{ Proto.m_iTotalFrameCount }
	, m_fTextureAnimSpeeds{ Proto.m_fTextureAnimSpeeds }
	, m_isEnd(Proto.m_isEnd)
	, m_vSize	 { Proto.m_vSize }
	, m_vCenter	 { Proto.m_vCenter }
	, m_vRange	 { Proto.m_vRange }
	, m_vLifeTime{ Proto.m_vLifeTime }
	, m_vSpeed	 { Proto.m_vSpeed }
	, m_vTextureAnimSpeed{Proto.m_vTextureAnimSpeed }
{
}

HRESULT CVIBuffer_Particle::Proto_Initialize()
{
	return S_OK;
}

HRESULT CVIBuffer_Particle::Clone_Initialize(void* pArg)
{
	return S_OK;
}

void CVIBuffer_Particle::Drop(float DT)
{
	MAPPED_SUBRESOURCE SubResource{};

	if (FAILED(Map(m_pVBInstanceParticle.Get(), 0, MAP_MODE_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return;

	VTX_INSTANCE_PARTICLE* pParticleInstance = static_cast<VTX_INSTANCE_PARTICLE*>(SubResource.pData);
	
	for (uint32 i = 0; i < m_iNumInstanceParticle; ++i)
	{
		/* 내 속도 만큼 아래로 내려준다 */
		pParticleInstance[i].vTranslation.y -= DT * m_fSpeeds[i];
		pParticleInstance[i].vLifeTime.x += DT;

		/* Acc 값이 누적된 값보다 크고 Loop라면? y 값을 최초의 Y 값으로 다시 올려준다. 그리고 Life Time 을 다시 0초로   */
		if (pParticleInstance[i].vLifeTime.x > pParticleInstance[i].vLifeTime.y && m_isLoop == true)
		{
			pParticleInstance[i].vTranslation.y = m_pInstanceParticleVertices[i].vTranslation.y;
			pParticleInstance[i].vLifeTime.x = 0.f;
		}
	}

	Unmap(m_pVBInstanceParticle.Get(), 0);
}

void CVIBuffer_Particle::Spread(float DT)
{
	MAPPED_SUBRESOURCE SubResource{};

	if (FAILED(Map(m_pVBInstanceParticle.Get(), 0, MAP_MODE_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return;

	VTX_INSTANCE_PARTICLE* pVertex = static_cast<VTX_INSTANCE_PARTICLE*>(SubResource.pData);


	for (uint32 i = 0; i < m_iNumInstanceParticle; ++i)
	{
		Vector4 vDir = XMVectorSetW(XMVector3Normalize(XMLoadFloat4(&pVertex[i].vTranslation) - XMLoadFloat3(&m_vPivot)),0.f);

		XMStoreFloat4(&pVertex[i].vTranslation , XMVectorAdd( XMLoadFloat4(&pVertex[i].vTranslation) , vDir * m_fSpeeds[i] * DT ));

		pVertex[i].vLifeTime.x += DT;

		if (pVertex[i].vLifeTime.x > pVertex[i].vLifeTime.y && m_isLoop == true)
		{
			pVertex[i].vTranslation = m_pInstanceParticleVertices[i].vTranslation;
			pVertex[i].vLifeTime.x = 0.f;
			m_isEnd = true;
		}

		if (m_iTotalFrameCount > NONE_USE_TEXTURE_ANIM)
		{
			float fTotalProgress = pVertex[i].vLifeTime.x * m_fTextureAnimSpeeds[i];
			if (m_isTextureAnimLoop)
				pVertex[i].iTextureFrame = (uint32)fmod(fTotalProgress, (float)m_iTotalFrameCount);
			else
			{
				if (fTotalProgress >= m_iTotalFrameCount)
				{
					pVertex[i].iTextureFrame = m_iTotalFrameCount - 1;
					m_isEnd = true;
				}
				else
					pVertex[i].iTextureFrame = (uint32)fTotalProgress;
			}
		}
	}

	Unmap(m_pVBInstanceParticle.Get(),0 );

	return;

}

void CVIBuffer_Particle::Hover(float DT)
{
	/* Life Time 이용해서 위아래로 올렸다가 내렸다가 */
	MAPPED_SUBRESOURCE SubResource{};

	if (FAILED(Map(m_pVBInstanceParticle.Get(), 0, MAP_MODE_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return;

	VTX_INSTANCE_PARTICLE* pVertex = static_cast<VTX_INSTANCE_PARTICLE*>(SubResource.pData);

	/* 입자들이 LifeTime을가지고 사라지는게아니라 LifeTime값을이용해서 위아래로 왔다갔다 */
	/* 해야하는데 이게 가능해? */

	for (uint32 i = 0; i < m_iNumInstanceParticle; ++i)
	{
		pVertex[i].vLifeTime.x += DT;

		if (pVertex[i].vLifeTime.x > pVertex[i].vLifeTime.y && m_isLoop == true)
		{
			pVertex[i].vTranslation = m_pInstanceParticleVertices[i].vTranslation;
			pVertex[i].vLifeTime.x = 0.f;
		}
	}

	Unmap(m_pVBInstanceParticle.Get(), 0);
}


void CVIBuffer_Particle::Gravity(float DT)
{

}


void CVIBuffer_Particle::RePlace_Particle()
{
	/* Life Time 이용해서 위아래로 올렸다가 내렸다가 */
	MAPPED_SUBRESOURCE SubResource{};

	
	if (FAILED(Map(m_pVBInstanceParticle.Get(), 0, MAP_MODE_WRITE_NO_OVERWRITE, 0, &SubResource)))
		return;

	VTX_INSTANCE_PARTICLE* pVertex = static_cast<VTX_INSTANCE_PARTICLE*>(SubResource.pData);

	/* 입자들이 LifeTime을가지고 사라지는게아니라 LifeTime값을이용해서 위아래로 왔다갔다 */
	/* 해야하는데 이게 가능해? */

	CGameManager* pMgr = CGameManager::GetInstance();

	for (uint32 i = 0; i < m_iNumInstanceParticle; ++i)
	{
		float fScale = pMgr->Get_Randomfloat(m_vSize.x , m_vSize.y);

		m_pInstanceParticleVertices[i].vRight = { fScale,0.f,0.f,0.f };
		m_pInstanceParticleVertices[i].vUp = { 0.f,fScale,0.f,0.f };
		m_pInstanceParticleVertices[i].vLook = { 0.f,0.f,fScale,0.f };

		m_pInstanceParticleVertices[i].vTranslation = Float4(
			pMgr->Get_Randomfloat(m_vCenter.x - m_vRange.x * 0.5f, m_vCenter.x + m_vRange.x* 0.5f),
			pMgr->Get_Randomfloat(m_vCenter.y - m_vRange.y * 0.5f, m_vCenter.y + m_vRange.y* 0.5f),
			pMgr->Get_Randomfloat(m_vCenter.z - m_vRange.z * 0.5f, m_vCenter.z + m_vRange.z* 0.5f),
			1.f
		);

		m_pInstanceParticleVertices[i].vLifeTime = { 0.f , pMgr->Get_Randomfloat(m_vLifeTime.x,m_vLifeTime.y) };
		m_fSpeeds[i] = pMgr->Get_Randomfloat(m_vSpeed.x, m_vSpeed.y);

		m_pInstanceParticleVertices[i].iTextureFrame = 0;

		if (m_iTotalFrameCount != NONE_USE_TEXTURE_ANIM)
			m_fTextureAnimSpeeds[i] = pMgr->Get_Randomfloat(m_vTextureAnimSpeed.x, m_vTextureAnimSpeed.y);
	}

	Unmap(m_pVBInstanceParticle.Get(), 0);
}


HRESULT	CVIBuffer_Particle::Bind_Resource()
{
	Buffer* pVBs[] =
	{
		m_pVertexBuffer.Get(),m_pVBInstanceParticle.Get()
	};

	uint32		  iVertexStride[] =
	{
		m_iStrideVertex, m_iInstanceParticleVertexStride
	};
	
	uint32		  iOffset[] =
	{
		0,0
	};

	IASetVertexBuffers(0,m_iCountVertexBuffer, pVBs ,iVertexStride , iOffset );
	IASetIndexBuffer(m_pIndexBuffer.Get() , m_iStrideIndex == 2 ? INDEX_TYPE_UINT16 : INDEX_TYPE_UINT32 , 0);
	IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT	CVIBuffer_Particle::Draw()
{
	Context_DrawIndexedInstanced(m_iIndexCountPerInstanceParticle,m_iNumInstanceParticle , 0 , 0 ,0);

	return S_OK;
}

void CVIBuffer_Particle::Free()
{
	__super::Free();

	/* Clone 마다 만든 Dynamic 인스턴스 버퍼의 CPU 사본 해제 */
	if (m_eMakeType == MAKE_TYPE::CLONE && m_pVBInstanceParticle != nullptr && m_pVBInstanceParticle->GetReferenceCounters()->GetNumStrongRefs() == 1)
		CVIBuffer::Release_DynamicBuffer(m_pVBInstanceParticle.Get());
	m_pVBInstanceParticle.Reset();

	if (m_eMakeType == MAKE_TYPE::PROTOTYPE)
	{
		Safe_Delete_Arr(m_pInstanceParticleVertices);
		Safe_Delete_Arr(m_fSpeeds);
		Safe_Delete_Arr(m_fTextureAnimSpeeds);
	}
}
