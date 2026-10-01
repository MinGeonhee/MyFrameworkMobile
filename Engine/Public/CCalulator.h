#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)




class ENGINE_DLL CCalulator
{
public:
	static Float2	Get_ScreenPos_To_NDC_ToStore(float ScreenPosX , float ScreenPosY , float _ScreenStartX , float _ScreenStartY , float _ScreenSizeX , float _ScreenSizeY );
	static Vector4  Get_Rotation_From_DirectionVector(const fVector4_1& vDirection);
};




NS_END