#pragma once

#include "CImGuiFunctionBase.h"
#include "EnumUtils.h"
#include "CImGuiFuntion.h"
#ifdef _DEBUG

NS_BEGIN(Engine)
NS_BEGIN(MYIMGUI)



template<class ENUMTYPE>
class ENGINE_DLL EnumRadioButton :public CImGuiFunctionBase
{
public:
	explicit EnumRadioButton(ENUMTYPE eType)
		: m_OnWork{}, m_iCount{} , m_eType(eType)
	{
	}
	virtual	~EnumRadioButton() {}
public:
	void	Set_EnumType(ENUMTYPE eType) { m_eType = eType; }
public:
	ENUMTYPE Render_Radio()
	{
		string str = EnumUtils::GetEnumCategoryName<CAMERA_TYPE>();

		ImGui::Text(string(str + " => ").c_str());
		ImGui::SameLine();
		CImGuiFuntion::ColorText(COLOR_RED, EnumUtils::ENUM_TYPE_Enum_To_String<ENUMTYPE>(m_eType).c_str());

		uint32 iSelect = Cast_Uint(m_eType);
		for (uint32 i = 0; i < (uint32)ENUMTYPE::END; ++i)
		{
			ImGui::PushID(m_iCount++);

			if (ImGui::RadioButton(EnumUtils::ENUM_TYPE_Enum_To_String<ENUMTYPE>(ENUMTYPE(i)).c_str(), iSelect == i))
			{
				m_eType =  (ENUMTYPE)i;
				ImGui::PopID(); 
				return m_eType;
			}

			ImGui::PopID();
		}

		return m_eType;
	}
public:
	template<typename Func>
	void SetOnWork(Func&& func)
	{
		m_OnWork = std::forward<Func>(func);
	}
protected:
	std::function<void()>	m_OnWork;
	ENUMTYPE				m_eType;
	uint32					m_iCount;
};


NS_END
NS_END

#endif // DEBUG