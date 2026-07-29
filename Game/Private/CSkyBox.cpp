#include "pch.h"
#include "CSkyBox.h"
#include "CGameManager.h"
#include "CCameraObjectBase.h"
#include "CVIBuffer_Cube.h"
#include "CShader.h"
#include "CTexture.h"
CSkyBox::CSkyBox():CGameObject{nullptr,nullptr}{}CSkyBox::CSkyBox(const CSkyBox&p):CGameObject{p}{}
HRESULT CSkyBox::Proto_Initialize(){return CGameObject::Proto_Initialize();}
HRESULT CSkyBox::Clone_Initialize(void*a){if(FAILED(CGameObject::Clone_Initialize(a)))return E_FAIL;m_pShader=static_cast<CShader*>(m_pGameManager->ResourceManager_Get_Resource(L"Shader_SkyBox",RESOURCE_TYPE::SHADER,false));m_pTexture=static_cast<CTexture*>(m_pGameManager->ResourceManager_Get_Resource(L"Texture_SkyBox_Dawn",RESOURCE_TYPE::TEXTURE,false));return m_pShader&&m_pTexture?S_OK:E_FAIL;}
HRESULT CSkyBox::Ready_Component(void*){if(FAILED(Add_Component(Engine::GLOBAL_LEVEL,L"Proto_Component_VIBuffer_Cube",L"Com_Buffer",nullptr)))return E_FAIL;m_pBuffer=static_cast<CVIBuffer_Cube*>(Find_Component(L"Com_Buffer"));return m_pBuffer?S_OK:E_FAIL;}
HRESULT CSkyBox::Add_RenderGroup(){return m_pGameManager->RenderManager_Add_RenderGroup(RENDER_GROUP::SKYBOX,this);}
HRESULT CSkyBox::Render(){auto*c=m_pGameManager->CameraManager_Get_MainCamera();if(!c||!m_pBuffer||!m_pShader||!m_pTexture)return E_FAIL;auto view=c->Get_ViewMatrix();view._41=view._42=view._43=0.f;struct C{Diligent::float4x4 VP;}v{(view*c->Get_ProjectionMatrix()).Transpose()};if(FAILED(m_pShader->Set_GPU_ConstantBuffer("SkyConstants",&v,sizeof(v)))||FAILED(m_pShader->Set_GPU_Texture("g_Texture",m_pTexture->GetShaderResourceView()))||FAILED(m_pShader->Apply_Pass("Default")))return E_FAIL;return m_pBuffer->Render();}
CSkyBox* CSkyBox::Create(){auto*p=new CSkyBox{};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p;}CGameObject* CSkyBox::Clone(void*a){auto*p=new CSkyBox{*this};if(FAILED(p->Clone_Initialize(a))){p->Release();return nullptr;}return p;}void CSkyBox::Free(){if(m_pTexture)m_pTexture->Release();if(m_pShader)m_pShader->Release();m_pTexture=nullptr;m_pShader=nullptr;m_pBuffer=nullptr;CGameObject::Free();}
