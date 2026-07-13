#pragma once

#include "Engine_Define.h"

class ENGINE_DLL CBase
{
protected:
    CBase() = default;
    virtual ~CBase() = default;

public:
    virtual void Free();
};

