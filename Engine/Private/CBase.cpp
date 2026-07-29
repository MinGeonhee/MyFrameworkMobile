#include "pch.h"
#include "CBase.h"

uint32 CBase::AddRef()
{
    return ++m_iRefCount;
}

uint32 CBase::Release()
{
    if (m_iRefCount == 0)
    {
        Free();
        delete this;
        return 0;
    }

    return --m_iRefCount;
}

void CBase::Free()
{
}
