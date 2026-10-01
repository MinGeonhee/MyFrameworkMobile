#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)
NS_BEGIN(MYIMGUI)

class CImGuiFunctionBase abstract
{
public:
	virtual void	Render() {};
public:
	template<typename Func>
	void SetOnWork(Func&& func)
	{
		/*  */
		m_OnWork = std::forward<Func>(func);
	}
protected:
	std::function<void()>	m_OnWork;
};

NS_END
NS_END