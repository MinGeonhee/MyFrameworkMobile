#include "Button.h"


USING(MYIMGUI)


#ifdef _DEBUG

Button::Button()
	: m_Name(nullptr), m_tSize{} , m_iCount(0)
{
	m_OnWork = NULL;
}

Button::Button(const char* Name, const ImVec2& imvevSize)
	: m_Name(Name), m_tSize(imvevSize), m_iCount(0)
{
}

Button::Button(const char* Name, const Float2& float2Size)
	: m_Name(Name), m_tSize(float2Size.x , float2Size.y), m_iCount(0)
{
}

Button::Button(const char* Name, float fSizeX, float fSizeY)
	: m_Name(Name), m_tSize(fSizeX, fSizeY) , m_iCount(0)
{
}
#endif // _DEBUG
