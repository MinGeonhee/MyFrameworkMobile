#include "CVIBuffer_Particle_Point.h"
#include "CGameManager.h"

CVIBuffer_Particle_Point::CVIBuffer_Particle_Point(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CVIBuffer_Particle(pDevice,pContext)
{
}

CVIBuffer_Particle_Point::CVIBuffer_Particle_Point(const CVIBuffer_Particle_Point& Proto)
    : CVIBuffer_Particle(Proto)
{

}

HRESULT	CVIBuffer_Particle_Point::Proto_Initialize(const VIBUFFER_PARTICLE_POINT_DESC* pInitialDesc)
{
	m_iStrideVertex = sizeof(VTXPOS);
	m_iCountVertex = 1;					
	m_iOffsetVertex = 0;							
	m_iInstanceParticleVertexStride = sizeof(VTX_INSTANCE_PARTICLE);
	m_iOffsetIndex = 0;							
	m_iCountVertexBuffer = 2;
	m_ePrimitiveType = TOPOLOGY_POINTLIST;

	VTXPOSTEX* pVertices = new VTXPOSTEX[m_iCountVertex];
	m_pVertexPosition = new Float3[m_iCountVertex];
	m_pVertexPosition[0] = pVertices[LT].vPosition = Float3(0.f,0.f,0.f);


	BUFFER_DESC		tVertexDesc{};

	tVertexDesc.ByteWidth = m_iStrideVertex * m_iCountVertex;
	tVertexDesc.StructureByteStride = m_iStrideVertex; 
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;	
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER; //
	tVertexDesc.CPUAccessFlags = 0;  
	tVertexDesc.MiscFlags = 0;	


	SUBRESOURCE_DATA	tVertexData{};
	tVertexData.pSysMem = pVertices;		// Vertex 배열의 시작 주소를 넣어준다.

	if (FAILED(Create_Buffer(tVertexDesc, &tVertexData, &m_pVertexBuffer)))
		return E_FAIL;


	delete[] pVertices;
	pVertices = nullptr;


	/* Instance Vuffer */

	m_iNumInstanceParticle	= pInitialDesc->iNumInstance;
	m_isLoop				= pInitialDesc->isLoop;
	m_vPivot				=  pInitialDesc->vPivot;
	m_iTotalFrameCount		= pInitialDesc->iTotalFrameCount;

	m_vCenter				= pInitialDesc->vCenter;
	m_vRange				= pInitialDesc->vRange;
	m_vLifeTime				= pInitialDesc->vLifeTime;
	m_vSpeed				= pInitialDesc->vSpeed;
	m_vSize					= pInitialDesc->vSize;
	m_vTextureAnimSpeed		= pInitialDesc->vTextureAnimSpeed;

	m_tInstanceParticleVerticleBufferDesc.ByteWidth = m_iInstanceParticleVertexStride * m_iNumInstanceParticle;
	m_tInstanceParticleVerticleBufferDesc.StructureByteStride = m_iInstanceParticleVertexStride;
	m_tInstanceParticleVerticleBufferDesc.Usage = BUFFER_USAGE_DYNAMIC;
	m_tInstanceParticleVerticleBufferDesc.CPUAccessFlags = BUFFER_CPU_ACCESS_WRITE;
	m_tInstanceParticleVerticleBufferDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
	m_tInstanceParticleVerticleBufferDesc.MiscFlags = 0;

	m_pInstanceParticleVertices = new VTX_INSTANCE_PARTICLE[m_iNumInstanceParticle];
	ZeroMemory(m_pInstanceParticleVertices, sizeof(VTX_INSTANCE_PARTICLE) * m_iNumInstanceParticle);


	m_fSpeeds = new float[m_iNumInstanceParticle];
	ZeroMemory(m_pInstanceParticleVertices, sizeof(float) * m_iNumInstanceParticle);

	if (m_iTotalFrameCount != NONE_USE_TEXTURE_ANIM)
		m_fTextureAnimSpeeds = new float[m_iNumInstanceParticle];


	CGameManager* pMgr = CGameManager::GetInstance();

	/* 네모공간에 내가지정한 Range = Extents 보면될듯? 이 크기안에서 랜덤한 위치를 잡는다*/

	for (uint32 i = 0; i < m_iNumInstanceParticle; ++i)
	{
		float fScale = pMgr->Get_Randomfloat(pInitialDesc->vSize.x, pInitialDesc->vSize.y);

		m_pInstanceParticleVertices[i].vRight	= {fScale,0.f,0.f,0.f};
		m_pInstanceParticleVertices[i].vUp		= {0.f,fScale,0.f,0.f};
		m_pInstanceParticleVertices[i].vLook	= {0.f,0.f,fScale,0.f};
		
		m_pInstanceParticleVertices[i].vTranslation = Float4(
			pMgr->Get_Randomfloat(pInitialDesc->vCenter.x - pInitialDesc->vRange.x * 0.5f, pInitialDesc->vCenter.x + pInitialDesc->vRange.x * 0.5f),
			pMgr->Get_Randomfloat(pInitialDesc->vCenter.y - pInitialDesc->vRange.y * 0.5f, pInitialDesc->vCenter.y + pInitialDesc->vRange.y * 0.5f),
			pMgr->Get_Randomfloat(pInitialDesc->vCenter.z - pInitialDesc->vRange.z * 0.5f, pInitialDesc->vCenter.z + pInitialDesc->vRange.z * 0.5f),
			1.f
		);

		m_pInstanceParticleVertices[i].vLifeTime =  { 0.f , pMgr->Get_Randomfloat(pInitialDesc->vLifeTime.x,pInitialDesc->vLifeTime.y)};
		m_fSpeeds[i] = pMgr->Get_Randomfloat(pInitialDesc->vSpeed.x, pInitialDesc->vSpeed.y);
		
		m_pInstanceParticleVertices[i].iTextureFrame = 0;

		if(m_iTotalFrameCount != NONE_USE_TEXTURE_ANIM)
			m_fTextureAnimSpeeds[i] = pMgr->Get_Randomfloat(pInitialDesc->vTextureAnimSpeed.x, pInitialDesc->vTextureAnimSpeed.y);
	}


	return S_OK;
}

HRESULT	CVIBuffer_Particle_Point::Clone_Initialize(void* pArg)
{
	SUBRESOURCE_DATA      InstanceInitialData{};
	InstanceInitialData.pSysMem = m_pInstanceParticleVertices;

	if (FAILED(Create_Buffer(m_tInstanceParticleVerticleBufferDesc, &InstanceInitialData, &m_pVBInstanceParticle)))
		return E_FAIL;


    return S_OK;
}


HRESULT	CVIBuffer_Particle_Point::Bind_Resource()
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

	IASetVertexBuffers(0, m_iCountVertexBuffer, pVBs, iVertexStride, iOffset);
	IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT	CVIBuffer_Particle_Point::Draw()
{
	Context_DrawInstanced(1,m_iNumInstanceParticle,0,0);

	return S_OK;
}



CVIBuffer_Particle_Point* CVIBuffer_Particle_Point::Create(const VIBUFFER_PARTICLE_POINT_DESC* pInitialDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CVIBuffer_Particle_Point* pParticlePoint = new CVIBuffer_Particle_Point(pDevice, pContext);

    if (FAILED(pParticlePoint->CVIBuffer_Particle_Point::Proto_Initialize(pInitialDesc)))
    {
        Safe_Release(pParticlePoint);
        MSG_BOX("VIBuffer Particle Point Buffer is failed to Create");
        return nullptr;
    }
    return pParticlePoint;
}


CComponent* CVIBuffer_Particle_Point::Clone(void* pArg)
{
    CVIBuffer_Particle_Point* pParticlePoint = new CVIBuffer_Particle_Point(*this);

    if (FAILED(pParticlePoint->CVIBuffer_Particle_Point::Clone_Initialize(pArg)))
    {
        Safe_Release(pParticlePoint);
        MSG_BOX("VIBuffer Particle Point Buffer is failed to Create");
        return nullptr;
    }
    return pParticlePoint;
}

void CVIBuffer_Particle_Point::Free()
{
    __super::Free();

}


const INPUT_ELEMENT_DESC* CVIBuffer_Particle_Point::GetInputElementDesc()
{
    return VTXPOS::GetInputElementDesc().data();
}

uint32 CVIBuffer_Particle_Point::GetInputElementDescSize()
{
    return Cast_Uint(VTXPOS::GetInputElementDescSize());
}
