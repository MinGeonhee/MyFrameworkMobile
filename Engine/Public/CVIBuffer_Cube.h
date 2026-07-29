#pragma once
#include "CVIBuffer.h"
class ENGINE_DLL CVIBuffer_Cube final:public CVIBuffer
{
private:explicit CVIBuffer_Cube(Device*,Context*);explicit CVIBuffer_Cube(const CVIBuffer_Cube&);~CVIBuffer_Cube()override=default;HRESULT Proto_Initialize();HRESULT Clone_Initialize(void*)override;
public:static CVIBuffer_Cube* Create(Device*,Context*);CComponent* Clone(void*)override;void Free()override;
};
