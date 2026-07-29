#pragma once
#include "CBase.h"
#include <functional>
class ENGINE_DLL CSoundManager final:public CBase
{
    DECLARE_SINGLETON(CSoundManager)
public:
    struct AudioVolumes{float master=1.f,sfx=.8f,music=.2f,ambient=.5f;};
    struct CHANNEL_STATE{std::wstring strSoundKey;float fVolume=1.f;bool isLoop=false,isPlaying=false;};
    using PlayHandler=std::function<HRESULT(const std::wstring&,uint32,float,bool)>;
private:CSoundManager()=default;
public:
    ~CSoundManager()override=default;
    HRESULT Initialize(uint32 count);HRESULT Add_Sound(const std::wstring& key,const std::wstring& path);
    HRESULT SoundPlay(const std::wstring& key,uint32 channel,float volume,bool loop=false);
    void PlayBGM(const std::wstring& key,uint32 channel,float volume){SoundPlay(key,channel,volume,true);}
    void SoundStop(uint32 channel);void StopAll();void SetChannelVolume(uint32 channel,float volume);
    bool IsPlayingSound(uint32 channel)const;std::wstring Get_CurChannel_SoundName(uint32 channel)const;
    void SetSoundSettingScale(float scale){m_fSoundVolumeSetting=scale;}void SetVol(const AudioVolumes& v){m_Volumes=v;}AudioVolumes GetVol()const{return m_Volumes;}
    void Set_PlayHandler(PlayHandler Handler){m_PlayHandler=std::move(Handler);}void Update(float);void Free()override;
private:std::unordered_map<std::wstring,std::wstring>m_Sounds;std::vector<CHANNEL_STATE>m_Channels;float m_fSoundVolumeSetting=1.f;AudioVolumes m_Volumes{};PlayHandler m_PlayHandler;
};
