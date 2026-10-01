#include "CVIBuffer_Particle_Rect.h"
#include "CGameManager.h"

CVIBuffer_Particle_Rect::CVIBuffer_Particle_Rect(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CVIBuffer_Particle(pDevice,pContext)
{
}

CVIBuffer_Particle_Rect::CVIBuffer_Particle_Rect(const CVIBuffer_Particle_Rect& Proto)
    : CVIBuffer_Particle(Proto)
{

}

HRESULT	CVIBuffer_Particle_Rect::Proto_Initialize(const VIBUFFER_PARTICLE_RECT_DESC* pInitialDesc)
{
	m_iStrideVertex = sizeof(VTXPOSTEX);
	m_iCountVertex = 4;					
	m_iOffsetVertex = 0;
	m_iStrideIndex = sizeof(uint16);
	m_iIndexCountPerInstanceParticle = m_iCountIndex = 6;							
	m_iInstanceParticleVertexStride = sizeof(VTX_INSTANCE_PARTICLE);
	m_iOffsetIndex = 0;							

	m_iCountVertexBuffer = 2;

#pragma region Vertex Index Create
	VTXPOSTEX* pVertices = new VTXPOSTEX[m_iCountVertex];
	m_pVertexPosition = new Float3[m_iCountVertex];

	m_pVertexPosition[0] = pVertices[LT].vPosition = Float3(-0.5f, +0.5f, 0.f);
	pVertices[LT].vTexcoord = { 0.f , 0.f };

	m_pVertexPosition[1] = pVertices[RT].vPosition = Float3(+0.5f, +0.5f, 0.f);
	pVertices[RT].vTexcoord = { 1.f , 0.f };

	m_pVertexPosition[2] = pVertices[RB].vPosition = Float3(+0.5f, -0.5f, 0.f);
	pVertices[RB].vTexcoord = { 1.f , 1.f };

	m_pVertexPosition[3] = pVertices[LB].vPosition = Float3(-0.5f, -0.5f, 0.f);
	pVertices[LB].vTexcoord = { 0.f , 1.f };

	/* Index Buffer Data */
	m_pIndices = new uint16[m_iCountIndex];
	uint16* pIndices = static_cast<uint16*>(m_pIndices);

	pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2;
	pIndices[3] = 0; pIndices[4] = 2; pIndices[5] = 3;


#pragma endregion


#pragma region Vertex Desc
	BUFFER_DESC		tVertexDesc{};

	tVertexDesc.ByteWidth = m_iStrideVertex * m_iCountVertex;
	tVertexDesc.StructureByteStride = m_iStrideVertex; 
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;	
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER; //
	tVertexDesc.CPUAccessFlags = 0;  
	tVertexDesc.MiscFlags = 0;	

#pragma endregion
#pragma region Vertex Data

	SUBRESOURCE_DATA	tVertexData{};
	tVertexData.pSysMem = pVertices;		// Vertex 배열의 시작 주소를 넣어준다.


#pragma endregion
#pragma region Index Desc
	BUFFER_DESC		tIndexDesc{};
	tIndexDesc.ByteWidth = m_iStrideIndex * m_iCountIndex;
	tIndexDesc.StructureByteStride = m_iStrideIndex;
	tIndexDesc.Usage = BUFFER_USAGE_DEFAULT;				// 일단 GPU말고는 접근을 막는다.
	tIndexDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;; //
	tIndexDesc.CPUAccessFlags = 0;
	tIndexDesc.MiscFlags = 0;
#pragma endregion
#pragma region Index Data

	SUBRESOURCE_DATA	tIndexData{};
	tIndexData.pSysMem = pIndices;

#pragma endregion

	if (FAILED(CVIBuffer::Proto_Initialize(tVertexDesc, tVertexData, tIndexDesc, tIndexData, TOPOLOGY_TRIANGLELIST)))
		return E_FAIL;

	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;


	/* Instance Vuffer */

	m_iNumInstanceParticle = pInitialDesc->iNumInstance;
	

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

	CGameManager* pMgr = CGameManager::GetInstance();

	/* 네모공간에 내가지정한 Range = Extents 보면될듯? 이 크기안에서 랜덤한 위치를 잡는다*/

	m_isLoop = pInitialDesc->isLoop;

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

	}


	return S_OK;
}

HRESULT	CVIBuffer_Particle_Rect::Clone_Initialize(void* pArg)
{
	SUBRESOURCE_DATA      InstanceInitialData{};
	InstanceInitialData.pSysMem = m_pInstanceParticleVertices;

	if (FAILED(Create_Buffer(m_tInstanceParticleVerticleBufferDesc, &InstanceInitialData, &m_pVBInstanceParticle)))
		return E_FAIL;


    return S_OK;
}


/* Particle 버퍼를 미리 생성가능하게끔?된거같은데 */

CVIBuffer_Particle_Rect* CVIBuffer_Particle_Rect::Create(const VIBUFFER_PARTICLE_RECT_DESC* pInitialDesc,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CVIBuffer_Particle_Rect* pParticleRect = new CVIBuffer_Particle_Rect(pDevice, pContext);

    if (FAILED(pParticleRect->CVIBuffer_Particle_Rect::Proto_Initialize(pInitialDesc)))
    {
        Safe_Release(pParticleRect);
        MSG_BOX("VIBuffer Particle Buffer is failed to Create");
        return nullptr;
    }
    return pParticleRect;
}


CComponent* CVIBuffer_Particle_Rect::Clone(void* pArg)
{
    CVIBuffer_Particle_Rect* pParticleRect = new CVIBuffer_Particle_Rect(*this);

    if (FAILED(pParticleRect->CVIBuffer_Particle_Rect::Clone_Initialize(pArg)))
    {
        Safe_Release(pParticleRect);
        MSG_BOX("VIBuffer Particle Buffer is failed to Create");
        return nullptr;
    }
    return pParticleRect;
}

void CVIBuffer_Particle_Rect::Free()
{
    __super::Free();

}


const INPUT_ELEMENT_DESC* CVIBuffer_Particle_Rect::GetInputElementDesc()
{
    return VTXPOSTEX().tInputElement_Desc.data();
}

uint32 CVIBuffer_Particle_Rect::GetInputElementDescSize()
{
    return Cast_Uint(VTXPOSTEX().tInputElement_Desc.size());
}
