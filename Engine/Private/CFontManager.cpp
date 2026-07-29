#include "pch.h"
#include "CFontManager.h"
CFontManager* CFontManager::Create(){return new CFontManager{};}
HRESULT CFontManager::Add_Font(float size,const std::wstring& key,const std::wstring& path){if(key.empty()||path.empty()||size<=0.f)return E_INVALIDARG;Engine::FONT_DESC d{path,size};return m_Fonts.emplace(key,std::move(d)).second?S_OK:E_FAIL;}
const Engine::FONT_DESC* CFontManager::Get_Font(const std::wstring& key)const{auto i=m_Fonts.find(key);return i==m_Fonts.end()?nullptr:&i->second;}
HRESULT CFontManager::Draw(const std::wstring& key,const std::wstring& text,float x,float y,const float color[4])const
{
    const auto* font=Get_Font(key);if(!font)return E_INVALIDARG;
    // DirectXTK SpriteFont는 모바일/Vulkan에서 그대로 쓸 수 없어 실제 출력기는 콜백 백엔드로 분리한다.
    return m_DrawHandler?m_DrawHandler(*font,text,x,y,color):E_NOTIMPL;
}
HRESULT CFontManager::Ready_Default(){return S_OK;}
void CFontManager::Free(){m_DrawHandler={};m_Fonts.clear();CBase::Free();}
