#pragma once
#include "CBase.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Texture.h"
class ENGINE_DLL CRenderTarget final:public CBase
{
private:CRenderTarget()=default;
public:
 ~CRenderTarget()override=default;
 HRESULT Initialize(const std::wstring& strName,uint32 w,uint32 h,const float* clearColor,Diligent::TEXTURE_FORMAT format,Diligent::IRenderDevice* device);
 Diligent::ITextureView* Get_RTV()const{return m_pRTV;}
 Diligent::ITextureView* Get_SRV()const{return m_pSRV;}
 const float* Get_ClearColor()const{return m_ClearColor;}
 static CRenderTarget* Create(const std::wstring& strName,uint32 w,uint32 h,const float* clearColor,Diligent::TEXTURE_FORMAT format,Diligent::IRenderDevice* device);
 void Free()override;
private:
 Diligent::RefCntAutoPtr<Diligent::ITexture> m_pTexture;
 Diligent::ITextureView* m_pRTV=nullptr;Diligent::ITextureView* m_pSRV=nullptr;
 float m_ClearColor[4]{};
};
