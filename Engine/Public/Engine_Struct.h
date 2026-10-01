#pragma once

//#include "Engine_Define.h"

//using namespace DirectX;

namespace Engine
{
	struct RenderState
	{
		class CRasterizerState* pRasterizerState{ nullptr };
		class CSamplerState* pSamplerState{ nullptr };
		class CBlendState* pBlendState{ nullptr };
		class CDepthStencilState* pDepthstencilState{ nullptr };
	};

	/* D3D11 Desc 구조체 대신 Diligent Desc 구조체를 사용한다. 기본 생성값은 D3D11_DEFAULT 와 같다. */
	struct RenderState_Desc
	{
		RASTERIZER_DESC		      tRasterizerState_Desc{};
		BLEND_DESC		          tBlendState_Desc{};
		DEPTH_STENCIL_DESC		  tDepthStencilState_Desc{};
		SAMPLER_DESC		      tSamplerSate_Desc{};
	};


#pragma region Input Layout
	/* D3D11_INPUT_ELEMENT_DESC 대응 구조체. 초기화 순서를 D3D11과 같게 두어 정점 구조체 선언을 그대로 쓴다. */
	enum INPUT_ELEMENT_FORMAT
	{
		FORMAT_R32_FLOAT,
		FORMAT_R32G32_FLOAT,
		FORMAT_R32G32B32_FLOAT,
		FORMAT_R32G32B32A32_FLOAT,
		FORMAT_R32_UINT,
		FORMAT_R32G32_UINT,
		FORMAT_R32G32B32_UINT,
		FORMAT_R32G32B32A32_UINT,
		FORMAT_R32_SINT,
		FORMAT_R32G32B32A32_SINT,
		FORMAT_R8G8B8A8_UNORM,
	};

	enum INPUT_CLASSIFICATION
	{
		INPUT_PER_VERTEX_DATA = 0,
		INPUT_PER_INSTANCE_DATA = 1,
	};

	inline constexpr unsigned int APPEND_ALIGNED_ELEMENT = 0xffffffff;

	/* D3D11_BUFFER_DESC / D3D11_SUBRESOURCE_DATA 대응 (CVIBuffer 가 Diligent 버퍼로 변환한다) */
	enum BUFFER_USAGE
	{
		BUFFER_USAGE_DEFAULT = 0,
		BUFFER_USAGE_IMMUTABLE = 1,
		BUFFER_USAGE_DYNAMIC = 2,
		BUFFER_USAGE_STAGING = 3,
	};

	enum BUFFER_BIND_FLAG
	{
		BUFFER_BIND_VERTEX_BUFFER = 0x1,
		BUFFER_BIND_INDEX_BUFFER = 0x2,
		BUFFER_BIND_CONSTANT_BUFFER = 0x4,
		BUFFER_BIND_SHADER_RESOURCE = 0x8,
		BUFFER_BIND_STREAM_OUTPUT = 0x10,
		BUFFER_BIND_RENDER_TARGET = 0x20,
		BUFFER_BIND_DEPTH_STENCIL = 0x40,
		BUFFER_BIND_UNORDERED_ACCESS = 0x80,
	};

	enum BUFFER_CPU_ACCESS_FLAG
	{
		BUFFER_CPU_ACCESS_WRITE = 0x10000,
		BUFFER_CPU_ACCESS_READ = 0x20000,
	};

	struct BUFFER_DESC
	{
		unsigned int	ByteWidth{ 0 };
		BUFFER_USAGE	Usage{ BUFFER_USAGE_DEFAULT };
		unsigned int	BindFlags{ 0 };
		unsigned int	CPUAccessFlags{ 0 };
		unsigned int	MiscFlags{ 0 };
		unsigned int	StructureByteStride{ 0 };
	};

	struct SUBRESOURCE_DATA
	{
		const void*		pSysMem{ nullptr };
		unsigned int	SysMemPitch{ 0 };
		unsigned int	SysMemSlicePitch{ 0 };
	};

	/* D3D_PRIMITIVE_TOPOLOGY 대응 */
	inline constexpr Diligent::PRIMITIVE_TOPOLOGY TOPOLOGY_POINTLIST = Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST;
	inline constexpr Diligent::PRIMITIVE_TOPOLOGY TOPOLOGY_LINELIST = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
	inline constexpr Diligent::PRIMITIVE_TOPOLOGY TOPOLOGY_LINESTRIP = Diligent::PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inline constexpr Diligent::PRIMITIVE_TOPOLOGY TOPOLOGY_TRIANGLELIST = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inline constexpr Diligent::PRIMITIVE_TOPOLOGY TOPOLOGY_TRIANGLESTRIP = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

	struct INPUT_ELEMENT_DESC
	{
		const char*				SemanticName;
		unsigned int			SemanticIndex;
		INPUT_ELEMENT_FORMAT	Format;
		unsigned int			InputSlot;
		unsigned int			AlignedByteOffset;
		INPUT_CLASSIFICATION	InputSlotClass;
		unsigned int			InstanceDataStepRate;
	};
#pragma endregion


#pragma region  Manager Desc

	struct CollisionManager_Desc
	{
		unsigned int iMaxColliderGroupID;
	};

	struct GraphicDevice_Desc
	{
		HWND			hWnd{ nullptr };		// Windows : HWND , Android : ANativeWindow*
		HINSTANCE		hInst{ nullptr };
		unsigned int	iWinSizeX{ 0 }, iWinSizeY{ 0 };
		WINMODE			eWindowMode{ WINMODE::WIN };
		GRAPHIC_API		eGraphicAPI{ GRAPHIC_API::D3D11 };
	};

	struct PrototypeManager_Desc
	{
		unsigned				iTotalSceneNum;
	};

	struct GameObjectManager_Desc
	{
		unsigned				iTotalSceneNum;
	};

#pragma endregion

	typedef struct UI_Info
	{
		float		fX{};
		float		fY{};
		float		fSizeX{};
		float		fSizeY{};
	} UI_INFO;

	typedef struct UI_RatioInfo
	{
		float		fRatioX{};
		float		fRatioY{};
		float		fRatioSizeX{};
		float		fRatioSizeY{};
	} UI_RATIO;


	struct Engine_Desc
	{
		CollisionManager_Desc	tCollisionManager_Desc;
		GameObjectManager_Desc	tGameObjectManager_Desc;
		PrototypeManager_Desc	tPrototypeManager_Desc;
		GraphicDevice_Desc		tGraphicDevice_Desc;
	};

	struct CreateScene_Desc
	{
		SCENE_ISPOP					eIsPop{ SCENE_ISPOP::NO };
		unsigned int				iMakeSceneNum = { 0 };
		bool						isNeedSceneOverLay = false;

		unsigned int				iNextSceneNum_ForLoading = { false }; // 다음으로 이동할 번호
		unsigned int				iNextSceneNum_NeedOverLay_ForLoading = { 0 }; // 다음으로 이동할 번호

		std::wstring						wstrNextScene_SceneDataPath = {};

	public:
		CreateScene_Desc()
		{

		}
		CreateScene_Desc(SCENE_ISPOP eIsPop, bool isNeedSceneOverLay = false ,unsigned int iMakeSceneNum = 0, unsigned int iNextSceneNum_ForLoading = 0, unsigned int iRoomDataJsonPath_ForRoom = 0 )
			: eIsPop(eIsPop), iMakeSceneNum(iMakeSceneNum), iNextSceneNum_ForLoading(iNextSceneNum_ForLoading), isNeedSceneOverLay(isNeedSceneOverLay), wstrNextScene_SceneDataPath{}
		{

		}
	};


	struct AccumulatedRotation
	{
		float pitch = 0.0f; // X축 회전 (상하)
		float yaw = 0.0f; // Y축 회전 (좌우)
		float roll = 0.0f; // Z축 회전 (비틀기)

		DirectX::XMFLOAT4 tQuatRot;

	public:
		AccumulatedRotation()
		{
			DirectX::XMStoreFloat4(&tQuatRot, DirectX::XMQuaternionIdentity());
		}
	};
	using ACC_ROT = AccumulatedRotation;


#pragma region Vertex Buffer

	typedef struct tagVertexPositionColor
	{
		DirectX::XMFLOAT3	vPosition;	// POSITION
		DirectX::XMFLOAT4	vColor;		// COLOR
		inline static const std::array<INPUT_ELEMENT_DESC, 2> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, FORMAT_R32G32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 }
		} };
	public:
		inline const std::array<INPUT_ELEMENT_DESC, 2>& GetInputElementDesc() { return tInputElement_Desc; }
		inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	}VTXPOSCOLOR;


	struct tagVertexPositionTexcoord
	{
		DirectX::XMFLOAT3	vPosition;	// POSITION
		DirectX::XMFLOAT2	vTexcoord; //  UV
		inline static const std::array<INPUT_ELEMENT_DESC, 2> tInputElement_Desc =
		{{
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 }
		}};
	public:
		inline const std::array<INPUT_ELEMENT_DESC, 2>& GetInputElementDesc() { return tInputElement_Desc; }
		inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	};

	using VTXPOSTEX = tagVertexPositionTexcoord;


	typedef struct VertexPosition
	{
		DirectX::XMFLOAT3	vPosition;
		inline static const std::array<INPUT_ELEMENT_DESC,1> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
		} };
	public:
		static inline const std::array<INPUT_ELEMENT_DESC,1>& GetInputElementDesc() { return tInputElement_Desc; }
		static inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	}VTXPOS;



	struct tagVertexNormalTexcoord
	{
		DirectX::XMFLOAT3	vPosition{0.f,0.f,0.f};	// POSITION
		DirectX::XMFLOAT3	vNormal{0.f,0.f,0.f};	// POSITION
		DirectX::XMFLOAT2	vTexcoord{0.f,0.f}; //  UV
		inline static const std::array<INPUT_ELEMENT_DESC, 3> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 }
		} };
	public:
		inline const std::array<INPUT_ELEMENT_DESC, 3>& GetInputElementDesc() { return tInputElement_Desc; }
		inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	};
	using VTXNORTEX = tagVertexNormalTexcoord;

	/* 기본매쉬 */
	struct tagVertexMesh
	{
		DirectX::XMFLOAT3	vPosition{ 0.f,0.f,0.f };	// POSITION
		DirectX::XMFLOAT3	vNormal{ 0.f,0.f,0.f };		// Normal
		DirectX::XMFLOAT3	vTangent{ 0.f,0.f,0.f };	// TANGENT
		DirectX::XMFLOAT3	vBinormal{ 0.f,0.f,0.f };	// BINORMAL
		DirectX::XMFLOAT2	vTexcoord{ 0.f,0.f };		// UV
		inline static const std::array<INPUT_ELEMENT_DESC, 5> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 36, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 48 , INPUT_PER_VERTEX_DATA, 0 }
		} };
	public:
		inline const std::array<INPUT_ELEMENT_DESC, 5>& GetInputElementDesc() { return tInputElement_Desc; }
		inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	};
	using VTXMESH = tagVertexMesh;

	/* Anim Mesh */

	typedef struct tagVertexAnimMesh
	{
		DirectX::XMFLOAT3	vPosition{ 0.f,0.f,0.f };	// POSITION
		DirectX::XMFLOAT3	vNormal{ 0.f,0.f,0.f };		// Normal
		DirectX::XMFLOAT3	vTangent{ 0.f,0.f,0.f };	// TANGENT
		DirectX::XMFLOAT3	vBinormal{ 0.f,0.f,0.f };	// BINORMAL
		DirectX::XMFLOAT2	vTexcoord{ 0.f,0.f };		// UV
		DirectX::XMINT4     vBlendIndex{ 0,0,0,0 };		// Blend Index => 이 정점에 Bone(뼈) 의 개수
		DirectX::XMFLOAT4   vBlendWeight{ 0.f,0.f,0.f,0.f }; // Blend Weight => 가중치를 뜻함
		inline static const std::array<INPUT_ELEMENT_DESC,7> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 36, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 48 , INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDEX", 0, FORMAT_R32G32B32A32_UINT,0,56,INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, FORMAT_R32G32B32A32_FLOAT,0,72,INPUT_PER_VERTEX_DATA, 0 }
		} };
	public:
		inline const std::array<INPUT_ELEMENT_DESC, 7>& GetInputElementDesc() { return tInputElement_Desc; }
		inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	}VTXANIMMESH;




	typedef struct VertexInstanceParticle
	{
		XMFLOAT4		vRight			{ 1.f,0.f,0.f,0.f };
		XMFLOAT4		vUp				{ 0.f,1.f,0.f,0.f };
		XMFLOAT4		vLook			{ 0.f,0.f,1.f,0.f };
		XMFLOAT4		vTranslation	{ 0.f,0.f,0.f,1.f };
		XMFLOAT2		vLifeTime		{ 0.f,0.f };
		unsigned int	iTextureFrame	{ 0 };
	}VTX_INSTANCE_PARTICLE;


	typedef struct VertexPositionTexcoord_Particle
	{
		inline static const std::array<INPUT_ELEMENT_DESC, 7> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 12, INPUT_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 1, FORMAT_R32G32B32A32_FLOAT, 1 , 0,  INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, FORMAT_R32G32B32A32_FLOAT, 1 , 16, INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, FORMAT_R32G32B32A32_FLOAT, 1 , 32, INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, FORMAT_R32G32B32A32_FLOAT, 1 , 48, INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, FORMAT_R32G32_FLOAT      , 1 , 64, INPUT_PER_INSTANCE_DATA, 1 }
		} };
	public:
		static inline const std::array<INPUT_ELEMENT_DESC,7>& GetInputElementDesc() { return tInputElement_Desc; }
		static inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	}VTXPOSTEX_PARTICLE;


	typedef struct VertexPositionParticle
	{
		inline static const std::array<INPUT_ELEMENT_DESC, 7> tInputElement_Desc =
		{ {
			{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_PER_VERTEX_DATA, 0 },

			{ "WORLD",0, FORMAT_R32G32B32A32_FLOAT, 1 , 0, INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD",1, FORMAT_R32G32B32A32_FLOAT, 1 , 16, INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD",2, FORMAT_R32G32B32A32_FLOAT, 1 , 32, INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD",3, FORMAT_R32G32B32A32_FLOAT, 1 , 48, INPUT_PER_INSTANCE_DATA, 1 },

			{ "TEXCOORD",0, FORMAT_R32G32_FLOAT   , 1 , 64, INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD",1, FORMAT_R32_UINT      , 1 , 72, INPUT_PER_INSTANCE_DATA, 1 }
		} };
	public:
		static inline const std::array<INPUT_ELEMENT_DESC, 7>& GetInputElementDesc() { return tInputElement_Desc; }
		static inline unsigned int GetInputElementDescSize() { return (unsigned int)tInputElement_Desc.size(); }
	}VTXPOS_PARTICLE;





#pragma endregion

#pragma region GameObject, Compoenent Desc

	typedef struct TransformCom_Desc
	{
		float			fSpeedPerSec{}, fRotationPerSec{};

		DirectX::XMFLOAT3			tScale{ 1.f,1.f,1.f };
		DirectX::XMFLOAT3			tRotation{ 0.f,0.f,0.f };
		DirectX::XMFLOAT3			tTransform{ 0.f,0.f,0.f };
	}TRANSFORMCOM_DESC;

	typedef struct GameObject_Desc : public TRANSFORMCOM_DESC
	{
		/* GameObject Arg 를 위한 구조체들을 담아두고 있을 예정 */
		std::wstring				GameObect_Desc_strGameObjectName{ L"NONE" };
		int							iEventIndex{ -1 };
		/* 하위 GameObject객체 이 Desc를 상속받아서 자체적으로 Desc를 만들게 되면된다.*/
	}GAMEOBJECT_DESC;

#pragma endregion


#pragma region Collision Info 

	typedef struct CollisionInfo
	{
		/* 충돌처리 관련... */
		unsigned int	   iSrcGroupIndex{0};
		unsigned int	   iDstGroupIndex{0};
		class CGameObject* pSrcGameObject{nullptr};
		class CGameObject* pDstGameObject{nullptr};

		class CCollider*   pSrcCollider{nullptr};
		class CCollider*   pDstCollider{nullptr};

		/* Src 가 밀려나야할 방향벡터 */
		/* Dst 가 밀려나야할 방향벡터 */
		XMFLOAT3		   vSrcPenetrationDir{};
		XMFLOAT3		   vDstPenetrationDir{};

		float			   fDepth{0.f};
	public:
		CollisionInfo Swap()
		{
			CollisionInfo tSwapColliderInfo{};

			tSwapColliderInfo.iSrcGroupIndex = this->iDstGroupIndex;
			tSwapColliderInfo.iDstGroupIndex = this->iSrcGroupIndex;

			tSwapColliderInfo.pSrcGameObject = this->pDstGameObject;
			tSwapColliderInfo.pDstGameObject = this->pSrcGameObject;

			tSwapColliderInfo.vSrcPenetrationDir = this->vDstPenetrationDir;
			tSwapColliderInfo.vDstPenetrationDir = this->vSrcPenetrationDir;

			tSwapColliderInfo.fDepth = this->fDepth;

			return tSwapColliderInfo;
		}
	}COLLISION_INFO;

	typedef struct ColliderCom_Desc
	{
		std::wstring	wstrFindKeyName{};	// Key값을 찾고싶으면 이름을 넣고 아니면 아래 Default값을 채워준다.
		COLLIDER_TYPE	eColliderType{ COLLIDER_TYPE::AABB };
	}COLLIDERCOM_DESC;


	/* 정점 관련된 데이터를 넘겨준다 */
	typedef struct Collider_BufferBox
	{
		XMFLOAT4 tCenter{1.f,1.f,1.f,1.f};       // xyz = center
		XMFLOAT4 tExtents{1.f,1.f,1.f,0.f};       // xyz = half size
		XMFLOAT4 tColor{1.f,0.f,0.f,1.f };
	}COLLIDER_BUFFER_BOX;

	/* 정점 관련된 데이터를 넘겨준다 */
	typedef struct Collider_BufferRotBox
	{
		XMFLOAT4X4 matColliderRotBoxWorld{};
		XMFLOAT4 tColor{ 1.f,0.f,0.f,1.f };
	}COLLIDER_BUFFER_ROTBOX;

	typedef struct Collider_BufferSphere
	{
		XMFLOAT4 tOffset{ 1.f,1.f,1.f,1.f };       // xyz = center
		XMFLOAT4 tCenter{ 1.f,1.f,1.f,1.f };
		XMFLOAT4 tColor{ 1.f,0.f,0.f,1.f };
	}COLLIDER_BUFFER_SPHERE;


	/* 콜라이더 */

#pragma endregion


#pragma region Ray

	typedef struct RayInfo
	{
		class CGameObject*		pRayShotGameObject{nullptr};
		DirectX::XMFLOAT3		vRayOrigin{0.f,0.f,0.f};
		DirectX::XMFLOAT3		vRayDir{ 0.f,0.f,0.f };
		RAY_TYPE	eRayType{ RAY_TYPE::END};
	}RAY_INFO;

	/* 아마 맵툴에서 사용을 제일많이 할듯하다... 쏘는 주체가 없 */
	typedef struct RayHit
	{
		class CGameObject* pRayShotGameObject{ nullptr };
		class CGameObject* pRayHitGameObject{ nullptr };
		bool						isHit{false};
		float						fDistance{0.f};
		DirectX::XMFLOAT3	vHitPos{ 0.f,0.f,0.f };
		DirectX::XMFLOAT3	vTriPos[3];
		DirectX::XMFLOAT3   vTriNormal[3];
		unsigned int			iTriIndex[3];
	public:
		RayHit()
			: vHitPos{}, isHit(false)
		{
			ZeroMemory(vTriPos, sizeof(DirectX::XMFLOAT3)*3);
			ZeroMemory(vTriNormal, sizeof(DirectX::XMFLOAT3)*3);
			ZeroMemory(iTriIndex, sizeof(unsigned int)*3);
		}
		/* 레이가 맞았을떄 넣어줄 Info  */
		void SetHitPos(const RAY_INFO& tRayInfo , float fDist)
		{
			fDistance = fDist;
			
			DirectX::XMVECTOR vRayPos = DirectX::XMLoadFloat3(&tRayInfo.vRayOrigin);
			DirectX::XMVECTOR vRayDir = DirectX::XMLoadFloat3(&tRayInfo.vRayDir);

			DirectX::XMStoreFloat3(&vHitPos, vRayPos + DirectX::XMVector3Normalize(vRayDir) * fDistance);

			if (tRayInfo.pRayShotGameObject != nullptr)
				pRayShotGameObject = tRayInfo.pRayShotGameObject;
		}
		void SetRayHit(const RAY_INFO& tRayInfo,float fDist, const DirectX::FXMVECTOR& vPos1, const DirectX::FXMVECTOR& vPos2, const DirectX::FXMVECTOR& vPos3
		 , unsigned int* pStartIndex,bool _isHit = true)
		{
			DirectX::XMVECTOR vRayPos = DirectX::XMLoadFloat3(&tRayInfo.vRayOrigin);
			DirectX::XMVECTOR vRayDir = DirectX::XMLoadFloat3(&tRayInfo.vRayDir);

			isHit = _isHit;
			fDistance = fDist;
			DirectX::XMStoreFloat3(&vHitPos, vRayPos + DirectX::XMVector3Normalize(vRayDir) * fDist);

			memcpy(&vTriPos[0], &vPos1, sizeof(float) * 3);
			memcpy(&vTriPos[1], &vPos2, sizeof(float) * 3);
			memcpy(&vTriPos[2], &vPos3, sizeof(float) * 3);

			memcpy(&iTriIndex,pStartIndex,sizeof(unsigned int) * 3);

			if (tRayInfo.pRayShotGameObject != nullptr)
				pRayShotGameObject = tRayInfo.pRayShotGameObject;
		}
		/* 노말이 존재할땐 반드시 Setting해주기 */
		void		Set_Normal(const XMFLOAT3& vNormal1, const XMFLOAT3& vNormal2, const XMFLOAT3& vNormal3)
		{
			vTriNormal[0] = vNormal1;
			vTriNormal[1] = vNormal2;
			vTriNormal[2] = vNormal3;
		}
		RayHit	Get_WorldRayHit(const XMMATRIX& WorldMatrix)
		{
			//DirectX::XMFLOAT3	vTriPos[3];
			//DirectX::XMFLOAT3	vTriNormal[3];
			/* Normal의 회전이나 이런것도 필요하기때문에 같이 월드좌표 곱해주고 Normalize할예정 */
			RayHit tWorldRayHit;

			for (int i = 0; i < 3; i++)
			{
				XMStoreFloat3(&tWorldRayHit.vTriPos[i], XMVector3Transform(XMLoadFloat3(&vTriPos[i]), WorldMatrix));
				XMStoreFloat3(&tWorldRayHit.vTriNormal[i], XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vTriNormal[i]), WorldMatrix)));
			}
			XMStoreFloat3(&tWorldRayHit.vHitPos, XMVector3Transform(XMLoadFloat3(&vHitPos), WorldMatrix));

			tWorldRayHit.isHit = isHit;


			return tWorldRayHit;
		}


	}RAY_HIT;
#pragma endregion

#pragma region CBufferData



#pragma region CameraBufferData;

	typedef struct CameraShaderInfo
	{
		float		fStartX		{ 0.f };
		float		fStartY		{ 0.f };
		float		fWidth		{ 0.f };
		float		fHeight		{ 0.f };
		float		fFar		{ 0.f };
		float		fNear		{ 0.f };
		float		fFov		{ 0.f };
		float	    fCamIndex	{ 0.f };	/* A-plan : which camera drew this pixel */
	}CAM_SHADER_INFO;

	/* 부모는 함수만 있기때문에 static이랑 구조체 사이즈에 들어가지않는다. */
	typedef struct CameraBufferData
	{
		DirectX::XMFLOAT4X4		matView;
		DirectX::XMFLOAT4X4		matProj;
		DirectX::XMFLOAT4X4		matViewProj;
		DirectX::XMFLOAT4X4		matViewInv;
		DirectX::XMFLOAT4X4		matProjInv;
		DirectX::XMFLOAT4		tCamPosition{0.f,0.f ,0.f ,0.f};
		CAM_SHADER_INFO			tCamInfo{};
	public:
		CameraBufferData()
			: tCamPosition{}
		{
			DirectX::XMStoreFloat4x4(&matView, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&matProj, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&matViewProj, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&matViewInv, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&matProjInv, DirectX::XMMatrixIdentity());
		}
	} CAM_BUF_DATA;


	typedef struct Camera_Info
	{
		/* Viewport 정보 저장 */
		VIEWPORT_DESC	tViewPortDesc{};
		/* 카메라 기본정보 Fov , Near/Far */
		float fFov{ 60.f };
		float fNear{ 0.1f }, fFar{ 1000.f };
	}CAM_INFO;

#pragma endregion


#pragma region LightBuffer;
	typedef struct alignas(16)  Light_Buffer_Data
	{
		// 기본
		XMFLOAT4	vDiffuse;
		// 반사광
		XMFLOAT4	vAmbient;
		// 하이라이트 표시용
		XMFLOAT4	vSpecular;
		//방향
		XMFLOAT4	vDirection;
		//위치
		XMFLOAT4	vPosition;

		float		fRange;

		XMFLOAT3	Dummy;
	public:
		Light_Buffer_Data()
			: vDiffuse{ 0.f , 1.f, 0.f, 1.f } , vAmbient{ 1.f , 1.f, 1.f, 1.f }, vSpecular{ 1.f , 1.f, 1.f, 1.f }, vDirection{ 1.f, -1.f, 1.f, 0.f }
			, vPosition{0.f,0.f,0.f,1.f} , fRange(1.f) , Dummy{0.f,0.f,0.f}
		{
		}
	}LIGHT_BUF_DATA;

	/* 실직적으로 맵핑할껀 이거 */
	typedef struct alignas(16)  Light_Group_Buffer
	{
		Light_Buffer_Data		tGlobalLightBuffer{};
		Light_Buffer_Data		arrPointLightBuffer[MAX_LIGHTS];
		int						iPointLightNum{0};
		XMINT3					iDummy;

	public:
		Light_Group_Buffer()
			: tGlobalLightBuffer(), iPointLightNum{ 0 }, iDummy{0,0,0}
		{
			for (int i = 0; i < MAX_LIGHTS; ++i)
				arrPointLightBuffer[i] = Light_Buffer_Data();
		}
	}LIGHT_GROUP_BUF;



#pragma endregion


	typedef struct MouseBufferData
	{
		XMFLOAT3	vMouseWorldPosition{0.f,0.f ,0.f};
		bool		isMouse{false};
	}MOUSE_BUF_DATA;

#pragma endregion

	typedef struct Material_Path
	{
		std::wstring wstrDiffusePath{L""};
		std::wstring wstrAmbientPath{ L"" };
		std::wstring wstrNormalPath{ L"" };
		std::wstring wstrSpecularPath{ L"" };
		std::wstring wstrEmissivePath{ L"" };
		std::wstring wstrMetalPath{ L"" };
	}MTL_PATH;



#pragma region Floder File
	typedef struct FolderInfo
	{
		std::wstring	wstrFloderFullPath{};
		std::wstring	wstrFloderName{};
		unsigned int	iCountFloders{0};
		unsigned int	iCountFiles{0};
	}FLODER_INFO;
	/* 파일 관련 */
	typedef struct FileInfo
	{
		/* 전체 경로 */
		std::wstring wstrFileFullPath{};
		std::wstring wstrFileNameEXT{};
		std::wstring wstrFileName{};
		std::wstring wstrFileEXT{};

	public:
		FileInfo()
			: wstrFileFullPath{} , wstrFileEXT{ } , wstrFileName{}
		{

		}
		FileInfo(const std::wstring& wstrPath)
		{
			std::filesystem::path psFile(wstrPath);

			wstrFileFullPath = psFile.wstring();
			wstrFileNameEXT = psFile.filename().wstring();
			wstrFileName = psFile.filename().stem().wstring();
			wstrFileEXT = psFile.extension().wstring();
			return;
		}
		FileInfo(const std::filesystem::path& Path)
			: wstrFileFullPath(Path.wstring()), wstrFileName(Path.filename().stem().wstring()), wstrFileEXT(Path.extension().wstring())
			, wstrFileNameEXT(Path.filename().wstring())
		{
		}
	}FILE_INFO;

#pragma endregion




#pragma region Model 관련
	typedef struct Bone_Info
	{
		char szName[MAX_PATH]; //Bone을 찾기위한 Bone 이름이 필요하다
		XMFLOAT4X4 matTransform{};
		XMFLOAT4X4 matCombinedTransformation = { 1.f, 0.f , 0.f , 0.f  , 0.f , 1.f, 0.f ,0.f , 0.f ,0.f , 1.f, 0.f , 0.f ,0.f ,0.f ,1.f }; // 내 부모의 matrix 정보 굳이 필요없긴한데 Identity로 만들어버리자
		int		   iParentIndex; // 내 보모의 Index번호 이건내가 저장시킬 Index번호라고생각하면된다 Vector size를 통해서에 들어갈 번호

	}BONE_INFO;


	typedef struct MeshConnectedBoneInfo
	{
		unsigned int iCountConnectedBone{0};
		std::vector<unsigned int> vecConnectedBoneID{};
		std::vector<XMFLOAT4X4>   vecConnectedOffsetMatrix{};
	}MESHCONNECTEDBONE_INFO;


	typedef struct VI_Info
	{
		unsigned int	 iVertexCnt{ 0 };
		unsigned int	 iIndexCnt{ 0 };
		std::string		 strVertexDataPath{};
		std::string		 strIndexDataPath{};

	}VI_INFO;


	/* 각 프레임 Key 별로 변환될 SRT를 들고있는다..이게 파싱하고도 필요할까??
		이미 다 곱해져서 들어올텐데....*/
	typedef struct Key_Frame
	{
		XMFLOAT3	S{};
		XMFLOAT4	R{};
		XMFLOAT3	T{};

		/* 이 뼈가 Duration 위에 어느 위치에 있는지? */
		float		fTrackPosition{};

	public:
		XMMATRIX GetMatrix()
		{
			return XMMatrixMultiply( XMMatrixMultiply(XMMatrixScalingFromVector(XMLoadFloat3(&S)), XMMatrixRotationQuaternion(XMLoadFloat4(&R))) , XMMatrixTranslationFromVector(XMVectorSetW( XMLoadFloat3(&T) ,1.f)));
		}

	}KEY_FRAME;

	/* Channel 정보*/
	typedef struct Channel_Info
	{
		unsigned int			iConnectedBoneIndex{};

		unsigned int			iCountKeyFrame{};
		std::vector<KEY_FRAME>  vecKeyFrame{};
	}CHANNEL_INFO;

	/* Animaiton 정보를 채우기 위한 Info */
	typedef struct Animation_Info
	{
		char	szName[MAX_PATH]{}; // 애니매이션 이름
		float   fDuration{};		// 애니매이션 전체길이
		float	fTickPerSecond{};	// 애니매이션의 속도

		unsigned int				iCountChannel{};
		/* Channel에 대한 정보들을 벡터로 담아서 Animation Create인자로넘겨줄예정*/
		std::vector<CHANNEL_INFO>	vecChannelInfo{};

	}ANIM_INFO;

	/* Cell 정보 */
	typedef struct Cell_Info
	{
		/* 삼각형 셀의 포지션 값 */
		XMFLOAT3		vPosition[Cast_Uint(CELL_POINT::END)]{};

		XMFLOAT3		vCenterPos{};

		/* 삼각형 각 셀의 AB ,BC ,CA의 법선벡터 */
		XMFLOAT3		vNormal[Cast_Uint(CELL_LINE::END)]{};
		/* 삼각형의 평면 */
		XMFLOAT4		vPlane{};
		/* Cell 의 인덱스 번호 */
		int				iIndex{};
		/* 삼각형 오륜 */
		int				iNeighborIndex[Cast_Uint(CELL_LINE::END)]{};

		unsigned int	iCellType{};

		bool			isActive{};
	}CELL_INFO;


	typedef struct UITransform_Info
	{
		XMFLOAT2  vSize				{ 0.f,0.f };
		XMFLOAT2  vPosition			{ 0.f,0.f };
		XMFLOAT2  vAnchor			{ 0.5f,0.5f };
		XMFLOAT2  vPivot			{ 0.5f,0.5f };			
	}UITRANSFORM_INFO;


#pragma endregion
}