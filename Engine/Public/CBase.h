#pragma once

#include <cstdint>

#include "Engine_Define.h"
#include "Engine_Macro.h"

class ENGINE_DLL CBase
{
protected:
    CBase() = default;
    CBase(const CBase&) : m_iRefCount{0} {}
    virtual ~CBase() {};

public:
    uint32 AddRef();
    uint32 Release();
    uint32 Get_RefCount() const { return m_iRefCount; }
    virtual void Free();

private:
    uint32 m_iRefCount = 0;
};
