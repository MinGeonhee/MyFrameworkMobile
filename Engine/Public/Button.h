#pragma once
#include "CImGuiFunctionBase.h"
#ifdef _DEBUG

NS_BEGIN(Engine)

NS_BEGIN(MYIMGUI)


class ENGINE_DLL Button :    public CImGuiFunctionBase
{
public:
	explicit Button();
	explicit Button(const char* Name , const ImVec2& imvevSize );
	explicit Button(const char* Name, const Float2& float2Size);
	explicit Button(const char* Name, float fSizeX, float fSizeY);
	virtual  ~Button() { m_Name = nullptr; }

public:
	void	Set_ButtonName(const char* Name) { m_Name = Name; }
	void	Set_ButtonSize(const ImVec2& imvecSize) { m_tSize = imvecSize; }
	void	Set_ButtonSize(const Float2& float2Size) { m_tSize = { float2Size.x, float2Size.y }; }
	void	Set_ButtonSize(float fSizeX, float fSizeY) { m_tSize = { fSizeX, fSizeY }; }
public:

	virtual void Render() override
	{
		bool isWork{ false };

		ImGui::PushID(m_iCount++);

		if(ImGui::Button(m_Name,m_tSize))
			m_OnWork();

		ImGui::PopID();
	}

private:
	const char* m_Name;
	ImVec2		m_tSize;
	uint32		m_iCount;
};


NS_END
NS_END

#endif
