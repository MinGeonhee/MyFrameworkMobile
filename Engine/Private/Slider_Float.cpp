#include "Slider_Float.h"


#ifdef _DEBUG



USING(MYIMGUI)


Slider_Float::Slider_Float()
	: m_Name(nullptr), m_fValue(nullptr), m_fMax{}, m_fMin{}, m_isRenderInputFloat(false), m_iCount(0)
{
}

Slider_Float::Slider_Float(const char* Name, float* fValue, float fMin, float fMax , bool isRenderInputFloat)
	: m_Name(Name) , m_fValue(fValue) , m_fMin(fMin) , m_fMax(fMax), m_isRenderInputFloat(isRenderInputFloat)
	, m_iCount{0}
{
}

#endif // DEBUG

