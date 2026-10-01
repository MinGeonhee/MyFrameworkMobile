#pragma once

//#include "Engine_Define.h"

namespace Diligent
{
	struct IRenderDevice;
	struct IDeviceContext;
	struct ITextureView;
	struct ITexture;
	struct IBuffer;
	struct IShader;
	struct IPipelineState;
	struct IShaderResourceBinding;
}

namespace Engine
{

	//char -> 문자열을 다룰때만 쓰고 정수를 다룰 때는 int8을 사용한다
	using CHAR = char;

	using int8 = signed char;
	using uint8 = unsigned char;


	using int16 = signed   short;
	using uint16 = unsigned short;

	using int32 = signed   int;
	using uint32 = unsigned int;

	/* 원본은 long 이었지만 Android(LP64)와 크기를 맞추기 위해 고정 크기 정수를 사용한다. */
	using int64 = std::int64_t;
	using uint64 = std::uint64_t;

	//ENUM포인터
	using P_ENUM = void*;

#pragma region ComPtr
	/* 원본은 Microsoft::WRL::ComPtr 를 사용했다.                                           */
	/* Android에는 WRL이 없으므로 같은 사용법(Get / GetAddressOf / Reset)을 가진 스마트포인터를  */
	/* 직접 정의한다. Diligent 객체도 AddRef / Release 규칙을 따르므로 그대로 담을 수 있다.        */
	template<class T>
	class ComPtr
	{
	public:
		using InterfaceType = T;

		ComPtr() noexcept = default;
		ComPtr(std::nullptr_t) noexcept {}
		ComPtr(T* pObject) noexcept : m_pObject(pObject) { InternalAddRef(); }
		ComPtr(const ComPtr& Other) noexcept : m_pObject(Other.m_pObject) { InternalAddRef(); }
		ComPtr(ComPtr&& Other) noexcept : m_pObject(Other.m_pObject) { Other.m_pObject = nullptr; }
		template<class U>
		ComPtr(const ComPtr<U>& Other) noexcept : m_pObject(Other.Get()) { InternalAddRef(); }
		~ComPtr() { InternalRelease(); }

		ComPtr& operator=(std::nullptr_t) noexcept { InternalRelease(); return *this; }
		ComPtr& operator=(T* pObject) noexcept
		{
			if (m_pObject != pObject)
			{
				T* pOld = m_pObject;
				m_pObject = pObject;
				InternalAddRef();
				if (pOld != nullptr) pOld->Release();
			}
			return *this;
		}
		ComPtr& operator=(const ComPtr& Other) noexcept { return operator=(Other.m_pObject); }
		ComPtr& operator=(ComPtr&& Other) noexcept
		{
			if (this != &Other)
			{
				InternalRelease();
				m_pObject = Other.m_pObject;
				Other.m_pObject = nullptr;
			}
			return *this;
		}

		T*			Get() const noexcept { return m_pObject; }
		T*			operator->() const noexcept { return m_pObject; }
		T&			operator*() const noexcept { return *m_pObject; }
		explicit	operator bool() const noexcept { return m_pObject != nullptr; }

		T* const*	GetAddressOf() const noexcept { return &m_pObject; }
		T**			GetAddressOf() noexcept { return &m_pObject; }
		T**			ReleaseAndGetAddressOf() noexcept { InternalRelease(); return &m_pObject; }

		/* WRL ComPtr 와 같이 &pCom 이 T** (생성 함수 출력) 와 ComPtr* 둘 다로 쓰일 수 있게 한다 */
		struct ComPtrRef
		{
			ComPtr* pOwner;
			operator T**() const noexcept { return pOwner->ReleaseAndGetAddressOf(); }
			operator ComPtr*() const noexcept { return pOwner; }
			operator void**() const noexcept { return reinterpret_cast<void**>(pOwner->ReleaseAndGetAddressOf()); }
		};
		ComPtrRef	operator&() noexcept { return ComPtrRef{ this }; }

		void		Reset() noexcept { InternalRelease(); }
		T*			Detach() noexcept { T* p = m_pObject; m_pObject = nullptr; return p; }
		void		Attach(T* pObject) noexcept { InternalRelease(); m_pObject = pObject; }

		bool operator==(std::nullptr_t) const noexcept { return m_pObject == nullptr; }
		bool operator!=(std::nullptr_t) const noexcept { return m_pObject != nullptr; }
		bool operator==(const ComPtr& Other) const noexcept { return m_pObject == Other.m_pObject; }
		bool operator!=(const ComPtr& Other) const noexcept { return m_pObject != Other.m_pObject; }

		friend bool operator==(std::nullptr_t, const ComPtr& Ptr) noexcept { return Ptr.m_pObject == nullptr; }
		friend bool operator!=(std::nullptr_t, const ComPtr& Ptr) noexcept { return Ptr.m_pObject != nullptr; }

	private:
		void InternalAddRef() const noexcept { if (m_pObject != nullptr) m_pObject->AddRef(); }
		void InternalRelease() noexcept
		{
			T* pTemp = m_pObject;
			if (pTemp != nullptr)
			{
				m_pObject = nullptr;
				pTemp->Release();
			}
		}
	private:
		T* m_pObject = nullptr;
	};
#pragma endregion

	/* 원본 ID3D11 별칭을 Diligent 인터페이스로 교체한다.                               */
	/* Device : 리소스 생성 , DeviceContext : 바인딩 및 Draw 명령 (DX11과 역할이 같다)       */
	using	Device = Diligent::IRenderDevice;
	using	DeviceContext = Diligent::IDeviceContext;
	using	ShaderResourceView = Diligent::ITextureView;
	using	RenderTargetView = Diligent::ITextureView;
	using	DepthStencilView = Diligent::ITextureView;
	using	Texture2D = Diligent::ITexture;
	using	Buffer = Diligent::IBuffer;
	using	VertexBuffer = Diligent::IBuffer;
	using	IndexBuffer = Diligent::IBuffer;
	using	ConstantBuffer = Diligent::IBuffer;

	using RTV = Diligent::ITextureView;

	/* D3D11 구조체 대응 */
	using VIEWPORT_DESC = Diligent::Viewport;				// D3D11_VIEWPORT	(TopLeftX/Y , Width/Height , Min/MaxDepth)
	using RECT_DESC = Diligent::Rect;						// D3D11_RECT		(left , top , right , bottom)
	using TEXTURE_FORMAT = Diligent::TEXTURE_FORMAT;		// DXGI_FORMAT
	using BLEND_DESC = Diligent::BlendStateDesc;			// D3D11_BLEND_DESC
	using DEPTH_STENCIL_DESC = Diligent::DepthStencilStateDesc;	// D3D11_DEPTH_STENCIL_DESC
	using RASTERIZER_DESC = Diligent::RasterizerStateDesc;	// D3D11_RASTERIZER_DESC
	using SAMPLER_DESC = Diligent::SamplerDesc;				// D3D11_SAMPLER_DESC
	using PRIMITIVE_TOPOLOGY = Diligent::PRIMITIVE_TOPOLOGY;	// D3D11_PRIMITIVE_TOPOLOGY

	// 수학 관련
	using Float2 = DirectX::XMFLOAT2;
	using Float3 = DirectX::XMFLOAT3;
	using Float4 = DirectX::XMFLOAT4;
	using Float4x4 = DirectX::XMFLOAT4X4;

	// 계산 전용
	/* 1 ~ 3 f  /  4 G   /  5 6 H / 7+ C */
	using Vector4 = DirectX::XMVECTOR;
	using fVector4_1 = DirectX::FXMVECTOR;
	using fVector4_2 = DirectX::FXMVECTOR;
	using fVector4_3 = DirectX::FXMVECTOR;
	using GVector4_4 = DirectX::GXMVECTOR;
	using HVector4_5 = DirectX::HXMVECTOR;
	using HVector4_6 = DirectX::HXMVECTOR;
	using CVector4_7 = DirectX::CXMVECTOR;

	using Quaternion = DirectX::XMVECTOR;

	using Color = Float4;

	/* 저장 전용 Matrix , 함수 인자 전용 XMatrix 1 , CMatrix 2+ */
	using Matrix = DirectX::XMMATRIX;
	using XMatrix_1 = DirectX::FXMMATRIX;
	using CMatrix_2 = DirectX::CXMMATRIX;

	/* SIMD 연산 -> 64 비트 환경에서 사용가능한 벡터레지스터를 사용한 vector 이다. */
	/* CPU -> 직렬연산에 특화 / 연산의 다양성  , GPU -> 병렬 연산에 특화 / 코어의 개수가 많아 단순반복 연산에 특화  */
}
