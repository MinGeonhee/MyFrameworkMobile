#pragma once
#include "CBase.h"

NS_BEGIN(Engine)


struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;

class ENGINE_DLL CSoundManager :
    public CBase
{
    DECLARE_SINGLETON(CSoundManager)
public:
    explicit CSoundManager();
    virtual ~CSoundManager();
public:
    struct AudioVolumes {
        float master = 1.0f;
        float sfx = 0.8f;
        float music = 0.2f;
        float ambient = 0.5f;
    };
    void    Initialize( uint32 iTotalChannelCount );
public:
    void    SoundPlay(const wstring& pSoundKey, uint32 iChannelID , float fVolume, bool loop = false);
    void    Sound_Fade_Out(uint32 iChannelID,float fDuration);
    
    void    PlayBGM(const std::wstring& soundKey, uint32 iBgmChannelID,float fVolume);

    void    SoundStop(uint32 iChannelID);
    void    StopAll();

    void    SetChannelVolume(uint32 iChannelID, float fVolume);

    void    SetSoundSettingScale(float fSound) { m_fSoundVolumeSetting = fSound; };

    bool    IsPlayingSound(uint32 iChannelID);

    string  Get_CurChannel_SoundName(uint32 iChannelID);
    

    inline void SetVol(const AudioVolumes& v) { m_vols = v; }
    inline AudioVolumes GetVol() const { return m_vols; }

private:
    HRESULT  LoadSoundFile();

    static std::wstring ExeDirW() {
#if !defined(_WIN32)
        return L".";
#else
        wchar_t buf[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        std::wstring s(buf);
        size_t p = s.find_last_of(L"\\/");
        if (p != std::wstring::npos) s.resize(p);
        return s; // ...\Client\Bin\Debug
#endif
    }

    static std::wstring UpW(std::wstring p, int n) {
        while (n-- > 0) {
            size_t pos = p.find_last_of(L"\\/");
            if (pos == std::wstring::npos) break;
            p.resize(pos);
        }
        return p;
    }
    // UTF-8 변환
    static std::string W2U8(const std::wstring& ws) {
#if !defined(_WIN32)
        return WString_To_UTF8(ws);
#else
        int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string out(len ? len - 1 : 0, '\0');
        if (len > 1) WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &out[0], len - 1, nullptr, nullptr);
        return out;
#endif
    }
private:
    map<std::wstring, FMOD_SOUND*> m_mapSound;

    // FMOD_CHANNEL : 재생하고 있는 사운드를 관리할 객체
    vector<FMOD_CHANNEL*>       m_vecChannel;

    uint32                      m_iTotalChannelCount;

    // 사운드, 채널 객체 및 장치를 관리하는 객체
    FMOD_SYSTEM*        m_pSystem;
    float               m_fSoundVolumeSetting = 1.f;

private:
    virtual     void    Free()override;

    AudioVolumes m_vols;
};

NS_END
