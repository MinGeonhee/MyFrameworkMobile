#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	explicit CBase();
	virtual ~CBase() = default;

	using Super = CBase;


public:
	uint32	GetRef() { return m_iRefCnt; }

public:

	uint32	AddRef();
	// Release() 함수는 레퍼런스 카운터를 내리거 '나' 삭제하거나 선택을해서 함수 실행을 해준다.
	// Add할때 반환값은 증가되고난 이후 Cnt 값이 반환되지만 Release할때는 감소되고 난 이후 Cnt값이 반횐된다.
	// 이러한 이유는 디버깅을하거나 Cnt값을 체크할때 삭제할때도 Cnt 0  삭제안될때도 Cnt 값이 0 인 경우를 방지하기위해서 
	// 차별점을두어 좀더 보기 쉽게하기위해서라고 생각하자
	uint32	Release();

private:
	uint32	m_iRefCnt;
public:
	virtual void	Free();
};


NS_END
