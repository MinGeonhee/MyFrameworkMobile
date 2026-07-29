#pragma once
#include "CBase.h"
class CGameObject;
class ENGINE_DLL CPlayerManager final:public CBase
{
private:CPlayerManager()=default;
public:
 ~CPlayerManager()override=default;
 HRESULT Add_Player(const std::wstring& strKey,CGameObject* pPlayer);
 CGameObject* Get_Player(const std::wstring& strKey)const;
 void Remove_Player(const std::wstring& strKey);void Clear_Player();
 static CPlayerManager* Create();void Free()override;
private:std::unordered_map<std::wstring,CGameObject*> m_Players;
};
