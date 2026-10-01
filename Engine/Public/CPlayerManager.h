#pragma once
#include "CBase.h"

namespace Engine
{
	class CGameObject;
}

NS_BEGIN(Engine)

class CPlayerManager final : public CBase
{
private:
	explicit CPlayerManager();
	virtual ~CPlayerManager() {}
public:
	HRESULT		 Add_Player(const wstring& wstrAddKey,CGameObject* pPlayer);
	CGameObject* Get_Player(const wstring& wstrFindKey);

	void		 Clear_Player();

	CGameObject* Compute_CheckRange(const fVector4_1& vCheckPos, float fCheckRange, CGameObject* pCurTargetPlayer = nullptr);

private:
	unordered_map<wstring,CGameObject*>	m_umapPlayer;
public:
	static	CPlayerManager* Create();
	virtual void Free()override;
};


NS_END
