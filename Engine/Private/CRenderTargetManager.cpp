#include "pch.h"
#include "CRenderTargetManager.h"
#include "CRenderTarget.h"
#include "CShader.h"
CRenderTargetManager* CRenderTargetManager::Create(Diligent::IRenderDevice* d,Diligent::IDeviceContext* c,Diligent::ISwapChain* s){return d&&c&&s?new CRenderTargetManager{d,c,s}:nullptr;}
HRESULT CRenderTargetManager::Ready_Default(){return S_OK;}
HRESULT CRenderTargetManager::Add_RenderTarget(const std::wstring& k,uint32 w,uint32 h,const float* color,Diligent::TEXTURE_FORMAT f){if(k.empty()||m_Targets.count(k))return E_FAIL;auto* t=CRenderTarget::Create(k,w,h,color,f,m_pDevice);if(!t)return E_FAIL;m_Targets.emplace(k,t);return S_OK;}
HRESULT CRenderTargetManager::Add_MRT(const std::wstring& mk,const std::wstring& tk){auto* t=Find_RenderTarget(tk);if(mk.empty()||!t)return E_FAIL;auto& v=m_MRTs[mk];if(std::find(v.begin(),v.end(),t)!=v.end())return E_FAIL;v.push_back(t);return S_OK;}
CRenderTarget* CRenderTargetManager::Find_RenderTarget(const std::wstring& k)const{auto it=m_Targets.find(k);return it!=m_Targets.end()?it->second:nullptr;}
HRESULT CRenderTargetManager::Bind_ShaderResource(const std::wstring& k,const std::string& var,CShader* s){auto* t=Find_RenderTarget(k);return t&&s?s->Set_GPU_Texture(var,t->Get_SRV()):E_FAIL;}
HRESULT CRenderTargetManager::Begin_MRT(const std::wstring& k){auto it=m_MRTs.find(k);if(it==m_MRTs.end()||it->second.empty())return E_FAIL;std::vector<Diligent::ITextureView*> views;for(auto* t:it->second){views.push_back(t->Get_RTV());m_pContext->ClearRenderTarget(t->Get_RTV(),t->Get_ClearColor(),Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);}m_pContext->SetRenderTargets(static_cast<Diligent::Uint32>(views.size()),views.data(),nullptr,Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);return S_OK;}
HRESULT CRenderTargetManager::End_MRT(){if(!m_pSwapChain)return E_FAIL;auto* rtv=m_pSwapChain->GetCurrentBackBufferRTV();auto* dsv=m_pSwapChain->GetDepthBufferDSV();m_pContext->SetRenderTargets(1,&rtv,dsv,Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);return S_OK;}
void CRenderTargetManager::Free(){for(auto& p:m_Targets)p.second->Release();m_Targets.clear();m_MRTs.clear();m_pDevice=nullptr;m_pContext=nullptr;m_pSwapChain=nullptr;CBase::Free();}
