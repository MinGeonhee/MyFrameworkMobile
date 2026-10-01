#pragma once
#include "CBase.h"

namespace Engine
{
	class CGameObject;
}


typedef struct EventBase
{
	/* 이벤트를 각자 Enum으로 관리할 수 있게 해주는? Event Index라고 봐도 무방 */
	uint32 iEventPath{}; //
}EVENT_BASE;

using HASH_CODE = size_t;

NS_BEGIN(Engine)

class CEventBus : public CBase
{
private:
	explicit CEventBus() {}
	virtual ~CEventBus() {}
public:
	template<class T>
	static void Subscribe(function<void(const T& tEvent)> funcEvent)
	{
		/* void* pPtr 이 이 매개변수를 받는 함수로 인식 */
		m_umapEventFunction[typeid(T).hash_code()].push_back([funcEvent](const void* pPtr) {
			funcEvent(*static_cast<const T*>(pPtr));
			});
	}
	template<class T>
	static void Publish(const T& tInfo, bool isDelay = false)
	{
		size_t Hash = typeid(T).hash_code();
		
		for (auto& Func : m_umapEventFunction[Hash])
		{
			if (!isDelay)
				Func(static_cast<const void*>(&tInfo));
			else
				m_vecDelayEvents.push_back([eventCopy = tInfo, Func]()->void {
				Func(static_cast<const void*>(&eventCopy));
					});
		}
	}

	static void Execute_DelayPulish()
	{
		for (auto& Func : m_vecDelayEvents)
			Func();
		m_vecDelayEvents.clear();
	}

public:
	static  inline unordered_map< HASH_CODE, vector<function<void(const void*)>>> m_umapEventFunction;
	static	inline vector<function<void()>> m_vecDelayEvents;

	static  void Reset_Func();
public:
	virtual void Free()override
	{
		__super::Free();
	}
};

NS_END
