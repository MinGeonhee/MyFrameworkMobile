#pragma once

/* ------------------------------------------------------------------------------------------------- */
/* DirectXTK 대체 (Windows 전용 라이브러리)                                                             */
/* 원본 엔진이 디버그 그리기에 사용한 PrimitiveBatch<VertexPositionColor> / BasicEffect 를                */
/* 같은 사용법으로 Diligent 위에 다시 구현한다. (Windows / Android 공통)                                */
/* ------------------------------------------------------------------------------------------------- */

#include "Engine_Define.h"
#include <DirectXColors.h>

namespace DirectX
{
	struct VertexPositionColor
	{
		XMFLOAT3 position{};
		XMFLOAT4 color{};

		VertexPositionColor() = default;
		VertexPositionColor(const XMFLOAT3& iposition, const XMFLOAT4& icolor) noexcept : position(iposition), color(icolor) {}
		VertexPositionColor(FXMVECTOR iposition, FXMVECTOR icolor) noexcept
		{
			XMStoreFloat3(&position, iposition);
			XMStoreFloat4(&color, icolor);
		}
	};

	/* 정점 색상만 사용하는 BasicEffect (World * View * Projection) */
	class ENGINE_DLL BasicEffect
	{
	public:
		explicit BasicEffect(Engine::Device* pDevice) {}

		void XM_CALLCONV SetWorld(FXMMATRIX value) { XMStoreFloat4x4(&m_matWorld, value); }
		void XM_CALLCONV SetView(FXMMATRIX value) { XMStoreFloat4x4(&m_matView, value); }
		void XM_CALLCONV SetProjection(FXMMATRIX value) { XMStoreFloat4x4(&m_matProj, value); }
		void SetVertexColorEnabled(bool value) {}

		/* 이후 PrimitiveBatch 가 그릴 때 이 Effect 의 행렬을 사용한다 */
		void Apply(Engine::DeviceContext* pContext);

		XMFLOAT4X4 Get_WorldViewProj() const;

		static BasicEffect* Get_Current();

	private:
		XMFLOAT4X4 m_matWorld{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		XMFLOAT4X4 m_matView{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		XMFLOAT4X4 m_matProj{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	};

	/* ---------------------------------------------------------------------------------------- */
	/* SpriteBatch : 화면 픽셀 좌표로 텍스처 사각형을 모아서 그린다 (DirectXTK 와 같은 사용법)       */
	/* ---------------------------------------------------------------------------------------- */
	enum SpriteEffects : uint32_t
	{
		SpriteEffects_None = 0,
		SpriteEffects_FlipHorizontally = 1,
		SpriteEffects_FlipVertically = 2,
		SpriteEffects_FlipBoth = SpriteEffects_FlipHorizontally | SpriteEffects_FlipVertically,
	};

	class ENGINE_DLL SpriteBatch
	{
	public:
		explicit SpriteBatch(Engine::DeviceContext* pContext);
		~SpriteBatch();

		void Begin();
		void End();

		/* 텍스처의 pSourceRect 영역을 vPosition 에 그린다 */
		void XM_CALLCONV Draw(Engine::ShaderResourceView* pTexture, const XMFLOAT2& vPosition, const RECT* pSourceRect, FXMVECTOR vColor,
			float fRotation = 0.f, const XMFLOAT2& vOrigin = XMFLOAT2(0.f, 0.f), const XMFLOAT2& vScale = XMFLOAT2(1.f, 1.f), SpriteEffects eEffects = SpriteEffects_None, float fLayerDepth = 0.f);

	public:
		struct SPRITE
		{
			Engine::ShaderResourceView* pTexture;
			XMFLOAT2 vCorner[4];
			XMFLOAT2 vUV[4];
			XMFLOAT4 vColor;
		};
	private:
		std::vector<SPRITE>	m_vecSprite;
		bool				m_isBegin = false;
	};

	/* ---------------------------------------------------------------------------------------- */
	/* SpriteFont : DirectXTK MakeSpriteFont 로 만든 .spritefont 파일을 읽는다                      */
	/* ---------------------------------------------------------------------------------------- */
	class ENGINE_DLL SpriteFont
	{
	public:
		struct Glyph
		{
			uint32_t Character;
			int32_t  SubrectLeft, SubrectTop, SubrectRight, SubrectBottom;
			float    XOffset;
			float    YOffset;
			float    XAdvance;
		};

		SpriteFont(Engine::Device* pDevice, const wchar_t* pFileName);
		~SpriteFont();

		void XM_CALLCONV DrawString(SpriteBatch* pBatch, const wchar_t* pText, const XMFLOAT2& vPosition, FXMVECTOR vColor = Colors::White,
			float fRotation = 0.f, const XMFLOAT2& vOrigin = XMFLOAT2(0.f, 0.f), float fScale = 1.f) const;
		void XM_CALLCONV DrawString(SpriteBatch* pBatch, const wchar_t* pText, const XMFLOAT2& vPosition, FXMVECTOR vColor,
			float fRotation, const XMFLOAT2& vOrigin, const XMFLOAT2& vScale) const;

		XMVECTOR XM_CALLCONV MeasureString(const wchar_t* pText) const;
		float GetLineSpacing() const { return m_fLineSpacing; }
		bool  Is_Valid() const { return m_pTexture != nullptr; }

	private:
		const Glyph* Find_Glyph(wchar_t wcCharacter) const;

	private:
		std::vector<Glyph>					m_vecGlyph;
		float								m_fLineSpacing = 0.f;
		uint32_t							m_iDefaultCharacter = 0;
		Engine::ComPtr<Diligent::ITexture>	m_pTexture;
	};

	/* 내부 구현 (VertexPositionColor 전용 그리기) */
	ENGINE_DLL void PrimitiveBatch_Draw(Engine::PRIMITIVE_TOPOLOGY eTopology, const VertexPositionColor* pVertices, size_t iVertexCount, const uint16_t* pIndices, size_t iIndexCount);
	ENGINE_DLL void PrimitiveBatch_ReleaseResources();

	template<class TVertex>
	class PrimitiveBatch
	{
		static_assert(std::is_same_v<TVertex, VertexPositionColor>, "Only VertexPositionColor is supported");
	public:
		explicit PrimitiveBatch(Engine::DeviceContext* pContext, size_t maxIndices = 4096 * 3, size_t maxVertices = 4096) {}

		void Begin() {}
		void End() {}

		void Draw(Engine::PRIMITIVE_TOPOLOGY eTopology, const TVertex* pVertices, size_t iVertexCount)
		{
			PrimitiveBatch_Draw(eTopology, pVertices, iVertexCount, nullptr, 0);
		}

		void DrawIndexed(Engine::PRIMITIVE_TOPOLOGY eTopology, const uint16_t* pIndices, size_t iIndexCount, const TVertex* pVertices, size_t iVertexCount)
		{
			PrimitiveBatch_Draw(eTopology, pVertices, iVertexCount, pIndices, iIndexCount);
		}

		void DrawLine(const TVertex& v1, const TVertex& v2)
		{
			const TVertex arrVertex[2] = { v1, v2 };
			Draw(Engine::TOPOLOGY_LINELIST, arrVertex, 2);
		}

		void DrawTriangle(const TVertex& v1, const TVertex& v2, const TVertex& v3)
		{
			const TVertex arrVertex[3] = { v1, v2, v3 };
			Draw(Engine::TOPOLOGY_TRIANGLELIST, arrVertex, 3);
		}

		void DrawQuad(const TVertex& v1, const TVertex& v2, const TVertex& v3, const TVertex& v4)
		{
			static const uint16_t s_Indices[6] = { 0, 1, 2, 0, 2, 3 };
			const TVertex arrVertex[4] = { v1, v2, v3, v4 };
			DrawIndexed(Engine::TOPOLOGY_TRIANGLELIST, s_Indices, 6, arrVertex, 4);
		}
	};
}
