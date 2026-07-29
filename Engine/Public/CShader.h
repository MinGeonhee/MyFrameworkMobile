#pragma once

#include "CResource.h"
#include "Engine_Struct.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CShader final : public CResource
{
private:
    struct PASS
    {
        std::string strPassKey;
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> pPipelineState;
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> pShaderResourceBinding;
    };
private:
    explicit CShader(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext);
    explicit CShader(const CShader& Prototype);
    ~CShader() override = default;

    HRESULT Initialize(const Engine::SHADER_PASS_DESC* pPassDescs, uint32 iPassCount);
    HRESULT Copy_Initialize(void* pArg) override;
    PASS* Find_Pass(const std::string& strPassKey);

public:
    // Effect11의 Technique/Pass 호출 형태는 유지한다.
    // 내부 구현만 Diligent의 PSO(Pipeline State Object)로 바뀐다.
    HRESULT Apply_Pass(const std::string& strPassKey, int32 iIndex = 0);
    HRESULT Apply_Pass(uint32 iPassIndex);

    HRESULT Set_GPU_Texture(const std::string& strFindKey, Diligent::ITextureView* pTextureView);
    HRESULT Set_GPU_Texture(Diligent::ITextureView* pTextureView);
    HRESULT Set_GPU_Textures(uint32 iCountTex, Diligent::ITextureView** ppTextureViews, uint32 iOffset = 0);
    HRESULT Set_GPU_ConstantBuffer(const std::string& strBufferName,
                                   const void* pData,
                                   uint32 iDataSize,
                                   uint32 iByteOffset = 0);

    uint32 Get_PassCount() const { return static_cast<uint32>(m_vecPass.size()); }
    std::vector<std::string> Get_AllPassName() const;

    static CShader* Create(const Engine::SHADER_PASS_DESC* pPassDescs,
                          uint32 iPassCount,
                           Diligent::IRenderDevice* pDevice,
                           Diligent::IDeviceContext* pDeviceContext);
    CResource* Copy_Resource(void* pArg = nullptr) override;
    void Free() override;

private:
    std::vector<PASS> m_vecPass;
    std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IBuffer>> m_mapConstantBuffer;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
