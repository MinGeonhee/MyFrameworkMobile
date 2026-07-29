#pragma once
#include "CComponent.h"
#include <functional>

class CTexture;
class CShader;
class CVIBuffer_Rect;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CSpriteAnimator final : public CComponent
{
public:
    // 아틀라스의 행마다 실제 프레임 수가 다른 경우 사용하는 애니메이션 클립이다.
    // iRowIndex와 iStartColumn은 0부터 시작한다.
    struct SPRITE_CLIP_DESC
    {
        uint32 iRowIndex = 0;
        uint32 iStartColumn = 0;
        uint32 iFrameCount = 1;
        float fFramePerSecond = 12.f;
        bool isLoop = true;
    };

    struct SPRITEANIMATOR_DESC
    {
        CTexture* pTexture = nullptr;
        CShader* pShader = nullptr;
        CVIBuffer_Rect* pVIBuffer = nullptr;
        SPRITE_TEXTURE_MODE eTextureMode = SPRITE_TEXTURE_MODE::ATLAS_GRID;
        uint32 iAtlasColumnCount = 1;
        uint32 iAtlasRowCount = 1;
        uint32 iTotalFrameCount = 0;
        uint32 iStartFrame = 0;
        uint32 iEndFrame = UINT32_MAX;
        float fFramePerSecond = 12.f;
        bool isLoop = true;
        bool isAutoPlay = true;
    };

private:
    explicit CSpriteAnimator(Device* pDevice, Context* pDeviceContext);
    explicit CSpriteAnimator(const CSpriteAnimator& Prototype);
    ~CSpriteAnimator() override = default;

public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    void Update(float fTimeDelta) override;
    HRESULT Render() override;

    void Play(bool isRestart = true);
    HRESULT Play(uint32 iStartFrame, uint32 iEndFrame, bool isLoop);
    HRESULT Add_Clip(const std::wstring& strClipKey, const SPRITE_CLIP_DESC& Desc);
    HRESULT Play_Clip(const std::wstring& strClipKey, bool isRestart = true);
    bool Has_Clip(const std::wstring& strClipKey) const;
    void Pause();
    void Resume();
    void Stop(bool isResetFrame = true);
    HRESULT Set_CurrentFrame(uint32 iFrame);
    void Set_FramePerSecond(float fFPS);
    void Set_Loop(bool isLoop) { m_isLoop = isLoop; }
    void Set_PlayFinishedCallback(std::function<void()> Callback) { m_PlayFinishedCallback = std::move(Callback); }

    bool Get_IsPlaying() const { return m_isPlaying; }
    bool Get_IsPaused() const { return m_isPaused; }
    uint32 Get_CurrentFrame() const { return m_iCurrentFrame; }
    uint32 Get_TotalFrameCount() const { return m_iTotalFrameCount; }
    const std::wstring& Get_CurrentClipKey() const { return m_strCurrentClipKey; }
    CVIBuffer_Rect* Get_VIBuffer() const { return m_pVIBuffer; }

    static CSpriteAnimator* Create(Device* pDevice, Context* pDeviceContext);
    CComponent* Clone(void* pArg) override;
    void Free() override;

private:
    HRESULT Apply_CurrentFrame();
    void Advance_Frame();

private:
    CTexture* m_pTexture = nullptr;
    CShader* m_pShader = nullptr;
    CVIBuffer_Rect* m_pVIBuffer = nullptr;
    SPRITE_TEXTURE_MODE m_eTextureMode = SPRITE_TEXTURE_MODE::ATLAS_GRID;
    uint32 m_iAtlasColumnCount = 1;
    uint32 m_iAtlasRowCount = 1;
    uint32 m_iTotalFrameCount = 1;
    uint32 m_iStartFrame = 0;
    uint32 m_iEndFrame = 0;
    uint32 m_iCurrentFrame = 0;
    float m_fFramePerSecond = 12.f;
    float m_fAccTime = 0.f;
    bool m_isLoop = true;
    bool m_isPlaying = false;
    bool m_isPaused = false;
    std::function<void()> m_PlayFinishedCallback;
    std::unordered_map<std::wstring, SPRITE_CLIP_DESC> m_Clips;
    std::wstring m_strCurrentClipKey;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
