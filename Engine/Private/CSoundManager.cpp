#include "pch.h"
#include "CSoundManager.h"
IMPLEMENT_SINGLETON(CSoundManager)
HRESULT CSoundManager::Initialize(uint32 count){if(!count)return E_INVALIDARG;m_Channels.assign(count,{});return S_OK;}
HRESULT CSoundManager::Add_Sound(const std::wstring& key,const std::wstring& path){if(key.empty()||path.empty())return E_INVALIDARG;return m_Sounds.emplace(key,path).second?S_OK:E_FAIL;}
HRESULT CSoundManager::SoundPlay(const std::wstring& key,uint32 channel,float volume,bool loop)
{
    if(channel>=m_Channels.size()||m_Sounds.find(key)==m_Sounds.end())return E_INVALIDARG;
    auto& c=m_Channels[channel];c={key,std::clamp(volume,0.f,1.f)*m_fSoundVolumeSetting,loop,true};
    // FMOD는 Android 패키징이 정해진 뒤 연결한다. 현재는 채널/리소스 관리와 백엔드 경계를 유지한다.
    return m_PlayHandler?m_PlayHandler(key,channel,c.fVolume,loop):S_OK;
}
void CSoundManager::SoundStop(uint32 channel){if(channel<m_Channels.size())m_Channels[channel]={};}
void CSoundManager::StopAll(){for(auto& c:m_Channels)c={};}
void CSoundManager::SetChannelVolume(uint32 channel,float volume){if(channel<m_Channels.size())m_Channels[channel].fVolume=std::clamp(volume,0.f,1.f);}
bool CSoundManager::IsPlayingSound(uint32 channel)const{return channel<m_Channels.size()&&m_Channels[channel].isPlaying;}
std::wstring CSoundManager::Get_CurChannel_SoundName(uint32 channel)const{return channel<m_Channels.size()?m_Channels[channel].strSoundKey:L"";}
void CSoundManager::Update(float){}
void CSoundManager::Free(){StopAll();m_Channels.clear();m_Sounds.clear();m_PlayHandler={};CBase::Free();}
