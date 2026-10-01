#pragma once
#include "CImGuiFunctionBase.h"
#ifdef _DEBUG

NS_BEGIN(Engine)

NS_BEGIN(MYIMGUI)


class ENGINE_DLL Slider_Float final : public CImGuiFunctionBase
{
public:
	explicit Slider_Float();
	explicit Slider_Float(const char* Name, float* fValue, float fMin, float fMax, bool isRenderInputFloat= false);
	virtual ~Slider_Float() {m_Name = nullptr, m_fValue = nullptr;
	}

public:
	void	Set_Slider(const char* Name, float* fValue, float fMin, float fMax)
	{
		m_Name = Name; m_fValue = fValue; m_fMin = fMin; m_fMax = fMax;
	}
	void	Set_Name(const char* Name) { m_Name = Name; }
	void	Set_Value(float* fValue) { m_fValue = fValue; }
	void	Set_fMin(float fMin) { m_fMin = fMin; }
	void	Set_fMax(float fMax) { m_fMax = fMax; }
	void	Set_IsRenderInputFloat ( bool isRender) { m_isRenderInputFloat = isRender;}
public:

	// CImGuiFunctionBase을(를) 통해 상속됨
	void Render() override
	{
		
		if (ImGui::SliderFloat(m_Name, m_fValue, m_fMin, m_fMax))
		{
			if( m_OnWork != nullptr)
				m_OnWork();
		}


		if (m_isRenderInputFloat)
		{
			char szName[128];
			sprintf_s(szName, std::size(szName), "Value##%d", m_iCount++);
			if (ImGui::InputFloat(szName, m_fValue, 0.1f, 0.1f))
				if( m_OnWork != nullptr)
					m_OnWork();
		}
	}

protected:
	const char* m_Name;
	float*		m_fValue;
	float		m_fMin, m_fMax;

	bool		m_isRenderInputFloat;
	uint32		m_iCount;
};



NS_END

NS_END

#endif // _DEBUG
