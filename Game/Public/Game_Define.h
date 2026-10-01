#pragma once

#include "Engine_Define.h"

/* ------------------------------------------------------------------------------------------- */
/* Windows Client / Android 가 함께 사용하는 Game 공용 정의                                         */
/* ------------------------------------------------------------------------------------------- */

namespace Game
{
	/* 엔진 렌더 해상도 (원본 WINCX / WINCY). 실제 화면 크기와 상관없이 이 해상도로 그린 뒤 화면에 맞춰 출력한다. */
	inline constexpr uint32 WINCX = 1280;
	inline constexpr uint32 WINCY = 720;

	enum class SCENEID : uint32
	{
		STATIC = 0,
		TEST,
		END
	};

	enum class COLLIDER_GROUP_ID : uint32
	{
		DEFAULT = 0,
		END
	};

	/* RenderStateManager 에 등록하는 상태 번호 (원본 Client 의 BLEND_TYPE / DSS_TYPE) */
	enum class BLEND_TYPE : uint32 { NON_BLEND, BLEND_ALPHA, BLEND_ADD, END };
	enum class DSS_TYPE : uint32 { Depth_Write_On, Depth_Write_Off, END };
}

#define Shader_ArrayBoneMatrix		"g_arrayBoneMatrix"
