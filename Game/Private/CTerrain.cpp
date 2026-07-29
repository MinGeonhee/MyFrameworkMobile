#include "pch.h"
#include "CTerrain.h"
#include "CGameManager.h"
#include "CCameraObjectBase.h"
#include "CTransform.h"
#include "CVIBuffer_Terrain.h"
#include "CShader.h"
CTerrain::CTerrain():CGameObject{nullptr,nullptr}{} CTerrain::CTerrain(const CTerrain&p):CGameObject{p}{}
HRESULT CTerrain::Proto_Initialize(){return CGameObject::Proto_Initialize();}
HRESULT CTerrain::Clone_Initialize(void*a){if(FAILED(CGameObject::Clone_Initialize(a)))return E_FAIL;m_pShader=static_cast<CShader*>(m_pGameManager->ResourceManager_Get_Resource(L"Shader_Terrain_White",RESOURCE_TYPE::SHADER,false));return m_pShader?S_OK:E_FAIL;}
HRESULT CTerrain::Ready_Component(void*a){if(!a)return E_INVALIDARG;auto&d=*static_cast<TERRAIN_DESC*>(a);Engine::TRANSFORM_DESC td{};if(FAILED(Add_Component(Engine::GLOBAL_LEVEL,L"Proto_Component_Transform",L"Com_Transform",&td)))return E_FAIL;m_pTransform=static_cast<CTransform*>(Find_Component(L"Com_Transform"));CVIBuffer_Terrain::TERRAIN_BUFFER_DESC bd{d.iNumCellsX,d.iNumCellsZ,d.fCellSize};if(FAILED(Add_Component(Engine::GLOBAL_LEVEL,L"Proto_Component_VIBuffer_Terrain",L"Com_Buffer",&bd)))return E_FAIL;m_pBuffer=static_cast<CVIBuffer_Terrain*>(Find_Component(L"Com_Buffer"));return m_pTransform&&m_pBuffer?S_OK:E_FAIL;}
HRESULT CTerrain::Add_RenderGroup(){return m_pGameManager->RenderManager_Add_RenderGroup(RENDER_GROUP::NONBLEND,this);}
HRESULT CTerrain::Render(){auto*c=m_pGameManager->CameraManager_Get_MainCamera();if(!c||!m_pShader||!m_pBuffer)return E_FAIL;struct C{Diligent::float4x4 WVP;float Color[4];}v{(m_pTransform->Get_WorldMatrix()*c->Get_ViewMatrix()*c->Get_ProjectionMatrix()).Transpose(),{1,1,1,1}};if(FAILED(m_pShader->Set_GPU_ConstantBuffer("WorldConstants",&v,sizeof(v)))||FAILED(m_pShader->Apply_Pass("Default")))return E_FAIL;return m_pBuffer->Render();}
CTerrain* CTerrain::Create(){auto*p=new CTerrain{};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p;}CGameObject* CTerrain::Clone(void*a){auto*p=new CTerrain{*this};if(FAILED(p->Clone_Initialize(a))){p->Release();return nullptr;}return p;}void CTerrain::Free(){if(m_pShader)m_pShader->Release();m_pShader=nullptr;m_pTransform=nullptr;m_pBuffer=nullptr;CGameObject::Free();}
