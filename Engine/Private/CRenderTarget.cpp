#include "pch.h"
#include "CRenderTarget.h"
HRESULT CRenderTarget::Initialize(const std::wstring&,uint32 w,uint32 h,const float* color,Diligent::TEXTURE_FORMAT format,Diligent::IRenderDevice* device)
{
 if(!device||w==0||h==0)return E_INVALIDARG;Diligent::TextureDesc d{};d.Name="Engine RenderTarget";d.Type=Diligent::RESOURCE_DIM_TEX_2D;d.Width=w;d.Height=h;d.Format=format;d.BindFlags=Diligent::BIND_RENDER_TARGET|Diligent::BIND_SHADER_RESOURCE;device->CreateTexture(d,nullptr,&m_pTexture);if(!m_pTexture)return E_FAIL;m_pRTV=m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);m_pSRV=m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);if(color)std::copy(color,color+4,m_ClearColor);return m_pRTV&&m_pSRV?S_OK:E_FAIL;
}
CRenderTarget* CRenderTarget::Create(const std::wstring& n,uint32 w,uint32 h,const float* c,Diligent::TEXTURE_FORMAT f,Diligent::IRenderDevice* d){auto* p=new CRenderTarget{};if(FAILED(p->Initialize(n,w,h,c,f,d))){p->Release();return nullptr;}return p;}
void CRenderTarget::Free(){m_pRTV=nullptr;m_pSRV=nullptr;m_pTexture.Release();CBase::Free();}
