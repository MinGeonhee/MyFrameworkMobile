#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
#include <functional>
class ENGINE_DLL CFontManager final : public CBase
{
public: using DrawHandler=std::function<HRESULT(const Engine::FONT_DESC&,const std::wstring&,float,float,const float[4])>;
private:CFontManager()=default;
public:
    ~CFontManager()override=default;
    HRESULT Add_Font(float fOriginFontSize,const std::wstring& strKey,const std::wstring& strFontPath);
    const Engine::FONT_DESC* Get_Font(const std::wstring& strKey)const;
    HRESULT Draw(const std::wstring& strKey,const std::wstring& strText,float x,float y,const float color[4])const;
    void Set_DrawHandler(DrawHandler Handler){m_DrawHandler=std::move(Handler);}
    HRESULT Ready_Default(); static CFontManager* Create(); void Free()override;
private:std::unordered_map<std::wstring,Engine::FONT_DESC>m_Fonts;DrawHandler m_DrawHandler;
};
