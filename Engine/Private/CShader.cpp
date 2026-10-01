#include "CShader.h"
#include "CTexture.h"
#include "CGameManager.h"
#include "CGraphicDevice.h"
#include "CConstantBuffer.h"

#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceVariable.h"

#include <regex>
#include <cctype>

/* ================================================================================================ */
/*                                  Effect 데이터 (Effects11 대체)                                    */
/* ================================================================================================ */

namespace
{
	/* 생성된 전역 변수 cbuffer 이름 (Effects11 의 $Globals 역할) */
	constexpr const char* GLOBALS_CB_NAME = "_Globals_";

	std::string ToLower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return str;
	}

	std::string Trim(const std::string& str)
	{
		size_t iStart = 0, iEnd = str.size();
		while (iStart < iEnd && std::isspace(static_cast<unsigned char>(str[iStart]))) ++iStart;
		while (iEnd > iStart && std::isspace(static_cast<unsigned char>(str[iEnd - 1]))) --iEnd;
		return str.substr(iStart, iEnd - iStart);
	}

	bool Is_IdentChar(char c)
	{
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}

	/* iOpen 위치의 여는 괄호에 맞는 닫는 괄호 위치 */
	size_t Find_MatchingBrace(const std::string& strText, size_t iOpen, char cOpen = '{', char cClose = '}')
	{
		int iDepth = 0;
		for (size_t i = iOpen; i < strText.size(); ++i)
		{
			if (strText[i] == cOpen) ++iDepth;
			else if (strText[i] == cClose)
			{
				--iDepth;
				if (iDepth == 0)
					return i;
			}
		}
		return std::string::npos;
	}

	/* 주석 제거 (줄 수는 유지) */
	std::string Strip_Comments(const std::string& strSrc)
	{
		std::string strOut;
		strOut.reserve(strSrc.size());
		for (size_t i = 0; i < strSrc.size(); ++i)
		{
			if (strSrc[i] == '/' && i + 1 < strSrc.size() && strSrc[i + 1] == '/')
			{
				while (i < strSrc.size() && strSrc[i] != '\n') ++i;
				if (i < strSrc.size()) strOut.push_back('\n');
			}
			else if (strSrc[i] == '/' && i + 1 < strSrc.size() && strSrc[i + 1] == '*')
			{
				i += 2;
				while (i + 1 < strSrc.size() && !(strSrc[i] == '*' && strSrc[i + 1] == '/'))
				{
					if (strSrc[i] == '\n') strOut.push_back('\n');
					++i;
				}
				++i;
				strOut.push_back(' ');
			}
			else if (strSrc[i] == '\r')
				continue;
			else
				strOut.push_back(strSrc[i]);
		}
		return strOut;
	}

	/* #include "..." 를 직접 펼친다 (Effect 상태 블록이 include 파일에 정의되어 있기 때문) */
	bool Expand_Includes(const std::filesystem::path& FilePath, std::string& strOut, std::set<std::string>& setIncluded, int iDepth = 0)
	{
		if (iDepth > 16)
			return false;

		const std::string strKey = Engine::To_AssetPath(FilePath.wstring());
		if (setIncluded.count(strKey) != 0)
			return true;
		setIncluded.insert(strKey);

		std::string strSource;
		if (!Engine::File_ReadText(FilePath.wstring(), strSource))
		{
			Engine::Engine_Log("[Shader] File not found : %s", WString_To_UTF8(FilePath.wstring()).c_str());
			return false;
		}
		strSource = Strip_Comments(strSource);

		std::stringstream ssIn(strSource);
		std::string strLine;
		while (std::getline(ssIn, strLine))
		{
			const std::string strTrim = Trim(strLine);
			if (strTrim.rfind("#include", 0) == 0)
			{
				const size_t iQ0 = strTrim.find_first_of("\"<");
				const size_t iQ1 = strTrim.find_last_of("\">");
				if (iQ0 != std::string::npos && iQ1 != std::string::npos && iQ1 > iQ0)
				{
					const std::string strInclude = strTrim.substr(iQ0 + 1, iQ1 - iQ0 - 1);
					std::filesystem::path IncludePath = FilePath.parent_path() / std::filesystem::path(strInclude);
					if (!Expand_Includes(IncludePath, strOut, setIncluded, iDepth + 1))
						return false;
					strOut += "\n";
					continue;
				}
			}
			if (strTrim.rfind("#pragma once", 0) == 0)
			{
				strOut += "\n";
				continue;
			}
			strOut += strLine;
			strOut += "\n";
		}
		return true;
	}

	/* 숫자 리터럴들을 순서대로 뽑는다 (기본값 초기화용) */
	std::vector<double> Parse_Numbers(const std::string& strInit)
	{
		std::vector<double> vecNumbers;
		static const std::regex NumberRegex(R"((true|false)|([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?))");
		for (std::sregex_iterator Iter(strInit.begin(), strInit.end(), NumberRegex), End; Iter != End; ++Iter)
		{
			const std::smatch& Match = *Iter;
			if (Match[1].matched)
				vecNumbers.push_back(Match[1].str() == "true" ? 1.0 : 0.0);
			else
			{
				/* float4(...) 의 4 처럼 타입 이름 안의 숫자는 제외한다 */
				const size_t iPos = static_cast<size_t>(Match.position(0));
				if (iPos > 0 && Is_IdentChar(strInit[iPos - 1]))
					continue;
				vecNumbers.push_back(std::stod(Match[2].str()));
			}
		}
		return vecNumbers;
	}

	bool Is_IntegerType(const std::string& strType)
	{
		const std::string strLower = ToLower(strType);
		return strLower.rfind("int", 0) == 0 || strLower.rfind("uint", 0) == 0 || strLower.rfind("bool", 0) == 0 || strLower.rfind("dword", 0) == 0;
	}

	bool Is_ResourceType(const std::string& strType)
	{
		static const char* pResourceTypes[] = {
			"Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture3D", "TextureCube", "TextureCubeArray",
			"Texture2DMS", "Texture2DMSArray", "Buffer", "StructuredBuffer", "ByteAddressBuffer",
			"RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D", "RWBuffer",
			"RWStructuredBuffer", "RWByteAddressBuffer", "SamplerState", "SamplerComparisonState", "sampler", "texture",
		};
		std::string strBase = strType;
		const size_t iAngle = strBase.find('<');
		if (iAngle != std::string::npos)
			strBase = strBase.substr(0, iAngle);
		strBase = Trim(strBase);
		for (const char* pType : pResourceTypes)
			if (strBase == pType)
				return true;
		return false;
	}

	/* ": register(...)" 제거 */
	std::string Remove_Register(const std::string& str)
	{
		static const std::regex RegisterRegex(R"(:\s*register\s*\([^)]*\))");
		return std::regex_replace(str, RegisterRegex, "");
	}

	/* -------- Effect 상태 값 파싱 -------- */
	bool Parse_Bool(const std::string& strValue)
	{
		const std::string strLower = ToLower(Trim(strValue));
		return strLower == "true" || strLower == "1";
	}

	Diligent::BLEND_FACTOR Parse_BlendFactor(const std::string& strValue)
	{
		const std::string s = ToLower(Trim(strValue));
		if (s == "zero") return Diligent::BLEND_FACTOR_ZERO;
		if (s == "one") return Diligent::BLEND_FACTOR_ONE;
		if (s == "src_color") return Diligent::BLEND_FACTOR_SRC_COLOR;
		if (s == "inv_src_color") return Diligent::BLEND_FACTOR_INV_SRC_COLOR;
		if (s == "src_alpha") return Diligent::BLEND_FACTOR_SRC_ALPHA;
		if (s == "inv_src_alpha") return Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		if (s == "dest_alpha") return Diligent::BLEND_FACTOR_DEST_ALPHA;
		if (s == "inv_dest_alpha") return Diligent::BLEND_FACTOR_INV_DEST_ALPHA;
		if (s == "dest_color") return Diligent::BLEND_FACTOR_DEST_COLOR;
		if (s == "inv_dest_color") return Diligent::BLEND_FACTOR_INV_DEST_COLOR;
		if (s == "src_alpha_sat") return Diligent::BLEND_FACTOR_SRC_ALPHA_SAT;
		if (s == "blend_factor") return Diligent::BLEND_FACTOR_BLEND_FACTOR;
		if (s == "inv_blend_factor") return Diligent::BLEND_FACTOR_INV_BLEND_FACTOR;
		if (s == "src1_color") return Diligent::BLEND_FACTOR_SRC1_COLOR;
		if (s == "inv_src1_color") return Diligent::BLEND_FACTOR_INV_SRC1_COLOR;
		if (s == "src1_alpha") return Diligent::BLEND_FACTOR_SRC1_ALPHA;
		if (s == "inv_src1_alpha") return Diligent::BLEND_FACTOR_INV_SRC1_ALPHA;
		return Diligent::BLEND_FACTOR_ONE;
	}

	Diligent::BLEND_OPERATION Parse_BlendOp(const std::string& strValue)
	{
		const std::string s = ToLower(Trim(strValue));
		if (s == "subtract") return Diligent::BLEND_OPERATION_SUBTRACT;
		if (s == "rev_subtract") return Diligent::BLEND_OPERATION_REV_SUBTRACT;
		if (s == "min") return Diligent::BLEND_OPERATION_MIN;
		if (s == "max") return Diligent::BLEND_OPERATION_MAX;
		return Diligent::BLEND_OPERATION_ADD;
	}

	Diligent::COMPARISON_FUNCTION Parse_Comparison(const std::string& strValue)
	{
		const std::string s = ToLower(Trim(strValue));
		if (s == "never") return Diligent::COMPARISON_FUNC_NEVER;
		if (s == "less") return Diligent::COMPARISON_FUNC_LESS;
		if (s == "equal") return Diligent::COMPARISON_FUNC_EQUAL;
		if (s == "less_equal") return Diligent::COMPARISON_FUNC_LESS_EQUAL;
		if (s == "greater") return Diligent::COMPARISON_FUNC_GREATER;
		if (s == "not_equal") return Diligent::COMPARISON_FUNC_NOT_EQUAL;
		if (s == "greater_equal") return Diligent::COMPARISON_FUNC_GREATER_EQUAL;
		if (s == "always") return Diligent::COMPARISON_FUNC_ALWAYS;
		return Diligent::COMPARISON_FUNC_LESS;
	}

	Diligent::STENCIL_OP Parse_StencilOp(const std::string& strValue)
	{
		const std::string s = ToLower(Trim(strValue));
		if (s == "zero") return Diligent::STENCIL_OP_ZERO;
		if (s == "replace") return Diligent::STENCIL_OP_REPLACE;
		if (s == "incr_sat") return Diligent::STENCIL_OP_INCR_SAT;
		if (s == "decr_sat") return Diligent::STENCIL_OP_DECR_SAT;
		if (s == "invert") return Diligent::STENCIL_OP_INVERT;
		if (s == "incr") return Diligent::STENCIL_OP_INCR_WRAP;
		if (s == "decr") return Diligent::STENCIL_OP_DECR_WRAP;
		return Diligent::STENCIL_OP_KEEP;
	}

	Diligent::TEXTURE_ADDRESS_MODE Parse_AddressMode(const std::string& strValue)
	{
		const std::string s = ToLower(Trim(strValue));
		if (s == "clamp") return Diligent::TEXTURE_ADDRESS_CLAMP;
		if (s == "mirror") return Diligent::TEXTURE_ADDRESS_MIRROR;
		if (s == "mirror_once") return Diligent::TEXTURE_ADDRESS_MIRROR_ONCE;
		if (s == "border") return Diligent::TEXTURE_ADDRESS_BORDER;
		return Diligent::TEXTURE_ADDRESS_WRAP;
	}

	void Parse_Filter(const std::string& strValue, Diligent::SamplerDesc& tDesc)
	{
		const std::string s = ToLower(Trim(strValue));
		auto Filter = [](bool isLinear) { return isLinear ? Diligent::FILTER_TYPE_LINEAR : Diligent::FILTER_TYPE_POINT; };

		if (s.find("anisotropic") != std::string::npos)
		{
			tDesc.MinFilter = tDesc.MagFilter = tDesc.MipFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
			tDesc.MaxAnisotropy = 16;
			return;
		}
		/* MIN_MAG_MIP_LINEAR / MIN_LINEAR_MAG_MIP_POINT ... */
		const bool isAllLinear = s.find("min_mag_mip_linear") != std::string::npos;
		const bool isAllPoint = s.find("min_mag_mip_point") != std::string::npos;
		if (isAllLinear || isAllPoint)
		{
			tDesc.MinFilter = tDesc.MagFilter = tDesc.MipFilter = Filter(isAllLinear);
			return;
		}
		const bool isMinLinear = s.find("min_linear") != std::string::npos;
		const bool isMagLinear = s.find("mag_linear") != std::string::npos || s.find("min_mag_linear") != std::string::npos;
		const bool isMipLinear = s.find("mip_linear") != std::string::npos;
		tDesc.MinFilter = Filter(isMinLinear || s.find("min_mag_linear") != std::string::npos);
		tDesc.MagFilter = Filter(isMagLinear);
		tDesc.MipFilter = Filter(isMipLinear);
	}

	/* "Key[Index] = Value" 를 나눈다 */
	struct STATE_ASSIGN
	{
		std::string strKey;
		int         iIndex = -1;
		std::string strValue;
	};

	std::vector<STATE_ASSIGN> Parse_StateAssignments(const std::string& strBody)
	{
		std::vector<STATE_ASSIGN> vecAssign;
		std::stringstream ss(strBody);
		std::string strStatement;
		while (std::getline(ss, strStatement, ';'))
		{
			const size_t iEqual = strStatement.find('=');
			if (iEqual == std::string::npos)
				continue;
			STATE_ASSIGN tAssign{};
			std::string strKey = Trim(strStatement.substr(0, iEqual));
			const size_t iBracket = strKey.find('[');
			if (iBracket != std::string::npos)
			{
				tAssign.iIndex = std::atoi(strKey.substr(iBracket + 1).c_str());
				strKey = Trim(strKey.substr(0, iBracket));
			}
			tAssign.strKey = ToLower(strKey);
			tAssign.strValue = Trim(strStatement.substr(iEqual + 1));
			vecAssign.push_back(tAssign);
		}
		return vecAssign;
	}

	Diligent::RasterizerStateDesc Parse_RasterizerState(const std::string& strBody)
	{
		/* D3D11 기본값 */
		Diligent::RasterizerStateDesc tDesc{};
		tDesc.FillMode = Diligent::FILL_MODE_SOLID;
		tDesc.CullMode = Diligent::CULL_MODE_BACK;
		tDesc.FrontCounterClockwise = false;
		tDesc.DepthClipEnable = true;
		for (const STATE_ASSIGN& tAssign : Parse_StateAssignments(strBody))
		{
			const std::string strValue = ToLower(tAssign.strValue);
			if (tAssign.strKey == "fillmode")
				tDesc.FillMode = strValue == "wireframe" ? Diligent::FILL_MODE_WIREFRAME : Diligent::FILL_MODE_SOLID;
			else if (tAssign.strKey == "cullmode")
				tDesc.CullMode = strValue == "none" ? Diligent::CULL_MODE_NONE : (strValue == "front" ? Diligent::CULL_MODE_FRONT : Diligent::CULL_MODE_BACK);
			else if (tAssign.strKey == "frontcounterclockwise")
				tDesc.FrontCounterClockwise = Parse_Bool(strValue);
			else if (tAssign.strKey == "depthbias")
				tDesc.DepthBias = std::atoi(strValue.c_str());
			else if (tAssign.strKey == "slopescaleddepthbias")
				tDesc.SlopeScaledDepthBias = static_cast<float>(std::atof(strValue.c_str()));
			else if (tAssign.strKey == "depthclipenable")
				tDesc.DepthClipEnable = Parse_Bool(strValue);
			else if (tAssign.strKey == "scissorenable")
				tDesc.ScissorEnable = Parse_Bool(strValue);
			else if (tAssign.strKey == "antialiasedlineenable")
				tDesc.AntialiasedLineEnable = Parse_Bool(strValue);
		}
		return tDesc;
	}

	Diligent::DepthStencilStateDesc Parse_DepthStencilState(const std::string& strBody)
	{
		Diligent::DepthStencilStateDesc tDesc{};
		tDesc.DepthEnable = true;
		tDesc.DepthWriteEnable = true;
		tDesc.DepthFunc = Diligent::COMPARISON_FUNC_LESS;
		for (const STATE_ASSIGN& tAssign : Parse_StateAssignments(strBody))
		{
			const std::string& strKey = tAssign.strKey;
			const std::string strValue = ToLower(tAssign.strValue);
			if (strKey == "depthenable") tDesc.DepthEnable = Parse_Bool(strValue);
			else if (strKey == "depthwritemask") tDesc.DepthWriteEnable = strValue != "zero";
			else if (strKey == "depthfunc") tDesc.DepthFunc = Parse_Comparison(strValue);
			else if (strKey == "stencilenable") tDesc.StencilEnable = Parse_Bool(strValue);
			else if (strKey == "stencilreadmask") tDesc.StencilReadMask = static_cast<Diligent::Uint8>(std::strtoul(strValue.c_str(), nullptr, 0));
			else if (strKey == "stencilwritemask") tDesc.StencilWriteMask = static_cast<Diligent::Uint8>(std::strtoul(strValue.c_str(), nullptr, 0));
			else if (strKey == "frontfacestencilfail") tDesc.FrontFace.StencilFailOp = Parse_StencilOp(strValue);
			else if (strKey == "frontfacestencildepthfail") tDesc.FrontFace.StencilDepthFailOp = Parse_StencilOp(strValue);
			else if (strKey == "frontfacestencilpass") tDesc.FrontFace.StencilPassOp = Parse_StencilOp(strValue);
			else if (strKey == "frontfacestencilfunc") tDesc.FrontFace.StencilFunc = Parse_Comparison(strValue);
			else if (strKey == "backfacestencilfail") tDesc.BackFace.StencilFailOp = Parse_StencilOp(strValue);
			else if (strKey == "backfacestencildepthfail") tDesc.BackFace.StencilDepthFailOp = Parse_StencilOp(strValue);
			else if (strKey == "backfacestencilpass") tDesc.BackFace.StencilPassOp = Parse_StencilOp(strValue);
			else if (strKey == "backfacestencilfunc") tDesc.BackFace.StencilFunc = Parse_Comparison(strValue);
		}
		return tDesc;
	}

	Diligent::BlendStateDesc Parse_BlendState(const std::string& strBody)
	{
		Diligent::BlendStateDesc tDesc{};
		bool isIndependent = false;
		std::array<bool, 8> arrEnableSet{};
		Diligent::RenderTargetBlendDesc tCommon{};
		for (const STATE_ASSIGN& tAssign : Parse_StateAssignments(strBody))
		{
			const std::string& strKey = tAssign.strKey;
			if (strKey == "alphatocoverageenable") tDesc.AlphaToCoverageEnable = Parse_Bool(tAssign.strValue);
			else if (strKey == "blendenable")
			{
				const int iIndex = tAssign.iIndex < 0 ? 0 : tAssign.iIndex;
				if (iIndex < 8)
				{
					tDesc.RenderTargets[iIndex].BlendEnable = Parse_Bool(tAssign.strValue);
					arrEnableSet[iIndex] = true;
					if (iIndex > 0) isIndependent = true;
				}
			}
			else if (strKey == "srcblend") tCommon.SrcBlend = Parse_BlendFactor(tAssign.strValue);
			else if (strKey == "destblend") tCommon.DestBlend = Parse_BlendFactor(tAssign.strValue);
			else if (strKey == "blendop") tCommon.BlendOp = Parse_BlendOp(tAssign.strValue);
			else if (strKey == "srcblendalpha") tCommon.SrcBlendAlpha = Parse_BlendFactor(tAssign.strValue);
			else if (strKey == "destblendalpha") tCommon.DestBlendAlpha = Parse_BlendFactor(tAssign.strValue);
			else if (strKey == "blendopalpha") tCommon.BlendOpAlpha = Parse_BlendOp(tAssign.strValue);
			else if (strKey == "rendertargetwritemask")
			{
				const int iIndex = tAssign.iIndex < 0 ? 0 : tAssign.iIndex;
				if (iIndex < 8)
					tDesc.RenderTargets[iIndex].RenderTargetWriteMask = static_cast<Diligent::COLOR_MASK>(std::strtoul(tAssign.strValue.c_str(), nullptr, 0) & 0xF);
			}
		}
		/* Effects11 : SrcBlend 등은 모든 RT 에 공통 적용된다 */
		for (int i = 0; i < 8; ++i)
		{
			Diligent::RenderTargetBlendDesc& tRT = tDesc.RenderTargets[i];
			tRT.SrcBlend = tCommon.SrcBlend;
			tRT.DestBlend = tCommon.DestBlend;
			tRT.BlendOp = tCommon.BlendOp;
			tRT.SrcBlendAlpha = tCommon.SrcBlendAlpha;
			tRT.DestBlendAlpha = tCommon.DestBlendAlpha;
			tRT.BlendOpAlpha = tCommon.BlendOpAlpha;
		}
		tDesc.IndependentBlendEnable = isIndependent;
		return tDesc;
	}

	Diligent::SamplerDesc Default_Sampler()
	{
		/* D3D11 기본 샘플러 : MIN_MAG_MIP_LINEAR , CLAMP */
		Diligent::SamplerDesc tDesc{};
		tDesc.MinFilter = tDesc.MagFilter = tDesc.MipFilter = Diligent::FILTER_TYPE_LINEAR;
		tDesc.AddressU = tDesc.AddressV = tDesc.AddressW = Diligent::TEXTURE_ADDRESS_CLAMP;
		return tDesc;
	}

	Diligent::SamplerDesc Parse_SamplerState(const std::string& strBody)
	{
		Diligent::SamplerDesc tDesc = Default_Sampler();
		for (const STATE_ASSIGN& tAssign : Parse_StateAssignments(strBody))
		{
			const std::string& strKey = tAssign.strKey;
			if (strKey == "filter") Parse_Filter(tAssign.strValue, tDesc);
			else if (strKey == "addressu") tDesc.AddressU = Parse_AddressMode(tAssign.strValue);
			else if (strKey == "addressv") tDesc.AddressV = Parse_AddressMode(tAssign.strValue);
			else if (strKey == "addressw") tDesc.AddressW = Parse_AddressMode(tAssign.strValue);
			else if (strKey == "maxanisotropy") tDesc.MaxAnisotropy = std::atoi(tAssign.strValue.c_str());
			else if (strKey == "mipmaplodbias") tDesc.MipLODBias = static_cast<float>(std::atof(tAssign.strValue.c_str()));
			else if (strKey == "minlod") tDesc.MinLOD = static_cast<float>(std::atof(tAssign.strValue.c_str()));
			else if (strKey == "maxlod") tDesc.MaxLOD = static_cast<float>(std::atof(tAssign.strValue.c_str()));
		}
		return tDesc;
	}

	Diligent::VALUE_TYPE To_ValueType(INPUT_ELEMENT_FORMAT eFormat, Diligent::Uint32& iNumComponents)
	{
		switch (eFormat)
		{
		case FORMAT_R32_FLOAT:				iNumComponents = 1; return Diligent::VT_FLOAT32;
		case FORMAT_R32G32_FLOAT:			iNumComponents = 2; return Diligent::VT_FLOAT32;
		case FORMAT_R32G32B32_FLOAT:		iNumComponents = 3; return Diligent::VT_FLOAT32;
		case FORMAT_R32G32B32A32_FLOAT:		iNumComponents = 4; return Diligent::VT_FLOAT32;
		case FORMAT_R32_UINT:				iNumComponents = 1; return Diligent::VT_UINT32;
		case FORMAT_R32G32_UINT:			iNumComponents = 2; return Diligent::VT_UINT32;
		case FORMAT_R32G32B32_UINT:			iNumComponents = 3; return Diligent::VT_UINT32;
		case FORMAT_R32G32B32A32_UINT:		iNumComponents = 4; return Diligent::VT_UINT32;
		case FORMAT_R32_SINT:				iNumComponents = 1; return Diligent::VT_INT32;
		case FORMAT_R32G32B32A32_SINT:		iNumComponents = 4; return Diligent::VT_INT32;
		case FORMAT_R8G8B8A8_UNORM:			iNumComponents = 4; return Diligent::VT_UINT8;
		default:							iNumComponents = 4; return Diligent::VT_FLOAT32;
		}
	}

	/* "TEXCOORD1" → ("TEXCOORD" , 1) */
	void Split_Semantic(const std::string& strSemantic, std::string& strName, uint32& iIndex)
	{
		size_t iDigit = strSemantic.size();
		while (iDigit > 0 && std::isdigit(static_cast<unsigned char>(strSemantic[iDigit - 1]))) --iDigit;
		strName = ToLower(strSemantic.substr(0, iDigit));
		iIndex = iDigit < strSemantic.size() ? static_cast<uint32>(std::atoi(strSemantic.c_str() + iDigit)) : 0;
	}

	/* 엔진 공용 흰색 텍스처 (바인딩되지 않은 텍스처 슬롯을 채운다 : D3D11 은 null SRV 가 허용되지만 Vulkan 은 안된다) */
	ComPtr<Diligent::ITexture> g_pWhiteTexture;
	ComPtr<Diligent::ITexture> g_pWhiteTextureArray;

	Diligent::ITextureView* Get_DefaultTextureView(Device* pDevice, Diligent::RESOURCE_DIMENSION eDim)
	{
		const bool isArray = eDim == Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		ComPtr<Diligent::ITexture>& pTexture = isArray ? g_pWhiteTextureArray : g_pWhiteTexture;
		if (pTexture == nullptr && pDevice != nullptr)
		{
			const Diligent::Uint32 iWhite = 0xFFFFFFFF;
			Diligent::TextureDesc tDesc{};
			tDesc.Name = "Engine Default White";
			tDesc.Type = isArray ? Diligent::RESOURCE_DIM_TEX_2D_ARRAY : Diligent::RESOURCE_DIM_TEX_2D;
			tDesc.Width = 1;
			tDesc.Height = 1;
			tDesc.ArraySize = 1;
			tDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
			tDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			tDesc.Usage = Diligent::USAGE_IMMUTABLE;
			Diligent::TextureSubResData tSub{ &iWhite, 4 };
			Diligent::TextureData tData{ &tSub, 1 };
			pDevice->CreateTexture(tDesc, &tData, &pTexture);
		}
		return pTexture != nullptr ? pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE) : nullptr;
	}
}

/* Effect 전체 데이터 */
struct CShader::EFFECT
{
	struct VARIABLE
	{
		int32		iCBIndex{ -1 };
		uint32		iOffset{ 0 };
		uint32		iSize{ 0 };
	};

	struct CONSTANT_BUFFER
	{
		std::string				strName;
		uint32					iSize{ 0 };
		CConstantBufferBase*	pOwnBuffer{ nullptr };
		CConstantBufferBase*	pExternal{ nullptr };
	};

	struct BINDING
	{
		Diligent::IShaderResourceVariable*	pVariable{ nullptr };
		Diligent::SHADER_RESOURCE_TYPE		eType{ Diligent::SHADER_RESOURCE_TYPE_UNKNOWN };
		std::string							strName;
		uint32								iArraySize{ 1 };
		int32								iCBIndex{ -1 };
		Diligent::RESOURCE_DIMENSION		eDim{ Diligent::RESOURCE_DIM_TEX_2D };
		std::vector<Diligent::IDeviceObject*> vecBound;
	};

	struct SRB_ENTRY
	{
		ComPtr<Diligent::IShaderResourceBinding>	pSRB;
		std::vector<BINDING>						vecBindings;
	};

	struct PSO_ENTRY
	{
		GRAPHIC_PIPELINE_STATE						tState;
		ComPtr<Diligent::IPipelineState>			pPSO;
		int32										iSRBIndex{ -1 };
	};

	struct PASS
	{
		std::string						strName;
		std::string						strVSEntry, strPSEntry, strGSEntry;
		bool							hasRasterizer{ false }, hasDepthStencil{ false }, hasBlend{ false };
		RASTERIZER_DESC					tRasterizer{};
		DEPTH_STENCIL_DESC				tDepthStencil{};
		uint32							iStencilRef{ 0 };
		BLEND_DESC						tBlend{};
		float							fBlendFactor[4]{ 0.f,0.f,0.f,0.f };

		ComPtr<Diligent::IShader>		pVS, pPS, pGS;
		std::vector<PSO_ENTRY>			vecPSO;
		std::vector<SRB_ENTRY>			vecSRB;
	};

	ComPtr<Device>										pDevice;
	ComPtr<DeviceContext>								pContext;
	std::wstring										wstrFileName;

	std::string											strSource;
	std::vector<PASS>									vecPass;
	std::vector<Diligent::LayoutElement>				vecLayout;
	std::vector<std::pair<std::string, uint32>>			vecLayoutSemantic;

	std::unordered_map<std::string, Diligent::SamplerDesc>	umapSampler;

	std::vector<CONSTANT_BUFFER>						vecCB;
	std::unordered_map<std::string, VARIABLE>			umapVariable;
	std::unordered_map<std::string, uint32>				umapTextureArraySize;
	std::unordered_map<std::string, Diligent::RESOURCE_DIMENSION> umapTextureDim;
	std::unordered_map<std::string, std::vector<ComPtr<Diligent::ITextureView>>> umapTexture;

	std::unordered_map<std::string, std::vector<double>>	umapDefaultValue;
	std::unordered_map<std::string, bool>					umapDefaultIsInt;

	std::unordered_map<std::string, ComPtr<Diligent::IShader>> umapCompiled;

	~EFFECT()
	{
		for (CONSTANT_BUFFER& tCB : vecCB)
		{
			Safe_Release(tCB.pOwnBuffer);
			Safe_Release(tCB.pExternal);
		}
	}

	int32 Find_CB(const std::string& strName) const
	{
		for (size_t i = 0; i < vecCB.size(); ++i)
			if (vecCB[i].strName == strName)
				return static_cast<int32>(i);
		return -1;
	}

	/* -------------------------------------------------------------------------- */
	/* 1. 파일 전처리 : include 펼치기 , 상태블록 / technique 파싱 후 제거          */
	/* -------------------------------------------------------------------------- */
	HRESULT Parse(const std::wstring& wstrPath)
	{
		std::string strText;
		std::set<std::string> setIncluded;
		if (!Expand_Includes(std::filesystem::path(wstrPath), strText, setIncluded))
			return E_FAIL;

		std::unordered_map<std::string, RASTERIZER_DESC>		umapRasterizer;
		std::unordered_map<std::string, DEPTH_STENCIL_DESC>		umapDepthStencil;
		std::unordered_map<std::string, BLEND_DESC>				umapBlend;

		/* ---- 상태 블록 ---- */
		static const std::regex StateRegex(R"(\b(RasterizerState|BlendState|DepthStencilState|SamplerState|sampler|SamplerComparisonState)\s+(\w+)\s*(=\s*sampler_state\s*)?\{)");
		for (;;)
		{
			std::smatch Match;
			if (!std::regex_search(strText, Match, StateRegex))
				break;

			const size_t iStart = static_cast<size_t>(Match.position(0));
			const size_t iOpen = iStart + Match.length(0) - 1;
			const size_t iClose = Find_MatchingBrace(strText, iOpen);
			if (iClose == std::string::npos)
				return E_FAIL;
			size_t iEnd = iClose + 1;
			while (iEnd < strText.size() && std::isspace(static_cast<unsigned char>(strText[iEnd]))) ++iEnd;
			if (iEnd < strText.size() && strText[iEnd] == ';') ++iEnd;
			else iEnd = iClose + 1;

			const std::string strType = Match[1].str();
			const std::string strName = Match[2].str();
			const std::string strBody = strText.substr(iOpen + 1, iClose - iOpen - 1);

			std::string strReplace;
			if (strType == "RasterizerState")			umapRasterizer[strName] = Parse_RasterizerState(strBody);
			else if (strType == "DepthStencilState")	umapDepthStencil[strName] = Parse_DepthStencilState(strBody);
			else if (strType == "BlendState")			umapBlend[strName] = Parse_BlendState(strBody);
			else
			{
				umapSampler[strName] = Parse_SamplerState(strBody);
				strReplace = (strType == "SamplerComparisonState" ? "SamplerComparisonState " : "SamplerState ") + strName + ";";
			}

			/* 줄 수 유지 */
			const std::string strRemoved = strText.substr(iStart, iEnd - iStart);
			const size_t iNewLines = static_cast<size_t>(std::count(strRemoved.begin(), strRemoved.end(), '\n'));
			strText.replace(iStart, iEnd - iStart, strReplace + std::string(iNewLines, '\n'));
		}

		/* ---- technique / pass ---- */
		static const std::regex TechniqueRegex(R"(\btechnique(?:10|11)?\s+(\w+)\s*\{)");
		static const std::regex PassRegex(R"(\bpass\s+(\w+)\s*\{)");
		static const std::regex ShaderAssignRegex(R"((VertexShader|PixelShader|GeometryShader)\s*=\s*(?:compile\s+\w+\s+(\w+)\s*\(|(NULL|null)))");
		static const std::regex SetShaderRegex(R"(Set(Vertex|Pixel|Geometry)Shader\s*\(\s*(?:CompileShader\s*\(\s*\w+\s*,\s*(\w+)\s*\(|(NULL|null)))");
		static const std::regex SetStateRegex(R"(Set(RasterizerState|DepthStencilState|BlendState)\s*\(\s*(\w+)\s*(?:,([^;]*))?\))");

		for (;;)
		{
			std::smatch Match;
			if (!std::regex_search(strText, Match, TechniqueRegex))
				break;

			const size_t iStart = static_cast<size_t>(Match.position(0));
			const size_t iOpen = iStart + Match.length(0) - 1;
			const size_t iClose = Find_MatchingBrace(strText, iOpen);
			if (iClose == std::string::npos)
				return E_FAIL;

			std::string strTechnique = strText.substr(iOpen + 1, iClose - iOpen - 1);

			for (;;)
			{
				std::smatch PassMatch;
				if (!std::regex_search(strTechnique, PassMatch, PassRegex))
					break;
				const size_t iPassStart = static_cast<size_t>(PassMatch.position(0));
				const size_t iPassOpen = iPassStart + PassMatch.length(0) - 1;
				const size_t iPassClose = Find_MatchingBrace(strTechnique, iPassOpen);
				if (iPassClose == std::string::npos)
					return E_FAIL;

				const std::string strBody = strTechnique.substr(iPassOpen + 1, iPassClose - iPassOpen - 1);

				PASS tPass{};
				tPass.strName = PassMatch[1].str();

				auto Apply_Shader = [&tPass](const std::string& strStage, const std::string& strEntry) {
					if (strStage == "VertexShader" || strStage == "Vertex") tPass.strVSEntry = strEntry;
					else if (strStage == "PixelShader" || strStage == "Pixel") tPass.strPSEntry = strEntry;
					else if (strStage == "GeometryShader" || strStage == "Geometry") tPass.strGSEntry = strEntry;
				};
				for (std::sregex_iterator Iter(strBody.begin(), strBody.end(), ShaderAssignRegex), End; Iter != End; ++Iter)
					Apply_Shader((*Iter)[1].str(), (*Iter)[2].matched ? (*Iter)[2].str() : std::string{});
				for (std::sregex_iterator Iter(strBody.begin(), strBody.end(), SetShaderRegex), End; Iter != End; ++Iter)
					Apply_Shader((*Iter)[1].str(), (*Iter)[2].matched ? (*Iter)[2].str() : std::string{});

				for (std::sregex_iterator Iter(strBody.begin(), strBody.end(), SetStateRegex), End; Iter != End; ++Iter)
				{
					const std::string strState = (*Iter)[1].str();
					const std::string strName = (*Iter)[2].str();
					const std::string strArgs = (*Iter)[3].matched ? (*Iter)[3].str() : std::string{};
					if (strState == "RasterizerState")
					{
						auto Find = umapRasterizer.find(strName);
						if (Find != umapRasterizer.end()) { tPass.hasRasterizer = true; tPass.tRasterizer = Find->second; }
					}
					else if (strState == "DepthStencilState")
					{
						auto Find = umapDepthStencil.find(strName);
						if (Find != umapDepthStencil.end()) { tPass.hasDepthStencil = true; tPass.tDepthStencil = Find->second; }
						const std::vector<double> vecArgs = Parse_Numbers(strArgs);
						tPass.iStencilRef = vecArgs.empty() ? 0 : static_cast<uint32>(vecArgs[0]);
					}
					else
					{
						auto Find = umapBlend.find(strName);
						if (Find != umapBlend.end()) { tPass.hasBlend = true; tPass.tBlend = Find->second; }
						const std::vector<double> vecArgs = Parse_Numbers(strArgs);
						for (int i = 0; i < 4 && i < static_cast<int>(vecArgs.size()); ++i)
							tPass.fBlendFactor[i] = static_cast<float>(vecArgs[i]);
					}
				}

				vecPass.push_back(std::move(tPass));
				strTechnique.erase(0, iPassClose + 1);
			}

			const std::string strRemoved = strText.substr(iStart, iClose + 1 - iStart);
			const size_t iNewLines = static_cast<size_t>(std::count(strRemoved.begin(), strRemoved.end(), '\n'));
			strText.replace(iStart, iClose + 1 - iStart, std::string(iNewLines, '\n'));
		}

		if (vecPass.empty())
			return E_FAIL;

		strSource = Process_Globals(strText);
		return S_OK;
	}

	/* -------------------------------------------------------------------------- */
	/* 2. 전역 변수 → cbuffer _Globals_ , cbuffer 멤버 초기값 기록 , register 제거  */
	/* -------------------------------------------------------------------------- */
	void Record_Default(const std::string& strDeclaration)
	{
		/* "row_major float4x4 g_matWorld = { ... }" */
		const size_t iEqual = strDeclaration.find('=');
		if (iEqual == std::string::npos)
			return;
		std::string strLeft = Trim(strDeclaration.substr(0, iEqual));
		const std::string strInit = strDeclaration.substr(iEqual + 1);

		/* 배열 , 시맨틱 제거 */
		const size_t iBracket = strLeft.find('[');
		if (iBracket != std::string::npos) strLeft = Trim(strLeft.substr(0, iBracket));
		const size_t iColon = strLeft.find(':');
		if (iColon != std::string::npos) strLeft = Trim(strLeft.substr(0, iColon));

		size_t iNameStart = strLeft.size();
		while (iNameStart > 0 && Is_IdentChar(strLeft[iNameStart - 1])) --iNameStart;
		const std::string strName = strLeft.substr(iNameStart);
		std::string strTypePart = Trim(strLeft.substr(0, iNameStart));
		/* 마지막 단어가 타입 */
		size_t iTypeStart = strTypePart.size();
		while (iTypeStart > 0 && (Is_IdentChar(strTypePart[iTypeStart - 1]) || strTypePart[iTypeStart - 1] == '>' || strTypePart[iTypeStart - 1] == '<' || strTypePart[iTypeStart - 1] == ',')) --iTypeStart;
		const std::string strType = strTypePart.substr(iTypeStart);

		if (strName.empty())
			return;
		umapDefaultValue[strName] = Parse_Numbers(strInit);
		umapDefaultIsInt[strName] = Is_IntegerType(strType);
	}

	std::string Strip_Initializer(const std::string& strDeclaration)
	{
		const size_t iEqual = strDeclaration.find('=');
		if (iEqual == std::string::npos)
			return strDeclaration;
		Record_Default(strDeclaration);
		return Trim(strDeclaration.substr(0, iEqual));
	}

	std::string Process_CBufferBody(const std::string& strBody)
	{
		/* 멤버 선언 단위로 초기값을 제거한다 */
		std::string strOut;
		size_t iPos = 0;
		int iDepth = 0;
		size_t iStatementStart = 0;
		for (iPos = 0; iPos < strBody.size(); ++iPos)
		{
			const char c = strBody[iPos];
			if (c == '{') ++iDepth;
			else if (c == '}') --iDepth;
			else if (c == ';' && iDepth == 0)
			{
				const std::string strStatement = strBody.substr(iStatementStart, iPos - iStatementStart);
				const size_t iNewLines = static_cast<size_t>(std::count(strStatement.begin(), strStatement.end(), '\n'));
				strOut += Remove_Register(Strip_Initializer(strStatement)) + ";" + std::string(iNewLines, '\n');
				iStatementStart = iPos + 1;
			}
		}
		strOut += strBody.substr(iStatementStart);
		return strOut;
	}

	std::string Process_Globals(const std::string& strText)
	{
		std::string strOut;
		std::string strGlobals;
		uint32 iGlobalsIndex = 0;

		/* 모아둔 전역 변수를 cbuffer 로 내보낸다.                                                  */
		/* 함수 정의 직전에 내보내므로 앞에서 정의된 #define / struct 를 그대로 사용할 수 있고         */
		/* 그 함수에서 전역 변수를 사용할 수 있다. (Effects11 의 $Globals 와 같은 역할)                */
		auto Flush_Globals = [&]()
		{
			if (strGlobals.empty())
				return;
			strOut += "\ncbuffer " + std::string(GLOBALS_CB_NAME) + std::to_string(iGlobalsIndex++) + "\n{\n" + strGlobals + "};\n";
			strGlobals.clear();
		};

		size_t i = 0;
		while (i < strText.size())
		{
			/* 공백 */
			if (std::isspace(static_cast<unsigned char>(strText[i])))
			{
				strOut.push_back(strText[i]);
				++i;
				continue;
			}
			/* 전처리기 줄 */
			if (strText[i] == '#')
			{
				const size_t iLineEnd = strText.find('\n', i);
				const size_t iEnd = iLineEnd == std::string::npos ? strText.size() : iLineEnd;
				strOut += strText.substr(i, iEnd - i);
				i = iEnd;
				continue;
			}

			/* 문장 하나 : ';' 또는 '{...}' 블록으로 끝난다 */
			size_t j = i;
			int iParen = 0;
			bool isBlock = false;
			size_t iBlockOpen = std::string::npos;
			for (; j < strText.size(); ++j)
			{
				const char c = strText[j];
				if (c == '(') ++iParen;
				else if (c == ')') --iParen;
				else if (c == '{' && iParen == 0)
				{
					/* "x = { ... };" 형태의 초기화는 블록이 아니다 */
					size_t k = j;
					while (k > i && std::isspace(static_cast<unsigned char>(strText[k - 1]))) --k;
					if (k > i && strText[k - 1] == '=')
					{
						const size_t iClose = Find_MatchingBrace(strText, j);
						if (iClose == std::string::npos) { j = strText.size(); break; }
						j = iClose;
						continue;
					}
					isBlock = true;
					iBlockOpen = j;
					break;
				}
				else if (c == ';' && iParen == 0)
					break;
			}

			if (isBlock)
			{
				const size_t iClose = Find_MatchingBrace(strText, iBlockOpen);
				if (iClose == std::string::npos)
				{
					strOut += strText.substr(i);
					break;
				}
				size_t iEnd = iClose + 1;
				size_t k = iEnd;
				while (k < strText.size() && std::isspace(static_cast<unsigned char>(strText[k])) && strText[k] != '\n') ++k;
				if (k < strText.size() && strText[k] == ';') iEnd = k + 1;

				std::string strHeader = strText.substr(i, iBlockOpen - i);
				const std::string strHeaderTrim = Trim(strHeader);
				const std::string strBody = strText.substr(iBlockOpen + 1, iClose - iBlockOpen - 1);

				if (strHeaderTrim.rfind("cbuffer", 0) == 0 || strHeaderTrim.rfind("tbuffer", 0) == 0)
					strOut += Remove_Register(strHeader) + "{" + Process_CBufferBody(strBody) + "}" + strText.substr(iClose + 1, iEnd - iClose - 1);
				else if (strHeaderTrim.rfind("struct", 0) == 0)
					strOut += strText.substr(i, iEnd - i);
				else
				{
					/* 함수 정의 : 앞에 모아둔 전역 변수를 먼저 선언한다 */
					Flush_Globals();
					strOut += strText.substr(i, iEnd - i);
				}
				i = iEnd;
				continue;
			}

			const size_t iEnd = j < strText.size() ? j + 1 : strText.size();
			const std::string strStatement = strText.substr(i, iEnd - i);
			i = iEnd;

			/* 선언문 분석 */
			std::string strDecl = Trim(strStatement);
			if (!strDecl.empty() && strDecl.back() == ';') strDecl.pop_back();

			std::stringstream ssWords(strDecl);
			std::vector<std::string> vecWords;
			std::string strWord;
			while (ssWords >> strWord) vecWords.push_back(strWord);

			bool isStatic = false;
			size_t iTypeWord = 0;
			for (; iTypeWord < vecWords.size(); ++iTypeWord)
			{
				const std::string& w = vecWords[iTypeWord];
				if (w == "static" || w == "groupshared") isStatic = true;
				else if (w == "const" || w == "uniform" || w == "extern" || w == "row_major" || w == "column_major" || w == "shared" || w == "precise" || w == "volatile")
					continue;
				else break;
			}

			const bool isTypedef = !vecWords.empty() && vecWords[0] == "typedef";
			std::string strType = iTypeWord < vecWords.size() ? vecWords[iTypeWord] : std::string{};

			if (isStatic || isTypedef || strType.empty() || strDecl.find('(') != std::string::npos && strDecl.find('=') == std::string::npos && !Is_ResourceType(strType))
			{
				/* static const , typedef , 함수 선언 등은 그대로 */
				strOut += strStatement;
				continue;
			}

			if (Is_ResourceType(strType))
			{
				strOut += Remove_Register(strStatement);
				continue;
			}

			/* 전역 상수 → 다음 함수 정의 앞에서 _Globals_N cbuffer 로 선언 (초기값은 따로 기록) */
			strGlobals += "    " + Remove_Register(Strip_Initializer(strDecl)) + ";\n";
			const size_t iNewLines = static_cast<size_t>(std::count(strStatement.begin(), strStatement.end(), '\n'));
			strOut += std::string(iNewLines, '\n');
		}

		Flush_Globals();
		return strOut;
	}

	/* -------------------------------------------------------------------------- */
	/* 3. 정점 셰이더 입력 시맨틱 → ATTRIBn (Diligent Vulkan / GL 요구사항)          */
	/* -------------------------------------------------------------------------- */
	std::string Remap_Semantics(const std::string& strBody)
	{
		static const std::regex SemanticRegex(R"(:\s*([A-Za-z_]+[0-9]*)\s*(?=[;,)]))");
		std::string strOut;
		std::sregex_iterator Iter(strBody.begin(), strBody.end(), SemanticRegex), End;
		size_t iLast = 0;
		for (; Iter != End; ++Iter)
		{
			const std::smatch& Match = *Iter;
			std::string strName;
			uint32 iIndex = 0;
			Split_Semantic(Match[1].str(), strName, iIndex);

			int32 iAttrib = -1;
			for (size_t i = 0; i < vecLayoutSemantic.size(); ++i)
			{
				if (vecLayoutSemantic[i].first == strName && vecLayoutSemantic[i].second == iIndex)
				{
					iAttrib = static_cast<int32>(i);
					break;
				}
			}
			strOut += strBody.substr(iLast, static_cast<size_t>(Match.position(0)) - iLast);
			if (iAttrib >= 0)
				strOut += ": ATTRIB" + std::to_string(iAttrib) + " ";
			else
				strOut += Match.str(0);
			iLast = static_cast<size_t>(Match.position(0) + Match.length(0));
		}
		strOut += strBody.substr(iLast);
		return strOut;
	}

	void Remap_VertexInputs()
	{
		std::set<std::string> setRemappedStruct;
		std::set<std::string> setEntries;
		for (const PASS& tPass : vecPass)
			if (!tPass.strVSEntry.empty())
				setEntries.insert(tPass.strVSEntry);

		for (const std::string& strEntry : setEntries)
		{
			const std::regex FunctionRegex("\\b" + strEntry + "\\s*\\(");
			std::smatch Match;
			if (!std::regex_search(strSource, Match, FunctionRegex))
				continue;
			const size_t iOpen = static_cast<size_t>(Match.position(0) + Match.length(0) - 1);
			const size_t iClose = Find_MatchingBrace(strSource, iOpen, '(', ')');
			if (iClose == std::string::npos)
				continue;

			const std::string strParams = strSource.substr(iOpen + 1, iClose - iOpen - 1);
			/* 시맨틱이 직접 붙은 파라미터 */
			strSource.replace(iOpen + 1, iClose - iOpen - 1, Remap_Semantics(strParams));

			/* 구조체 파라미터 : "VS_IN In" */
			std::stringstream ssParams(strParams);
			std::string strParam;
			while (std::getline(ssParams, strParam, ','))
			{
				std::stringstream ssWords(strParam);
				std::vector<std::string> vecWords;
				std::string strWord;
				while (ssWords >> strWord)
					if (strWord != "in" && strWord != "const" && strWord != "uniform")
						vecWords.push_back(strWord);
				if (vecWords.empty())
					continue;
				const std::string strTypeName = vecWords[0];
				if (setRemappedStruct.count(strTypeName) != 0)
					continue;

				const std::regex StructRegex("\\bstruct\\s+" + strTypeName + "\\s*\\{");
				std::smatch StructMatch;
				if (!std::regex_search(strSource, StructMatch, StructRegex))
					continue;
				const size_t iStructOpen = static_cast<size_t>(StructMatch.position(0) + StructMatch.length(0) - 1);
				const size_t iStructClose = Find_MatchingBrace(strSource, iStructOpen);
				if (iStructClose == std::string::npos)
					continue;
				const std::string strStructBody = strSource.substr(iStructOpen + 1, iStructClose - iStructOpen - 1);
				strSource.replace(iStructOpen + 1, iStructClose - iStructOpen - 1, Remap_Semantics(strStructBody));
				setRemappedStruct.insert(strTypeName);
			}
		}
	}

	/* -------------------------------------------------------------------------- */
	/* 4. 컴파일 + 리플렉션                                                        */
	/* -------------------------------------------------------------------------- */
	ComPtr<Diligent::IShader> Compile(const std::string& strEntry, Diligent::SHADER_TYPE eType)
	{
		if (strEntry.empty())
			return nullptr;

		const std::string strKey = strEntry + "#" + std::to_string(static_cast<int>(eType));
		auto Find = umapCompiled.find(strKey);
		if (Find != umapCompiled.end())
			return Find->second;

		const std::string strName = WString_To_UTF8(wstrFileName) + " : " + strEntry;

		Diligent::ShaderCreateInfo tCI{};
		tCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		tCI.Source = strSource.c_str();
		tCI.SourceLength = strSource.size();
		tCI.EntryPoint = strEntry.c_str();
		tCI.Desc.Name = strName.c_str();
		tCI.Desc.ShaderType = eType;
		tCI.Desc.UseCombinedTextureSamplers = false;
		tCI.LoadConstantBufferReflection = true;
		tCI.HLSLVersion = Diligent::ShaderVersion{ 5, 0 };

		ComPtr<Diligent::IShader> pShader;
		Diligent::IDataBlob* pOutput = nullptr;
		pDevice->CreateShader(tCI, &pShader, &pOutput);
		if (pShader == nullptr)
		{
			std::string strError = "[Shader Compile Error] " + strName + "\n";
			if (pOutput != nullptr && pOutput->GetSize() > 0)
				strError += std::string(static_cast<const char*>(pOutput->GetConstDataPtr()), pOutput->GetSize());
			Engine::Engine_Log("%s", strError.c_str());
		}
		if (pOutput != nullptr)
			pOutput->Release();

		umapCompiled[strKey] = pShader;
		return pShader;
	}

	void Reflect(Diligent::IShader* pShader)
	{
		if (pShader == nullptr)
			return;

		const Diligent::Uint32 iCount = pShader->GetResourceCount();
		for (Diligent::Uint32 i = 0; i < iCount; ++i)
		{
			Diligent::ShaderResourceDesc tRes{};
			pShader->GetResourceDesc(i, tRes);
			if (tRes.Name == nullptr)
				continue;

			if (tRes.Type == Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER)
			{
				const Diligent::ShaderCodeBufferDesc* pCBDesc = pShader->GetConstantBufferDesc(i);
				std::string strCBName = tRes.Name;
				/* glslang 은 $Global , FXC 는 $Globals 로 부를 수 있다 */
				if (strCBName == "$Global" || strCBName == "$Globals")
					strCBName = GLOBALS_CB_NAME;

				int32 iCB = Find_CB(strCBName);
				if (iCB < 0)
				{
					CONSTANT_BUFFER tCB{};
					tCB.strName = strCBName;
					tCB.iSize = pCBDesc != nullptr ? pCBDesc->Size : 16;
					vecCB.push_back(tCB);
					iCB = static_cast<int32>(vecCB.size() - 1);
				}
				else if (pCBDesc != nullptr && pCBDesc->Size > vecCB[iCB].iSize)
					vecCB[iCB].iSize = pCBDesc->Size;

				if (pCBDesc != nullptr)
				{
					std::vector<std::pair<uint32, std::string>> vecOffsets;
					for (Diligent::Uint32 v = 0; v < pCBDesc->NumVariables; ++v)
					{
						const Diligent::ShaderCodeVariableDesc& tVar = pCBDesc->pVariables[v];
						if (tVar.Name == nullptr) continue;
						vecOffsets.push_back({ tVar.Offset, tVar.Name });
					}
					std::sort(vecOffsets.begin(), vecOffsets.end());
					const uint32 iCBSize = vecCB[iCB].iSize;
					for (size_t v = 0; v < vecOffsets.size(); ++v)
					{
						VARIABLE tVar{};
						tVar.iCBIndex = iCB;
						tVar.iOffset = vecOffsets[v].first;
						const uint32 iNext = v + 1 < vecOffsets.size() ? vecOffsets[v + 1].first : iCBSize;
						tVar.iSize = iNext > tVar.iOffset ? iNext - tVar.iOffset : 0;
						umapVariable[vecOffsets[v].second] = tVar;
					}
				}
			}
			else if (tRes.Type == Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV)
			{
				uint32& iArraySize = umapTextureArraySize[tRes.Name];
				iArraySize = (std::max)(iArraySize, static_cast<uint32>(tRes.ArraySize));
			}
		}
	}

	HRESULT Build(const INPUT_ELEMENT_DESC* pElements, uint32 iNumElements)
	{
		for (uint32 i = 0; i < iNumElements; ++i)
		{
			const INPUT_ELEMENT_DESC& tElement = pElements[i];
			Diligent::LayoutElement tLayout{};
			tLayout.InputIndex = i;
			tLayout.BufferSlot = tElement.InputSlot;
			tLayout.ValueType = To_ValueType(tElement.Format, tLayout.NumComponents);
			tLayout.IsNormalized = tElement.Format == FORMAT_R8G8B8A8_UNORM;
			tLayout.RelativeOffset = tElement.AlignedByteOffset == APPEND_ALIGNED_ELEMENT ? Diligent::LAYOUT_ELEMENT_AUTO_OFFSET : tElement.AlignedByteOffset;
			tLayout.Frequency = tElement.InputSlotClass == INPUT_PER_INSTANCE_DATA ? Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE : Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;
			tLayout.InstanceDataStepRate = tElement.InputSlotClass == INPUT_PER_INSTANCE_DATA ? (std::max)(1u, tElement.InstanceDataStepRate) : 0;
			vecLayout.push_back(tLayout);
			vecLayoutSemantic.push_back({ ToLower(tElement.SemanticName), tElement.SemanticIndex });
		}

		Remap_VertexInputs();

		for (PASS& tPass : vecPass)
		{
			tPass.pVS = Compile(tPass.strVSEntry, Diligent::SHADER_TYPE_VERTEX);
			tPass.pPS = Compile(tPass.strPSEntry, Diligent::SHADER_TYPE_PIXEL);
			tPass.pGS = Compile(tPass.strGSEntry, Diligent::SHADER_TYPE_GEOMETRY);
			if (tPass.pVS == nullptr || (!tPass.strPSEntry.empty() && tPass.pPS == nullptr))
			{
				Engine::Engine_Log("[Shader] %s : pass '%s' shader create failed (VS:%s PS:%s)", WString_To_UTF8(wstrFileName).c_str(), tPass.strName.c_str(), tPass.strVSEntry.c_str(), tPass.strPSEntry.c_str());
				return E_FAIL;
			}

			Reflect(tPass.pVS.Get());
			Reflect(tPass.pPS.Get());
			Reflect(tPass.pGS.Get());
		}

		/* 셰이더 소유 상수버퍼 생성 + 초기값 */
		for (CONSTANT_BUFFER& tCB : vecCB)
		{
			tCB.pOwnBuffer = CConstantBufferBase::Create((std::max)(16u, tCB.iSize), tCB.strName.c_str(), pDevice, pContext);
			if (tCB.pOwnBuffer == nullptr)
			{
				Engine::Engine_Log("[Shader] %s : ConstantBuffer '%s' (%u Byte) create failed", WString_To_UTF8(wstrFileName).c_str(), tCB.strName.c_str(), tCB.iSize);
				return E_FAIL;
			}
		}
		for (auto& Pair : umapDefaultValue)
		{
			auto Find = umapVariable.find(Pair.first);
			if (Find == umapVariable.end())
				continue;
			const VARIABLE& tVar = Find->second;
			const bool isInt = umapDefaultIsInt[Pair.first];
			uint32 iOffset = tVar.iOffset;
			for (double dValue : Pair.second)
			{
				if (iOffset + 4 > tVar.iOffset + tVar.iSize)
					break;
				if (isInt)
				{
					const int32 iValue = static_cast<int32>(dValue);
					vecCB[tVar.iCBIndex].pOwnBuffer->Write(&iValue, 4, iOffset);
				}
				else
				{
					const float fValue = static_cast<float>(dValue);
					vecCB[tVar.iCBIndex].pOwnBuffer->Write(&fValue, 4, iOffset);
				}
				iOffset += 4;
			}
		}
		return S_OK;
	}

	/* -------------------------------------------------------------------------- */
	/* 5. Draw 시점 PSO                                                            */
	/* -------------------------------------------------------------------------- */
	static bool Is_SameState(const GRAPHIC_PIPELINE_STATE& A, const GRAPHIC_PIPELINE_STATE& B)
	{
		if (!(A.tRasterizer == B.tRasterizer) || !(A.tDepthStencil == B.tDepthStencil) || !(A.tBlend == B.tBlend))
			return false;
		if (A.iNumRenderTargets != B.iNumRenderTargets || A.eDSVFormat != B.eDSVFormat || A.eTopology != B.eTopology)
			return false;
		for (uint32 i = 0; i < A.iNumRenderTargets; ++i)
			if (A.eRTVFormats[i] != B.eRTVFormats[i])
				return false;
		return A.iVertexStrides[0] == B.iVertexStrides[0] && A.iVertexStrides[1] == B.iVertexStrides[1];
	}

	HRESULT Create_PSO(PASS& tPass, const GRAPHIC_PIPELINE_STATE& tState, PSO_ENTRY& tEntry)
	{
		Diligent::GraphicsPipelineStateCreateInfo tCI{};
		const std::string strPSOName = WString_To_UTF8(wstrFileName) + " : " + tPass.strName;
		tCI.PSODesc.Name = strPSOName.c_str();
		tCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

		Diligent::GraphicsPipelineDesc& tGP = tCI.GraphicsPipeline;
		tGP.RasterizerDesc = tState.tRasterizer;
		tGP.DepthStencilDesc = tState.tDepthStencil;
		tGP.BlendDesc = tState.tBlend;
		tGP.NumRenderTargets = static_cast<Diligent::Uint8>(tState.iNumRenderTargets);
		for (uint32 i = 0; i < tState.iNumRenderTargets; ++i)
			tGP.RTVFormats[i] = tState.eRTVFormats[i];
		tGP.DSVFormat = tState.eDSVFormat;
		tGP.PrimitiveTopology = tState.eTopology;
		/* 깊이버퍼가 없으면 깊이테스트를 끈다 (D3D11 은 자동 무시) */
		if (tState.eDSVFormat == Diligent::TEX_FORMAT_UNKNOWN)
		{
			tGP.DepthStencilDesc.DepthEnable = false;
			tGP.DepthStencilDesc.DepthWriteEnable = false;
			tGP.DepthStencilDesc.StencilEnable = false;
		}

		std::vector<Diligent::LayoutElement> vecElements = vecLayout;
		for (Diligent::LayoutElement& tElement : vecElements)
		{
			const uint32 iSlot = tElement.BufferSlot < 2 ? tElement.BufferSlot : 0;
			tElement.Stride = tState.iVertexStrides[iSlot] != 0 ? tState.iVertexStrides[iSlot] : Diligent::LAYOUT_ELEMENT_AUTO_STRIDE;
		}
		tGP.InputLayout.LayoutElements = vecElements.data();
		tGP.InputLayout.NumElements = static_cast<Diligent::Uint32>(vecElements.size());

		tCI.pVS = tPass.pVS.Get();
		tCI.pPS = tPass.pPS.Get();
		tCI.pGS = tPass.pGS.Get();

		/* 모든 변수는 Draw 마다 바뀔 수 있으므로 DYNAMIC (D3D11 / Effects11 과 같은 사용성) */
		tCI.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
		tCI.PSODesc.ResourceLayout.DefaultVariableMergeStages = Diligent::SHADER_TYPE_ALL_GRAPHICS;

		/* Effect 파일에 정의된 sampler_state → Immutable Sampler */
		std::vector<Diligent::ImmutableSamplerDesc> vecSamplers;
		std::set<std::string> setSamplerNames;
		for (Diligent::IShader* pShader : { tPass.pVS.Get(), tPass.pPS.Get(), tPass.pGS.Get() })
		{
			if (pShader == nullptr) continue;
			for (Diligent::Uint32 i = 0; i < pShader->GetResourceCount(); ++i)
			{
				Diligent::ShaderResourceDesc tRes{};
				pShader->GetResourceDesc(i, tRes);
				if (tRes.Type == Diligent::SHADER_RESOURCE_TYPE_SAMPLER && tRes.Name != nullptr)
					setSamplerNames.insert(tRes.Name);
			}
		}
		for (const std::string& strSampler : setSamplerNames)
		{
			auto Find = umapSampler.find(strSampler);
			const Diligent::SamplerDesc tDesc = Find != umapSampler.end() ? Find->second : Default_Sampler();
			vecSamplers.push_back(Diligent::ImmutableSamplerDesc{ Diligent::SHADER_TYPE_ALL_GRAPHICS, nullptr, tDesc });
			vecSamplers.back().SamplerOrTextureName = setSamplerNames.find(strSampler)->c_str();
		}
		tCI.PSODesc.ResourceLayout.ImmutableSamplers = vecSamplers.data();
		tCI.PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<Diligent::Uint32>(vecSamplers.size());

		ComPtr<Diligent::IPipelineState> pPSO;
		pDevice->CreateGraphicsPipelineState(tCI, &pPSO);
		if (pPSO == nullptr)
			return E_FAIL;

		tEntry.tState = tState;
		tEntry.pPSO = pPSO;

		/* SRB : 호환되는 기존 SRB 가 있으면 재사용 */
		for (size_t i = 0; i < tPass.vecPSO.size(); ++i)
		{
			if (tPass.vecPSO[i].pPSO != nullptr && tPass.vecPSO[i].iSRBIndex >= 0 && pPSO->IsCompatibleWith(tPass.vecPSO[i].pPSO.Get()))
			{
				tEntry.iSRBIndex = tPass.vecPSO[i].iSRBIndex;
				return S_OK;
			}
		}

		SRB_ENTRY tSRB{};
		pPSO->CreateShaderResourceBinding(&tSRB.pSRB, true);
		if (tSRB.pSRB == nullptr)
			return E_FAIL;

		for (Diligent::SHADER_TYPE eStage : { Diligent::SHADER_TYPE_VERTEX, Diligent::SHADER_TYPE_PIXEL, Diligent::SHADER_TYPE_GEOMETRY })
		{
			const Diligent::Uint32 iVarCount = tSRB.pSRB->GetVariableCount(eStage);
			for (Diligent::Uint32 i = 0; i < iVarCount; ++i)
			{
				Diligent::IShaderResourceVariable* pVar = tSRB.pSRB->GetVariableByIndex(eStage, i);
				if (pVar == nullptr) continue;
				Diligent::ShaderResourceDesc tRes{};
				pVar->GetResourceDesc(tRes);

				BINDING tBinding{};
				tBinding.pVariable = pVar;
				tBinding.eType = tRes.Type;
				tBinding.strName = tRes.Name != nullptr ? tRes.Name : "";
				if (tBinding.strName == "$Global" || tBinding.strName == "$Globals")
					tBinding.strName = GLOBALS_CB_NAME;
				tBinding.iArraySize = (std::max)(1u, static_cast<uint32>(tRes.ArraySize));
				tBinding.iCBIndex = tRes.Type == Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER ? Find_CB(tBinding.strName) : -1;
				tBinding.vecBound.assign(tBinding.iArraySize, nullptr);
				tSRB.vecBindings.push_back(tBinding);
			}
		}

		tPass.vecSRB.push_back(std::move(tSRB));
		tEntry.iSRBIndex = static_cast<int32>(tPass.vecSRB.size() - 1);
		return S_OK;
	}

	HRESULT Bind_Resources(SRB_ENTRY& tSRB)
	{
		for (BINDING& tBinding : tSRB.vecBindings)
		{
			if (tBinding.eType == Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER)
			{
				if (tBinding.iCBIndex < 0)
					continue;
				CONSTANT_BUFFER& tCB = vecCB[tBinding.iCBIndex];
				CConstantBufferBase* pCB = tCB.pExternal != nullptr ? tCB.pExternal : tCB.pOwnBuffer;
				if (pCB == nullptr)
					continue;
				pCB->Ensure_Uploaded();
				Diligent::IDeviceObject* pObject = pCB->Get_Buffer();
				if (tBinding.vecBound[0] != pObject)
				{
					tBinding.pVariable->Set(pObject);
					tBinding.vecBound[0] = pObject;
				}
			}
			else if (tBinding.eType == Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV)
			{
				auto Find = umapTexture.find(tBinding.strName);
				Diligent::RESOURCE_DIMENSION eDim = Diligent::RESOURCE_DIM_TEX_2D;
				auto FindDim = umapTextureDim.find(tBinding.strName);
				if (FindDim != umapTextureDim.end()) eDim = FindDim->second;
				Diligent::ITextureView* pDefault = Get_DefaultTextureView(pDevice.Get(), eDim);

				bool isChanged = false;
				std::vector<Diligent::IDeviceObject*> vecObjects(tBinding.iArraySize, pDefault);
				if (Find != umapTexture.end())
				{
					for (uint32 i = 0; i < tBinding.iArraySize && i < Find->second.size(); ++i)
						if (Find->second[i] != nullptr)
							vecObjects[i] = Find->second[i].Get();
				}
				for (uint32 i = 0; i < tBinding.iArraySize; ++i)
					if (tBinding.vecBound[i] != vecObjects[i])
						isChanged = true;

				if (isChanged)
				{
					if (tBinding.iArraySize == 1)
						tBinding.pVariable->Set(vecObjects[0]);
					else
						tBinding.pVariable->SetArray(vecObjects.data(), 0, tBinding.iArraySize);
					tBinding.vecBound = vecObjects;
				}
			}
		}
		return S_OK;
	}
};

/* ================================================================================================ */
/*                                           CShader                                                 */
/* ================================================================================================ */

CShader::CShader(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CResource(pDevice,pContext), m_pEffect(nullptr), m_strPath({}), m_iTotalPassNum{ 0 }
{
	m_eResourceType = RESOURCE_TYPE::SHADER;
}

CShader::CShader(const CShader& Prototype)
	: CResource(Prototype), m_pEffect(Prototype.m_pEffect), m_wstrFileName(Prototype.m_wstrFileName)
	, m_strPath(Prototype.m_strPath), m_iTotalPassNum(Prototype.m_iTotalPassNum)
{
}


HRESULT CShader::Shader_Initialize(const wstring& strPath, const INPUT_ELEMENT_DESC* tInputElement_Desc, uint32 iInputElementNum)
{
	if (FAILED(__super::Proto_Initialize()))
		return E_FAIL;

	m_wstrFileName = std::filesystem::path(strPath).stem().wstring();

	m_pEffect = std::make_shared<EFFECT>();
	m_pEffect->pDevice = m_pDevice;
	m_pEffect->pContext = m_pDeviceContext;
	m_pEffect->wstrFileName = m_wstrFileName;

	if (FAILED(LoadShaderFromFile(strPath)))
		return E_FAIL;

	if (FAILED(m_pEffect->Build(tInputElement_Desc, iInputElementNum)))
	{
		wstring Error_MSG = m_wstrFileName + L" Shader Compile Failed";
		MessageBox(nullptr, Error_MSG.c_str(), L"System Message", MB_OK);
		return E_FAIL;
	}

	/* Pass 개수 저장후  */
	m_iTotalPassNum = static_cast<uint32>(m_pEffect->vecPass.size());

	if (FAILED(Ready_GlobalEffectVariable()))
		return E_FAIL;

	return S_OK;
}

HRESULT CShader::Copy_Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CShader::Ready_GlobalEffectVariable()
{
	auto GameMgr = CGameManager::GetInstance();

	/* Camera / Light Group / Mouse 상수버퍼 Link (원본과 동일하게 PipeLine 의 공용 상수버퍼를 연결한다) */
	if (Has_ConstantBuffer(GB_Shader_CameraBuffer))
		GameMgr->PipeLine_Link_EffectConstantBuffer(GLOBAL_CB_TYPE::CAMERA, this);
	if (Has_ConstantBuffer(GB_Shader_LightGroupBuffer))
		GameMgr->PipeLine_Link_EffectConstantBuffer(GLOBAL_CB_TYPE::LIGHTGROUP, this);
	if (Has_ConstantBuffer(GB_Shader_MouseBuffer))
		GameMgr->PipeLine_Link_EffectConstantBuffer(GLOBAL_CB_TYPE::MOUSE, this);

	return S_OK;
}

HRESULT CShader::LoadShaderFromFile(const wstring& strPath)
{
	OutputDebugStringW((L"[FX Compile Test] Path = " + strPath + L"\n").c_str());

	if (!Engine::File_Exists(strPath))
	{
		MessageBoxW(nullptr, (L"File not found:\n" + strPath).c_str(), L"FX Compile Error", MB_OK);
		return E_FAIL;
	}

	m_strPath = strPath;

	return m_pEffect->Parse(strPath);
}

vector<string> CShader::Get_ALlPassName()
{
	vector<string> vecPasName{};

	if (m_pEffect == nullptr)
		return vecPasName;

	for (auto& Pass : m_pEffect->vecPass)
		vecPasName.push_back(Pass.strName);

	return vecPasName;
}

uint32 CShader::Get_PassCount() const
{
	return m_pEffect != nullptr ? static_cast<uint32>(m_pEffect->vecPass.size()) : 0;
}

#pragma region Defualt Texture Bind
HRESULT CShader::Set_GPU_Texture(const string& strValuealbeName, ShaderResourceView* pSRV)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);

	auto Find = m_pEffect->umapTextureArraySize.find(strValuealbeName);
	if (Find == m_pEffect->umapTextureArraySize.end())
		return E_FAIL;

	auto& vecTexture = m_pEffect->umapTexture[strValuealbeName];
	vecTexture.resize((std::max)(1u, Find->second));
	vecTexture[0] = pSRV;
	if (pSRV != nullptr)
		m_pEffect->umapTextureDim[strValuealbeName] = pSRV->GetTexture()->GetDesc().Type;
	return S_OK;
}
HRESULT CShader::Set_GPU_Texture(ShaderResourceView* pShaderResourceView)
{
	return Set_GPU_Texture(GB_Shader_Texture, pShaderResourceView);
}
HRESULT CShader::Set_GPU_Textures(uint32 iCountTex, ShaderResourceView** ppStartSRV, uint32 iOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	NULLPTR_RETURN_EFAIL(ppStartSRV);

	auto Find = m_pEffect->umapTextureArraySize.find(GB_Shader_Textures);
	if (Find == m_pEffect->umapTextureArraySize.end())
		return E_FAIL;

	auto& vecTexture = m_pEffect->umapTexture[GB_Shader_Textures];
	vecTexture.resize((std::max)(Find->second, iOffset + iCountTex));
	for (uint32 i = 0; i < iCountTex; ++i)
		vecTexture[iOffset + i] = ppStartSRV[i];
	return S_OK;
}
#pragma endregion

#pragma region Defualt Mix Textures Bind
HRESULT CShader::Set_GPU_MixTextureArr(uint32 iCountTex, ShaderResourceView** ppMixTexutres, uint32 iOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	NULLPTR_RETURN_EFAIL(ppMixTexutres);

	auto Find = m_pEffect->umapTextureArraySize.find(GB_Shader_MixTextureArr);
	if (Find == m_pEffect->umapTextureArraySize.end())
		return E_FAIL;

	auto& vecTexture = m_pEffect->umapTexture[GB_Shader_MixTextureArr];
	vecTexture.resize((std::max)(Find->second, iOffset + iCountTex));
	for (uint32 i = 0; i < iCountTex; ++i)
		vecTexture[iOffset + i] = ppMixTexutres[i];
	return S_OK;
}
HRESULT CShader::Set_GPU_MaskTexture(ShaderResourceView* pShaderResourceView)
{
	return Set_GPU_Texture(GB_Shader_MaskTexture, pShaderResourceView);
}
#pragma endregion
#pragma region Material Texture Bind
HRESULT CShader::Set_GPU_MaterialTextures(ShaderResourceView** ppMaterialRSV)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	NULLPTR_RETURN_EFAIL(ppMaterialRSV);

	auto Find = m_pEffect->umapTextureArraySize.find(GB_Shader_MaterialTexture);
	if (Find == m_pEffect->umapTextureArraySize.end())
		return E_FAIL;

	auto& vecTexture = m_pEffect->umapTexture[GB_Shader_MaterialTexture];
	vecTexture.resize((std::max)(Find->second, Cast_Uint(MATERIAL_TYPE::END)));
	for (uint32 i = 0; i < Cast_Uint(MATERIAL_TYPE::END); ++i)
		vecTexture[i] = ppMaterialRSV[i];
	return S_OK;
}
#pragma endregion


HRESULT CShader::Set_GPU_CameraBuffer(const CAM_BUF_DATA& tCamBuffer, uint32 iByteOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	const int32 iCB = m_pEffect->Find_CB(GB_Shader_CameraBuffer);
	if (iCB < 0) return E_FAIL;
	return m_pEffect->vecCB[iCB].pOwnBuffer->Write(&tCamBuffer, (std::min)(static_cast<uint32>(sizeof(CAM_BUF_DATA)), m_pEffect->vecCB[iCB].iSize - iByteOffset), iByteOffset);
}

HRESULT CShader::Set_GPU_LightGroupBuffer(const LIGHT_GROUP_BUF* tLightBuffer, uint32 iByteOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	NULLPTR_RETURN_EFAIL(tLightBuffer);
	const int32 iCB = m_pEffect->Find_CB(GB_Shader_LightGroupBuffer);
	if (iCB < 0) return E_FAIL;
	return m_pEffect->vecCB[iCB].pOwnBuffer->Write(tLightBuffer, (std::min)(static_cast<uint32>(sizeof(LIGHT_GROUP_BUF)), m_pEffect->vecCB[iCB].iSize - iByteOffset), iByteOffset);
}

HRESULT CShader::Set_GPU_UIAplha(float fAlpha)
{
	return Set_RawValue(GB_Shader_UIAlpha, &fAlpha, sizeof(float));
}

HRESULT CShader::Set_GPU_DT(float fDT)
{
	return Set_RawValue(GB_Shader_DT, &fDT, sizeof(float));
}

HRESULT CShader::Set_GPU_Ratio(float fRatio)
{
	if (fRatio <= 0) fRatio = 0.f;
	else if(fRatio >= 1.f) fRatio = 1.f;

	return Set_RawValue("g_Ratio", &fRatio, sizeof(float));
}

HRESULT CShader::Set_GPU_Color(const Color& vColor)
{
	return Set_RawValue("g_Color", &vColor.x, sizeof(Color));
}

HRESULT CShader::Set_GPU_ConstantBuffer(const string& strFindKey, const void* pCB, uint32 iCBSize, uint32 iByteOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);

	/* cbuffer 이름 → 버퍼 전체 , 변수 이름 → 해당 변수 */
	const int32 iCB = m_pEffect->Find_CB(strFindKey);
	if (iCB >= 0)
	{
		const uint32 iSize = (std::min)(iCBSize, m_pEffect->vecCB[iCB].iSize > iByteOffset ? m_pEffect->vecCB[iCB].iSize - iByteOffset : 0u);
		return m_pEffect->vecCB[iCB].pOwnBuffer->Write(pCB, iSize, iByteOffset);
	}
	return Set_RawValue(strFindKey, pCB, iCBSize, iByteOffset);
}


HRESULT CShader::Set_GPU_Vector(const Float4& tVector, const string& strFindKey)
{
	return Set_RawValue(strFindKey, &tVector.x, sizeof(Float4));
}

HRESULT CShader::Set_GPU_Vector(const float* fStartX, const string& strFindKey)
{
	return Set_RawValue(strFindKey, fStartX, sizeof(Float4));
}


HRESULT CShader::Set_GPU_Transform(const Float4x4& matWorld)
{
	return Set_RawValue(GB_Shader_WorldMatrix, &matWorld._11, sizeof(Float4x4));
}

HRESULT CShader::Set_GPU_Transform(const XMatrix_1& matWorld)
{
	Float4x4 matStore;
	XMStoreFloat4x4(&matStore, matWorld);
	return Set_GPU_Transform(matStore);
}

HRESULT CShader::Set_GPU_Canvas(const Float4x4& matCanvas)
{
	return Set_RawValue(GB_Shader_CanvasMatrix, &matCanvas._11, sizeof(Float4x4));
}

HRESULT CShader::Set_GPU_Matrix(const string& strFindKey, const Float4x4& Matrix)
{
	return Set_RawValue(strFindKey, &Matrix._11, sizeof(Float4x4));
}

HRESULT CShader::Set_GPU_Matrices(const string& strFindKey, const Float4x4* pMatrix, uint32 iCountMatrix, uint32 iOffset)
{
	return Set_RawValue(strFindKey, pMatrix, iCountMatrix * sizeof(Float4x4), iOffset * sizeof(Float4x4));
}

HRESULT CShader::Set_RawValue(const std::string& strVarName, const void* pData, uint32 iSize, uint32 iByteOffset)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	NULLPTR_RETURN_EFAIL(pData);

	auto Find = m_pEffect->umapVariable.find(strVarName);
	if (Find == m_pEffect->umapVariable.end())
		return E_FAIL;

	const EFFECT::VARIABLE& tVar = Find->second;
	if (iByteOffset >= tVar.iSize)
		return E_FAIL;
	const uint32 iCopySize = (std::min)(iSize, tVar.iSize - iByteOffset);

	return m_pEffect->vecCB[tVar.iCBIndex].pOwnBuffer->Write(pData, iCopySize, tVar.iOffset + iByteOffset);
}


HRESULT CShader::Apply_Pass(const string& strPassKey , int32 iIndex)
{
	const int32 iPass = Find_PassIndex(strPassKey);
	if (iPass < 0)
		return E_FAIL;

	CGraphicDevice* pGraphicDevice = CGraphicDevice::Get_Instance();
	NULLPTR_RETURN_EFAIL(pGraphicDevice);

	/* Effects11 Apply 와 같이 패스에 지정된 상태만 장치 상태를 덮어쓴다 */
	const EFFECT::PASS& tPass = m_pEffect->vecPass[iPass];
	if (tPass.hasRasterizer)
		pGraphicDevice->RS_SetState(tPass.tRasterizer);
	if (tPass.hasDepthStencil)
		pGraphicDevice->OM_SetDepthStencilState(tPass.tDepthStencil, tPass.iStencilRef);
	if (tPass.hasBlend)
		pGraphicDevice->OM_SetBlendState(tPass.tBlend, tPass.fBlendFactor);

	pGraphicDevice->Set_CurrentPass(this, static_cast<uint32>(iPass));

	return S_OK;
}

int32 CShader::Find_PassIndex(const std::string& strPassKey) const
{
	if (m_pEffect == nullptr)
		return -1;
	for (size_t i = 0; i < m_pEffect->vecPass.size(); ++i)
		if (m_pEffect->vecPass[i].strName == strPassKey)
			return static_cast<int32>(i);
	return -1;
}

HRESULT CShader::Prepare_Draw(uint32 iPassIndex, const GRAPHIC_PIPELINE_STATE& tState, Diligent::IPipelineState** ppPSO, Diligent::IShaderResourceBinding** ppSRB)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	if (iPassIndex >= m_pEffect->vecPass.size())
		return E_FAIL;

	EFFECT::PASS& tPass = m_pEffect->vecPass[iPassIndex];

	EFFECT::PSO_ENTRY* pEntry = nullptr;
	for (EFFECT::PSO_ENTRY& tEntry : tPass.vecPSO)
	{
		if (EFFECT::Is_SameState(tEntry.tState, tState))
		{
			pEntry = &tEntry;
			break;
		}
	}

	if (pEntry == nullptr)
	{
		EFFECT::PSO_ENTRY tNewEntry{};
		if (FAILED(m_pEffect->Create_PSO(tPass, tState, tNewEntry)))
		{
			Engine::Engine_Log("[Shader] PSO Create Failed : %s / %s", WString_To_UTF8(m_wstrFileName).c_str(), tPass.strName.c_str());
			tPass.vecPSO.push_back(tNewEntry);
			return E_FAIL;
		}
		tPass.vecPSO.push_back(tNewEntry);
		pEntry = &tPass.vecPSO.back();
	}

	if (pEntry->pPSO == nullptr || pEntry->iSRBIndex < 0)
		return E_FAIL;

	EFFECT::SRB_ENTRY& tSRB = tPass.vecSRB[pEntry->iSRBIndex];
	m_pEffect->Bind_Resources(tSRB);

	*ppPSO = pEntry->pPSO.Get();
	*ppSRB = tSRB.pSRB.Get();
	return S_OK;
}

HRESULT CShader::Add_EffectConstantBuffer(const std::string& strKey)
{
	if (strKey.empty()) return E_FAIL;

	return Has_ConstantBuffer(strKey) ? S_OK : E_FAIL;
}

HRESULT CShader::Add_EffectVariable(EFFECT_VAL_TYPE eValType, const std::string& strKey)
{
	if (SUCCEEDED(Check_EffectVariable(eValType, strKey)))
		return S_OK;

	/* Effects11 과 달리 셰이더 컴파일러는 사용하지 않는 변수를 제거한다.             */
	/* 파일에 선언은 되어 있지만 어떤 패스에서도 쓰지 않는 변수는 오류로 보지 않는다. */
	if (m_pEffect != nullptr && m_pEffect->strSource.find(strKey) != std::string::npos)
		return S_OK;

	wstring wstrError = String_To_WString(strKey) + L" Variable Can't Found in ShaderFile ";
	MessageBox(nullptr,wstrError.c_str(),L"System Message" ,MB_OK);
	return E_FAIL;
}

HRESULT CShader::Bind_ConstantBuffer(const std::string& strCBName, CConstantBufferBase* pConstantBuffer)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);
	const int32 iCB = m_pEffect->Find_CB(strCBName);
	if (iCB < 0)
		return E_FAIL;

	EFFECT::CONSTANT_BUFFER& tCB = m_pEffect->vecCB[iCB];
	Safe_Release(tCB.pExternal);
	tCB.pExternal = pConstantBuffer;
	Safe_AddRef(tCB.pExternal);
	return S_OK;
}

bool CShader::Has_ConstantBuffer(const std::string& strCBName) const
{
	return m_pEffect != nullptr && m_pEffect->Find_CB(strCBName) >= 0;
}

HRESULT CShader::Check_EffectVariable(EFFECT_VAL_TYPE eValType, const std::string& strFindKey)
{
	NULLPTR_RETURN_EFAIL(m_pEffect);

	if (eValType == EFFECT_VAL_TYPE::TEXTURE)
		return m_pEffect->umapTextureArraySize.count(strFindKey) != 0 ? S_OK : E_FAIL;

	return m_pEffect->umapVariable.count(strFindKey) != 0 ? S_OK : E_FAIL;
}


CShader* CShader::Create(const wstring& strPath, const INPUT_ELEMENT_DESC* pInputElement_Desc, uint32 iInputElementNum, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CShader* pShader = new CShader(pDevice, pContext);

	if (FAILED(pShader->Shader_Initialize(strPath, pInputElement_Desc, iInputElementNum)))
	{
		Safe_Release(pShader);
		MSG_BOX("Shader is failed to Create");
		return nullptr;
	}

	return pShader;
}
CResource* CShader::Copy_Resource(void* pArg)
{
	CShader* pShader = new CShader(*this);

	if (FAILED(pShader->Copy_Initialize(pArg)))
	{
		Safe_Release(pShader);
		MSG_BOX("Copy Resource is failed to Shader");
		return nullptr;
	}

	return pShader;
}

void CShader::Release_SharedResources()
{
	g_pWhiteTexture.Reset();
	g_pWhiteTextureArray.Reset();
}

void CShader::Free()
{
	__super::Free();
	/* CShader::Free()*/

	/* 마지막 복사본이 해제될 때 Effect 가 해제된다 */
	m_pEffect.reset();
}
