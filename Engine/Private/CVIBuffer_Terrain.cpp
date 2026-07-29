#include "pch.h"
#include "CVIBuffer_Terrain.h"

CVIBuffer_Terrain::CVIBuffer_Terrain(Device* d, Context* c):CVIBuffer{d,c}{}
CVIBuffer_Terrain::CVIBuffer_Terrain(const CVIBuffer_Terrain& p):CVIBuffer{p}{}
HRESULT CVIBuffer_Terrain::Proto_Initialize(){return S_OK;}
HRESULT CVIBuffer_Terrain::Clone_Initialize(void* pArg)
{
    if(!pArg)return E_INVALIDARG;
    const auto& d=*static_cast<TERRAIN_BUFFER_DESC*>(pArg);
    if(d.iNumCellsX==0||d.iNumCellsZ==0||d.fCellSize<=0.f)return E_INVALIDARG;
    const uint32 vx=d.iNumCellsX+1,vz=d.iNumCellsZ+1;
    std::vector<Engine::VTXPOSTEX> vertices(vx*vz);
    const float halfX=d.iNumCellsX*d.fCellSize*.5f,halfZ=d.iNumCellsZ*d.fCellSize*.5f;
    for(uint32 z=0;z<vz;++z)for(uint32 x=0;x<vx;++x){auto&v=vertices[z*vx+x];v.vPosition[0]=x*d.fCellSize-halfX;v.vPosition[2]=z*d.fCellSize-halfZ;v.vTexcoord[0]=float(x)/d.iNumCellsX;v.vTexcoord[1]=float(z)/d.iNumCellsZ;}
    std::vector<uint32> indices;indices.reserve(d.iNumCellsX*d.iNumCellsZ*6);
    for(uint32 z=0;z<d.iNumCellsZ;++z)for(uint32 x=0;x<d.iNumCellsX;++x){uint32 a=z*vx+x,b=a+1,c=a+vx,dv=c+1;indices.insert(indices.end(),{a,c,b,b,c,dv});}
    return CVIBuffer::Proto_Initialize(vertices.data(),uint32(vertices.size()*sizeof(Engine::VTXPOSTEX)),sizeof(Engine::VTXPOSTEX),uint32(vertices.size()),indices.data(),uint32(indices.size()*sizeof(uint32)),Diligent::VT_UINT32,uint32(indices.size()),Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}
CVIBuffer_Terrain* CVIBuffer_Terrain::Create(Device*d,Context*c){auto*p=new CVIBuffer_Terrain{d,c};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p;}
CComponent* CVIBuffer_Terrain::Clone(void*a){auto*p=new CVIBuffer_Terrain{*this};if(FAILED(p->Clone_Initialize(a))){p->Release();return nullptr;}return p;}
void CVIBuffer_Terrain::Free(){CVIBuffer::Free();}
