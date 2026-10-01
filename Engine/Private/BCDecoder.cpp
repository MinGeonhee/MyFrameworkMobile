#include "BCDecoder.h"

#include <algorithm>
#include <cstring>

/* ------------------------------------------------------------------------------------------- */
/* BC1 ~ BC5 , BC7 CPU 디코더                                                                   */
/* BC7 파티션 / Fix-up 표는 Microsoft DirectXTex (MIT License) 의 BC6HBC7.cpp 에서 가져왔다.       */
/* ------------------------------------------------------------------------------------------- */

namespace
{
	/* ---------------- BC1 ~ BC5 ---------------- */
	void Decode565(uint16_t c, uint8_t* pOut)
	{
		const uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
		pOut[0] = static_cast<uint8_t>((r << 3) | (r >> 2));
		pOut[1] = static_cast<uint8_t>((g << 2) | (g >> 4));
		pOut[2] = static_cast<uint8_t>((b << 3) | (b >> 2));
		pOut[3] = 255;
	}

	void Decode_ColorBlock(const uint8_t* pBlock, uint8_t* pOut, bool isBC1)
	{
		const uint16_t c0 = static_cast<uint16_t>(pBlock[0] | (pBlock[1] << 8));
		const uint16_t c1 = static_cast<uint16_t>(pBlock[2] | (pBlock[3] << 8));
		uint8_t arrColor[4][4];
		Decode565(c0, arrColor[0]);
		Decode565(c1, arrColor[1]);
		if (c0 > c1 || !isBC1)
		{
			for (int k = 0; k < 3; ++k)
			{
				arrColor[2][k] = static_cast<uint8_t>((2 * arrColor[0][k] + arrColor[1][k]) / 3);
				arrColor[3][k] = static_cast<uint8_t>((arrColor[0][k] + 2 * arrColor[1][k]) / 3);
			}
			arrColor[2][3] = arrColor[3][3] = 255;
		}
		else
		{
			for (int k = 0; k < 3; ++k)
				arrColor[2][k] = static_cast<uint8_t>((arrColor[0][k] + arrColor[1][k]) / 2);
			arrColor[2][3] = 255;
			arrColor[3][0] = arrColor[3][1] = arrColor[3][2] = 0;
			arrColor[3][3] = 0;
		}
		const uint32_t iIndices = pBlock[4] | (pBlock[5] << 8) | (pBlock[6] << 16) | (static_cast<uint32_t>(pBlock[7]) << 24);
		for (int i = 0; i < 16; ++i)
			std::memcpy(pOut + i * 4, arrColor[(iIndices >> (i * 2)) & 3], 4);
	}

	void Decode_AlphaBlock(const uint8_t* pBlock, uint8_t* pOut, int iStride)
	{
		const uint32_t a0 = pBlock[0], a1 = pBlock[1];
		uint8_t arrAlpha[8];
		arrAlpha[0] = static_cast<uint8_t>(a0);
		arrAlpha[1] = static_cast<uint8_t>(a1);
		if (a0 > a1)
		{
			for (int i = 1; i < 7; ++i)
				arrAlpha[i + 1] = static_cast<uint8_t>(((7 - i) * a0 + i * a1) / 7);
		}
		else
		{
			for (int i = 1; i < 5; ++i)
				arrAlpha[i + 1] = static_cast<uint8_t>(((5 - i) * a0 + i * a1) / 5);
			arrAlpha[6] = 0;
			arrAlpha[7] = 255;
		}
		uint64_t iBits = 0;
		for (int i = 0; i < 6; ++i)
			iBits |= static_cast<uint64_t>(pBlock[2 + i]) << (8 * i);
		for (int i = 0; i < 16; ++i)
			pOut[i * iStride] = arrAlpha[(iBits >> (i * 3)) & 7];
	}

	/* ---------------- BC7 ---------------- */
	const uint8_t g_aPartition2[64][16] = {
		{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 }, { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
		{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 }, { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
		{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
		{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
		{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
		{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 }, { 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
		{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
		{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 }, { 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
		{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 }, { 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 }, { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
		{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 }, { 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
		{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
		{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
		{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
		{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 }, { 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
		{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 }, { 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 }, { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
	};

	const uint8_t g_aPartition3[64][16] = {
		{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2 }, { 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1 },
		{ 0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1 }, { 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2 }, { 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2 },
		{ 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 }, { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 },
		{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2 },
		{ 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2 }, { 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2 },
		{ 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, { 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0 },
		{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2 }, { 0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0 },
		{ 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2 }, { 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2 }, { 0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1 },
		{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2 }, { 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0 },
		{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0 }, { 0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2 },
		{ 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1 },
		{ 0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2 }, { 0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2 },
		{ 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1 }, { 0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2 }, { 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1 },
		{ 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2 }, { 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0 },
		{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 }, { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 },
		{ 0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0 }, { 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1 },
		{ 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1 }, { 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1 }, { 0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2 },
		{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1 }, { 0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1 },
		{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1 }, { 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 },
		{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2 }, { 0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1 },
		{ 0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2 }, { 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2 },
		{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2 }, { 0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2 },
		{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2 }, { 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2 },
		{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2 },
		{ 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2 },
		{ 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1 }, { 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2 },
		{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }, { 0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0 },
	};

	/* 2 Subset 의 두번째 Subset Fix-up */
	const uint8_t g_aFixUp2[64] = {
		15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
		15, 2, 8, 2,  2, 8, 8,15,  2, 8, 2, 2,  8, 8, 2, 2,
		15,15, 6, 8,  2, 8,15,15,  2, 8, 2, 2,  2,15,15, 6,
		 6, 2, 6, 8, 15,15, 2, 2, 15,15,15,15, 15, 2, 2,15,
	};

	/* 3 Subset 의 두번째 / 세번째 Subset Fix-up */
	const uint8_t g_aFixUp3[64][2] = {
		{ 3,15 },{ 3, 8 },{15, 8 },{15, 3 }, { 8,15 },{ 3,15 },{15, 3 },{15, 8 },
		{ 8,15 },{ 8,15 },{ 6,15 },{ 6,15 }, { 6,15 },{ 5,15 },{ 3,15 },{ 3, 8 },
		{ 3,15 },{ 3, 8 },{ 8,15 },{15, 3 }, { 3,15 },{ 3, 8 },{ 6,15 },{10, 8 },
		{ 5, 3 },{ 8,15 },{ 8, 6 },{ 6,10 }, { 8,15 },{ 5,15 },{15,10 },{15, 8 },
		{ 8,15 },{15, 3 },{ 3,15 },{ 5,10 }, { 6,10 },{10, 8 },{ 8, 9 },{15,10 },
		{15, 6 },{ 3,15 },{15, 8 },{ 5,15 }, {15, 3 },{15, 6 },{15, 6 },{15, 8 },
		{ 3,15 },{15, 3 },{ 5,15 },{ 5,15 }, { 5,15 },{ 8,15 },{ 5,15 },{10,15 },
		{ 5,15 },{10,15 },{ 8,15 },{13,15 }, {15, 3 },{12,15 },{ 3,15 },{ 3, 8 },
	};

	const uint16_t g_aWeights2[4] = { 0, 21, 43, 64 };
	const uint16_t g_aWeights3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
	const uint16_t g_aWeights4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };

	struct BC7_MODE
	{
		uint8_t iNumSubsets;
		uint8_t iPartitionBits;
		uint8_t iRotationBits;
		uint8_t iIndexSelectionBits;
		uint8_t iColorBits;
		uint8_t iAlphaBits;
		uint8_t iEndpointPBits;
		uint8_t iSharedPBits;
		uint8_t iIndexBits;
		uint8_t iIndexBits2;
	};

	const BC7_MODE g_aModes[8] = {
		{ 3, 4, 0, 0, 4, 0, 1, 0, 3, 0 },
		{ 2, 6, 0, 0, 6, 0, 0, 1, 3, 0 },
		{ 3, 6, 0, 0, 5, 0, 0, 0, 2, 0 },
		{ 2, 6, 0, 0, 7, 0, 1, 0, 2, 0 },
		{ 1, 0, 2, 1, 5, 6, 0, 0, 2, 3 },
		{ 1, 0, 2, 0, 7, 8, 0, 0, 2, 2 },
		{ 1, 0, 0, 0, 7, 7, 1, 0, 4, 0 },
		{ 2, 6, 0, 0, 5, 5, 1, 0, 2, 0 },
	};

	struct BIT_READER
	{
		const uint8_t* pData;
		uint32_t       iPos = 0;
		uint32_t Read(uint32_t iCount)
		{
			uint32_t iValue = 0;
			for (uint32_t i = 0; i < iCount; ++i)
			{
				const uint32_t iBit = (pData[(iPos + i) >> 3] >> ((iPos + i) & 7)) & 1;
				iValue |= iBit << i;
			}
			iPos += iCount;
			return iValue;
		}
	};

	uint8_t Interpolate(uint8_t e0, uint8_t e1, uint32_t iIndex, uint32_t iBits)
	{
		const uint16_t* pWeights = iBits == 2 ? g_aWeights2 : (iBits == 3 ? g_aWeights3 : g_aWeights4);
		const uint32_t w = pWeights[iIndex];
		return static_cast<uint8_t>(((64 - w) * e0 + w * e1 + 32) >> 6);
	}

	uint8_t Expand(uint32_t iValue, uint32_t iBits)
	{
		iValue <<= (8 - iBits);
		return static_cast<uint8_t>(iValue | (iValue >> iBits));
	}

	bool Is_Anchor(uint32_t iNumSubsets, uint32_t iPartition, uint32_t iPixel)
	{
		if (iPixel == 0)
			return true;
		if (iNumSubsets == 2)
			return iPixel == g_aFixUp2[iPartition];
		if (iNumSubsets == 3)
			return iPixel == g_aFixUp3[iPartition][0] || iPixel == g_aFixUp3[iPartition][1];
		return false;
	}
}

namespace Engine
{
	void BC7_DecodeBlock(const uint8_t* pBlock, uint8_t* pOut)
	{
		uint32_t iMode = 0;
		while (iMode < 8 && ((pBlock[0] >> iMode) & 1) == 0)
			++iMode;
		if (iMode >= 8)
		{
			std::memset(pOut, 0, 64);
			return;
		}

		const BC7_MODE& tMode = g_aModes[iMode];
		BIT_READER tReader{ pBlock };
		tReader.Read(iMode + 1);

		const uint32_t iPartition = tReader.Read(tMode.iPartitionBits);
		const uint32_t iRotation = tReader.Read(tMode.iRotationBits);
		const uint32_t iIndexSelection = tReader.Read(tMode.iIndexSelectionBits);

		const uint32_t iNumEndpoints = tMode.iNumSubsets * 2u;
		uint8_t arrEndpoint[6][4] = {};

		for (uint32_t c = 0; c < 3; ++c)
			for (uint32_t e = 0; e < iNumEndpoints; ++e)
				arrEndpoint[e][c] = static_cast<uint8_t>(tReader.Read(tMode.iColorBits));
		if (tMode.iAlphaBits > 0)
			for (uint32_t e = 0; e < iNumEndpoints; ++e)
				arrEndpoint[e][3] = static_cast<uint8_t>(tReader.Read(tMode.iAlphaBits));

		uint32_t iColorBits = tMode.iColorBits;
		uint32_t iAlphaBits = tMode.iAlphaBits;
		if (tMode.iEndpointPBits)
		{
			for (uint32_t e = 0; e < iNumEndpoints; ++e)
			{
				const uint32_t p = tReader.Read(1);
				for (uint32_t c = 0; c < 4; ++c)
					arrEndpoint[e][c] = static_cast<uint8_t>((arrEndpoint[e][c] << 1) | p);
			}
			++iColorBits;
			if (iAlphaBits) ++iAlphaBits;
		}
		else if (tMode.iSharedPBits)
		{
			for (uint32_t s = 0; s < tMode.iNumSubsets; ++s)
			{
				const uint32_t p = tReader.Read(1);
				for (uint32_t e = s * 2; e < s * 2 + 2; ++e)
					for (uint32_t c = 0; c < 4; ++c)
						arrEndpoint[e][c] = static_cast<uint8_t>((arrEndpoint[e][c] << 1) | p);
			}
			++iColorBits;
			if (iAlphaBits) ++iAlphaBits;
		}

		for (uint32_t e = 0; e < iNumEndpoints; ++e)
		{
			for (uint32_t c = 0; c < 3; ++c)
				arrEndpoint[e][c] = Expand(arrEndpoint[e][c], iColorBits);
			arrEndpoint[e][3] = iAlphaBits ? Expand(arrEndpoint[e][3], iAlphaBits) : 255;
		}

		/* 인덱스 */
		uint32_t arrIndex[16] = {}, arrIndex2[16] = {};
		for (uint32_t i = 0; i < 16; ++i)
		{
			const uint32_t iBits = Is_Anchor(tMode.iNumSubsets, iPartition, i) ? tMode.iIndexBits - 1u : tMode.iIndexBits;
			arrIndex[i] = tReader.Read(iBits);
		}
		if (tMode.iIndexBits2)
			for (uint32_t i = 0; i < 16; ++i)
				arrIndex2[i] = tReader.Read(i == 0 ? tMode.iIndexBits2 - 1u : tMode.iIndexBits2);

		for (uint32_t i = 0; i < 16; ++i)
		{
			uint32_t iSubset = 0;
			if (tMode.iNumSubsets == 2) iSubset = g_aPartition2[iPartition][i];
			else if (tMode.iNumSubsets == 3) iSubset = g_aPartition3[iPartition][i];

			const uint8_t* e0 = arrEndpoint[iSubset * 2];
			const uint8_t* e1 = arrEndpoint[iSubset * 2 + 1];
			uint8_t* pPixel = pOut + i * 4;

			if (tMode.iIndexBits2 == 0)
			{
				for (uint32_t c = 0; c < 4; ++c)
					pPixel[c] = Interpolate(e0[c], e1[c], arrIndex[i], tMode.iIndexBits);
			}
			else
			{
				/* Mode 4 / 5 : 색상과 알파 인덱스가 따로 있다 */
				uint32_t iColorIndex = arrIndex[i], iColorIndexBits = tMode.iIndexBits;
				uint32_t iAlphaIndex = arrIndex2[i], iAlphaIndexBits = tMode.iIndexBits2;
				if (iIndexSelection)
				{
					std::swap(iColorIndex, iAlphaIndex);
					std::swap(iColorIndexBits, iAlphaIndexBits);
				}
				for (uint32_t c = 0; c < 3; ++c)
					pPixel[c] = Interpolate(e0[c], e1[c], iColorIndex, iColorIndexBits);
				pPixel[3] = Interpolate(e0[3], e1[3], iAlphaIndex, iAlphaIndexBits);
			}

			if (iRotation == 1) std::swap(pPixel[0], pPixel[3]);
			else if (iRotation == 2) std::swap(pPixel[1], pPixel[3]);
			else if (iRotation == 3) std::swap(pPixel[2], pPixel[3]);
		}
	}

	void BC_Decompress(int iFormat, const uint8_t* pBlocks, uint32_t iWidth, uint32_t iHeight, std::vector<uint8_t>& vecRGBA)
	{
		vecRGBA.assign(static_cast<size_t>(iWidth) * iHeight * 4, 255);
		const uint32_t iBlocksX = (std::max)(1u, (iWidth + 3) / 4);
		const uint32_t iBlocksY = (std::max)(1u, (iHeight + 3) / 4);
		const uint32_t iBlockBytes = (iFormat == 1 || iFormat == 4) ? 8u : 16u;

		uint8_t arrBlock[64];
		for (uint32_t by = 0; by < iBlocksY; ++by)
		{
			for (uint32_t bx = 0; bx < iBlocksX; ++bx)
			{
				const uint8_t* pBlock = pBlocks + (static_cast<size_t>(by) * iBlocksX + bx) * iBlockBytes;
				switch (iFormat)
				{
				case 1: Decode_ColorBlock(pBlock, arrBlock, true); break;
				case 2:
					Decode_ColorBlock(pBlock + 8, arrBlock, false);
					for (int i = 0; i < 16; ++i)
					{
						const uint32_t a = (pBlock[i / 2] >> ((i & 1) * 4)) & 0xF;
						arrBlock[i * 4 + 3] = static_cast<uint8_t>(a * 17);
					}
					break;
				case 3:
					Decode_ColorBlock(pBlock + 8, arrBlock, false);
					Decode_AlphaBlock(pBlock, arrBlock + 3, 4);
					break;
				case 4:
					Decode_AlphaBlock(pBlock, arrBlock, 4);
					for (int i = 0; i < 16; ++i) { arrBlock[i * 4 + 1] = arrBlock[i * 4 + 2] = arrBlock[i * 4]; arrBlock[i * 4 + 3] = 255; }
					break;
				case 5:
					Decode_AlphaBlock(pBlock, arrBlock, 4);
					Decode_AlphaBlock(pBlock + 8, arrBlock + 1, 4);
					for (int i = 0; i < 16; ++i) { arrBlock[i * 4 + 2] = 0; arrBlock[i * 4 + 3] = 255; }
					break;
				default: BC7_DecodeBlock(pBlock, arrBlock); break;
				}

				for (uint32_t y = 0; y < 4; ++y)
				{
					const uint32_t py = by * 4 + y;
					if (py >= iHeight) break;
					for (uint32_t x = 0; x < 4; ++x)
					{
						const uint32_t px = bx * 4 + x;
						if (px >= iWidth) break;
						std::memcpy(&vecRGBA[(static_cast<size_t>(py) * iWidth + px) * 4], arrBlock + (y * 4 + x) * 4, 4);
					}
				}
			}
		}
	}
}
