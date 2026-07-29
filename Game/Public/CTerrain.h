#pragma once
#include "CGameObject.h"
class CTransform;class CVIBuffer_Terrain;class CShader;
class CTerrain final:public CGameObject
{
public:struct TERRAIN_DESC:public Engine::GAMEOBJECT_DESC{uint32 iNumCellsX=10;uint32 iNumCellsZ=10;float fCellSize=1.f;};
private:CTerrain();CTerrain(const CTerrain&);~CTerrain()override=default;
public:HRESULT Proto_Initialize()override;HRESULT Clone_Initialize(void*)override;HRESULT Ready_Component(void*)override;HRESULT Add_RenderGroup()override;HRESULT Render()override;static CTerrain* Create();CGameObject* Clone(void*)override;void Free()override;
private:CTransform*m_pTransform=nullptr;CVIBuffer_Terrain*m_pBuffer=nullptr;CShader*m_pShader=nullptr;
};
