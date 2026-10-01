#include "CBase.h"

USING(Engine)


CBase::CBase()
	: m_iRefCnt(0)
{
}

uint32 CBase::AddRef()
{
	return ++m_iRefCnt;
}

uint32 CBase::Release()
{
	if (m_iRefCnt == 0)
	{
		Free();

		delete this;

		return 0;
	}
	else
		return m_iRefCnt--;
}

void CBase::Free()
{
}
