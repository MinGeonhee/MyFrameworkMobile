#pragma once

#include <cstdint>
#include <vector>

namespace Engine
{
	/* BC(Block Compression) 텍스처를 RGBA8 로 푼다.                                            */
	/* 대부분의 Android GPU 는 BC 포맷(DX 계열 압축)을 지원하지 않으므로 로딩 시 CPU 에서 변환한다. */
	/* iFormat : 1 = BC1 , 2 = BC2 , 3 = BC3 , 4 = BC4 , 5 = BC5 , 6 = BC7                       */
	void BC_Decompress(int iFormat, const uint8_t* pBlocks, uint32_t iWidth, uint32_t iHeight, std::vector<uint8_t>& vecRGBA);

	/* 블록 하나(4x4) → RGBA8 16 픽셀 */
	void BC7_DecodeBlock(const uint8_t* pBlock, uint8_t* pOutRGBA);
}
