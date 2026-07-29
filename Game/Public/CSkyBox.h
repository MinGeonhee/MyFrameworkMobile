#pragma once
#include "CGameObject.h"
class CVIBuffer_Cube;class CShader;class CTexture;
class CSkyBox final:public CGameObject
{
private:CSkyBox();CSkyBox(const CSkyBox&);~CSkyBox()override=default;
public:HRESULT Proto_Initialize()override;HRESULT Clone_Initialize(void*)override;HRESULT Ready_Component(void*)override;HRESULT Add_RenderGroup()override;HRESULT Render()override;static CSkyBox* Create();CGameObject* Clone(void*)override;void Free()override;
private:CVIBuffer_Cube*m_pBuffer=nullptr;CShader*m_pShader=nullptr;CTexture*m_pTexture=nullptr;
};
