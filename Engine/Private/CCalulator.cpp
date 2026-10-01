#include "CCalulator.h"



Float2 CCalulator::Get_ScreenPos_To_NDC_ToStore(float ScreenPosX, float ScreenPosY, float _ScreenStartX, float _ScreenStartY, float _ScreenSizeX, float _ScreenSizeY)
{
	float X = ( 2.f * ( ScreenPosX - _ScreenStartX  ) / _ScreenSizeX  - 1.f ) ;
	float Y = ((1.f - 2.f * (ScreenPosY - _ScreenStartY) / _ScreenSizeY));

	return Float2 { X , Y	};
}

Vector4 CCalulator::Get_Rotation_From_DirectionVector(const fVector4_1& vDirection)
{
	Vector4 vDir = XMVector3Normalize(vDirection);

	float Dir_X = XMVectorGetX(vDir);
	float Dir_Y = XMVectorGetY(vDir);
	float Dir_Z = XMVectorGetZ(vDir);

	float fLenXZ = sqrtf(Dir_X * Dir_X + Dir_Z * Dir_Z);

	float fRotX = atan2(-Dir_Y,fLenXZ);
	float fRotY = atan2(Dir_X, Dir_Z);

	Vector4 vRotation{ fRotX , fRotY , 0.f , 0.f };

	vRotation = vRotation * To_DEGREE;

	return vRotation;
}
