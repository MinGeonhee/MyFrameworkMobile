#pragma once
#include "CBase.h"
class CRenderTarget;class CShader;
class ENGINE_DLL CRenderTargetManager final:public CBase
{
private:CRenderTargetManager(Diligent::IRenderDevice* d,Diligent::IDeviceContext* c,Diligent::ISwapChain* s):m_pDevice(d),m_pContext(c),m_pSwapChain(s){}
public:
 ~CRenderTargetManager()override=default;
 HRESULT Ready_Default();
 HRESULT Add_RenderTarget(const std::wstring& key,uint32 w,uint32 h,const float* color,Diligent::TEXTURE_FORMAT format=Diligent::TEX_FORMAT_RGBA8_UNORM);
 HRESULT Add_MRT(const std::wstring& mrtKey,const std::wstring& targetKey);
 CRenderTarget* Find_RenderTarget(const std::wstring& key)const;
 HRESULT Bind_ShaderResource(const std::wstring& targetKey,const std::string& variable,CShader* shader);
 HRESULT Begin_MRT(const std::wstring& key);HRESULT End_MRT();
 static CRenderTargetManager* Create(Diligent::IRenderDevice* d,Diligent::IDeviceContext* c,Diligent::ISwapChain* s);void Free()override;
private:
 std::unordered_map<std::wstring,CRenderTarget*> m_Targets;
 std::unordered_map<std::wstring,std::vector<CRenderTarget*>> m_MRTs;
 Diligent::IRenderDevice* m_pDevice=nullptr;Diligent::IDeviceContext* m_pContext=nullptr;Diligent::ISwapChain* m_pSwapChain=nullptr;
};
