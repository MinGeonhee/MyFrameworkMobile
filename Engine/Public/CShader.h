#pragma once
#include "CResource.h"


namespace Engine
{
	FD CTexture;
	FD CResource;
	FD CConstantBufferBase;
	struct GRAPHIC_PIPELINE_STATE;
}

NS_BEGIN(Engine)

/* ------------------------------------------------------------------------------------------------ */
/* 원본 CShader 는 Effects11(ID3DX11Effect) 로 .hlsl(technique11 / pass) 파일을 컴파일했다.             */
/* Effects11 은 Windows/D3D11 전용이므로 같은 동작을 엔진 내부에서 직접 수행한다.                      */
/*                                                                                                  */
/*  1. 셰이더 파일을 읽어 technique11 / pass 블록과 RasterizerState / BlendState / DepthStencilState  */
/*     / sampler_state 블록을 파싱한다. (파일 형식은 원본 Effect 문법 그대로 사용)                     */
/*  2. 파싱한 블록을 제거한 HLSL 을 Diligent 로 컴파일한다. (Windows D3D11 , Vulkan SPIR-V)            */
/*  3. 상수버퍼 리플렉션으로 변수 이름 → (cbuffer , offset) 을 찾아 Set_GPU_* 를 원본과 같게 지원한다.  */
/*  4. Apply_Pass 는 "현재 패스" 를 GraphicDevice 에 기록하고 , 실제 PSO 는 Draw 시점에                 */
/*     현재 RenderState / RenderTarget Format / 정점 Stride 로 골라서 만든다 (캐시).                    */
/* ------------------------------------------------------------------------------------------------ */

class ENGINE_DLL CShader final : public CResource
{
public:
	struct EFFECT;		/* Effect 데이터 (복사본끼리 공유한다 : 원본 Copy_Resource 도 같은 Effect 를 공유했다) */

protected:
	explicit CShader(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CShader(const CShader& Prototype);
	virtual ~CShader() {};

protected:
	HRESULT							Shader_Initialize(const wstring& strPath, const INPUT_ELEMENT_DESC* tInputElement_Desc, uint32 iInputElementNum);
	virtual HRESULT					Copy_Initialize(void* pArg)override;
	HRESULT							Ready_GlobalEffectVariable();
	HRESULT							LoadShaderFromFile(const wstring& strPath);

public	:
	vector<string>						Get_ALlPassName();
	uint32								Get_PassCount() const;

#pragma region Set / SetGPU
	HRESULT								Set_GPU_Texture(CONST string& strFindKey ,ShaderResourceView* pTexture);
	HRESULT								Set_GPU_Texture(ShaderResourceView* pShaderResourceView);
	HRESULT								Set_GPU_Textures( uint32 iCountTex , ShaderResourceView** ppStartSRV, uint32 iOffset = 0);

	/* Diffuse Mak Binding */
	HRESULT								Set_GPU_MixTextureArr(uint32 iCountTex, ShaderResourceView** ppStartTexture, uint32 iOffset = 0 );
	HRESULT								Set_GPU_MaskTexture(ShaderResourceView* pShaderResourceView);

	/* Materal Texture Binding */
	HRESULT								Set_GPU_MaterialTextures(ShaderResourceView** ppMaterialRSV);

	/* Camera Buffer Data */
	HRESULT								Set_GPU_CameraBuffer(CONST CAM_BUF_DATA& tCamBuffer , uint32 iByteOffset = 0);

	/* Light Buffer Data */
	HRESULT								Set_GPU_LightGroupBuffer(CONST LIGHT_GROUP_BUF* tLightBuffer, uint32 iByteOffset=0);

	/* UIAlpha */
	HRESULT								Set_GPU_UIAplha(float fAlpha);
	/* DT */
	HRESULT								Set_GPU_DT(float fDT);
	HRESULT								Set_GPU_Ratio(float fRatio);

	/* Color */
	HRESULT								Set_GPU_Color(const Color& vColor);


	HRESULT								Set_GPU_ConstantBuffer(CONST string& strFindKey, const void* pCB , uint32 iCBSize ,uint32 iByteOffset = 0 );

	HRESULT								Set_GPU_Vector(CONST Float4& tVector , const string& strFindKey);
	HRESULT								Set_GPU_Vector(CONST float* fStartX, const string& strFindKey);

	/* 기본적인 bool , float , int 등등  Scalar Setting*/
	template<class Scalar>
	HRESULT								Set_GPU_Scalar(CONST string& strFindKey, Scalar _Scalar , uint32 iByteOffset = 0)
	{
		if (FAILED(Check_EffectVariable(EFFECT_VAL_TYPE::SCALAR, strFindKey)))
			return E_FAIL;

		/* HLSL 의 bool 은 4 Byte 이다 */
		if constexpr (std::is_same_v<Scalar, bool>)
		{
			const int32 iValue = _Scalar ? 1 : 0;
			return Set_RawValue(strFindKey, &iValue, sizeof(int32), 0);
		}
		else
			return Set_RawValue(strFindKey, &_Scalar, sizeof(Scalar), iByteOffset);
	}


	/* Matrix 관련 */
	HRESULT												Set_GPU_Transform(CONST Float4x4& matWorld);
	HRESULT												Set_GPU_Transform(CONST XMatrix_1& matWorld);
	HRESULT												Set_GPU_Canvas(CONST Float4x4& matCanvas);
	HRESULT												Set_GPU_Matrix(CONST string& strFindKey, CONST Float4x4& Matrix);
	HRESULT												Set_GPU_Matrices(CONST string& strFindKey, const Float4x4* pMatrix, uint32 iCountMatrix, uint32 iOffset = 0);


	HRESULT												Apply_Pass(CONST string& strPassKey , int32 iIndex = 0);

#pragma endregion
	HRESULT												Add_EffectConstantBuffer(const std::string& strKey);
	HRESULT												Add_EffectVariable(EFFECT_VAL_TYPE eValType ,const std::string& strKey);

	/* 원본 : EffectConstantBuffer->SetConstantBuffer(ID3D11Buffer*) 로 공용 상수버퍼를 연결했다 (CPipeLine) */
	HRESULT												Bind_ConstantBuffer(const std::string& strCBName, CConstantBufferBase* pConstantBuffer);
	bool												Has_ConstantBuffer(const std::string& strCBName) const;

	/* 이름으로 직접 값을 넣는다 (cbuffer 변수 / 전역 변수) */
	HRESULT												Set_RawValue(const std::string& strVarName, const void* pData, uint32 iSize, uint32 iByteOffset = 0);

#pragma region Engine 내부 (GraphicDevice 가 Draw 시점에 호출)
public:
	HRESULT												Prepare_Draw(uint32 iPassIndex, const struct GRAPHIC_PIPELINE_STATE& tState, Diligent::IPipelineState** ppPSO, Diligent::IShaderResourceBinding** ppSRB);
	int32												Find_PassIndex(const std::string& strPassKey) const;
	const std::wstring&									Get_FileName() const { return m_wstrFileName; }
	/* 엔진 종료 시 공용 기본 텍스처 해제 (GraphicDevice 해제 전에 호출) */
	static void											Release_SharedResources();
#pragma endregion

private:
	HRESULT												Check_EffectVariable(EFFECT_VAL_TYPE eValType, CONST std::string& strFindKey);

protected:
	std::shared_ptr<EFFECT>		m_pEffect;
protected:
	wstring					m_wstrFileName;
	wstring					m_strPath;
	uint32					m_iTotalPassNum;
public:
	/* Shader 파일 Path / 이 쉐이더 파일에서 사용할 정점정보를 가져오기  */
	static CShader* Create(const wstring& strPath, const INPUT_ELEMENT_DESC* pInputElement_Desc , uint32 iInputElementNum, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CResource* Copy_Resource(void* pArg) override;
	virtual void Free() override;

};

NS_END
