#pragma once
#include "EnumUtils.h"


namespace Engine
{
	class CTransform;
	class CGameObject;
}


NS_BEGIN(Engine)

class ENGINE_DLL CImGuiFuntion
{
public:
	static	void	ColorText( const Color& tColor , const string& str);
	static	void	ColorText( const Color& tColor , const wstring& wstr);
	static	void	RenderMatrix(const Float4x4& tMatrix);
	static	void	RenderMatrix(const Matrix& tMatrix);

	static	void	RenderTransform(CTransform* pTransform);

	static	void	RenderGameObjectName(CGameObject* pGameObject);
	static	void	RenderName(const string& str);
	static	void	FixName(wstring* pName);
	static	void	RenderName(const wstring& wstr);

	static	void	TextMouseClickFunc(std::function<void()> func);
	static void		RenderIndexAndName(int32 iIndex , wstring& wstrName);

	static void		Combo(const char* szLabel,std::string* strListName, uint32 iCurSelectIndex ,uint32 iListMaxNum, uint32& iSelectIndex,const std::function<void()>& EventFunc);

	static void		DeleteButton(CGameObject* pGameObject);
};

NS_END

