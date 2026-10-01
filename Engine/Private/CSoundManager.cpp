#include "CSoundManager.h"

/* FMOD 는 Windows 라이브러리만 포함되어 있다. Android 는 FMOD Android SDK(.so) 를 추가하기 전까지 소리 없이 동작한다. */
#if defined(_WIN32)
#include "fmod.h"
#include "fmod.hpp"

#if defined(_DEBUG)
#  pragma comment(lib, "fmodL_vc.lib")
#else
#  pragma comment(lib, "fmod_vc.lib")
#endif

IMPLEMENT_SINGLETON(CSoundManager)

CSoundManager::CSoundManager()
	:m_pSystem(nullptr), m_iTotalChannelCount{0}
	, m_vecChannel{}
{
}

CSoundManager::~CSoundManager()
{
	Free();
}

void CSoundManager::Initialize(uint32 iTotalChannelCount)
{
	m_iTotalChannelCount = iTotalChannelCount;
	m_vecChannel.resize(m_iTotalChannelCount);


	FMOD_System_Create(&m_pSystem, FMOD_VERSION);
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);
	LoadSoundFile();
}

void CSoundManager::Free()
{
	for (auto& kv : m_mapSound)
	{
		FMOD_Sound_Release(kv.second);
	}
	m_mapSound.clear();

	FMOD_System_Close(m_pSystem);
	FMOD_System_Release(m_pSystem);
	m_pSystem = nullptr;
}

void CSoundManager::SoundPlay(const std::wstring& soundKey, uint32 iChannelID, float fVolume, bool loop)
{
	if (m_iTotalChannelCount < iChannelID) return;

	auto iter = m_mapSound.find(soundKey);
	if (iter == m_mapSound.end())
		return;

	if (m_vecChannel[iChannelID])
		FMOD_Channel_Stop(m_vecChannel[iChannelID]);

	FMOD_System_PlaySound(m_pSystem, iter->second, 0, FALSE, &m_vecChannel[iChannelID]);

	if (loop) {
		FMOD_Channel_SetMode(m_vecChannel[iChannelID], FMOD_LOOP_NORMAL);
		FMOD_Channel_SetLoopCount(m_vecChannel[iChannelID], -1); // 무한 반복
	}
	else {
		FMOD_Channel_SetMode(m_vecChannel[iChannelID], FMOD_LOOP_OFF);
	}

	FMOD_Channel_SetVolume(m_vecChannel[iChannelID], fVolume * m_vols.master * m_fSoundVolumeSetting);
	FMOD_System_Update(m_pSystem);
}

void CSoundManager::Sound_Fade_Out(uint32 iChannelID, float fDuration)
{
	unsigned long long	iDspClock{};
	unsigned long long	iParentClock{};
	unsigned long long	iTargetClock{};
	int32 iRate{};

	FMOD::Channel* pCh = reinterpret_cast<FMOD::Channel*>(m_vecChannel[iChannelID]);
	if (!pCh) return;
	
	pCh->getDSPClock(&iDspClock,&iParentClock);
	
	FMOD::System* pSystem = reinterpret_cast<FMOD::System*>(m_pSystem);
	FMOD_SPEAKERMODE eMode;
	int32 iNumDrivers{};
	pSystem->getSoftwareFormat(&iRate,&eMode,&iNumDrivers);

	iTargetClock = iParentClock + static_cast<unsigned long long>(iRate * fDuration);
	float fCurVolume{ 0.f };
	pCh->getVolume(&fCurVolume);

	pCh->addFadePoint(iDspClock,fCurVolume);
	pCh->addFadePoint(iDspClock + iTargetClock ,0.f);

	pCh->setDelay(0 , iParentClock + iTargetClock, true );
}

void CSoundManager::PlayBGM(const std::wstring& soundKey, uint32 iBgmChannelID  ,float fVolume)
{
	if (m_iTotalChannelCount < iBgmChannelID) return;

	auto iter = m_mapSound.find(soundKey);
	if (iter == m_mapSound.end())
		return;

	if (m_vecChannel[iBgmChannelID])
		FMOD_Channel_Stop(m_vecChannel[iBgmChannelID]);

	FMOD_System_PlaySound(
		m_pSystem,
		iter->second,
		0,
		FALSE,
		&m_vecChannel[iBgmChannelID]
	);
	FMOD_Channel_SetMode(m_vecChannel[iBgmChannelID], FMOD_LOOP_NORMAL);
	FMOD_Channel_SetVolume(m_vecChannel[iBgmChannelID], fVolume * m_vols.master * m_fSoundVolumeSetting);
	FMOD_System_Update(m_pSystem);
}


void CSoundManager::SoundStop(uint32 iChannelID)
{
	/* < allowed iChannelID == count (one past the end), and a finished channel
	   can already be null - StopAll below guards it, this path did not. */
	if (iChannelID >= m_vecChannel.size()) return;
	if (m_vecChannel[iChannelID] == nullptr) return;

	FMOD_Channel_Stop(m_vecChannel[iChannelID]);
}

void CSoundManager::StopAll()
{
	for(auto& Channel : m_vecChannel)
		if(Channel)
			FMOD_Channel_Stop(Channel);
}

void CSoundManager::SetChannelVolume(uint32 eChannelID, float fVolume)
{
	FMOD_Channel_SetVolume(m_vecChannel[eChannelID], fVolume * m_vols.master * m_fSoundVolumeSetting);
	FMOD_System_Update(m_pSystem);
}

bool CSoundManager::IsPlayingSound(uint32 iChannelID)
{
	if (iChannelID >= m_iTotalChannelCount) return false; 
	FMOD::Channel* pChannel =  reinterpret_cast<FMOD::Channel*>(m_vecChannel[iChannelID]);
	if (pChannel == nullptr)  return false;

	bool isPlaying{false};
	pChannel->isPlaying(&isPlaying);

	return isPlaying;
}

string CSoundManager::Get_CurChannel_SoundName(uint32 iChannelID)
{
	if (iChannelID >= m_iTotalChannelCount) return string();
	FMOD::Channel* pChannel = reinterpret_cast<FMOD::Channel*>(m_vecChannel[iChannelID]);
	if (pChannel == nullptr)  return string();

	bool isPlaying{ false };
	pChannel->isPlaying(&isPlaying);

	if (isPlaying == false) return string();

	FMOD::Sound* pSound{ nullptr };
	pChannel->getCurrentSound(&pSound);
	if (pSound == nullptr) return string();


	char szName[MAX_PATH];
	pSound->getName(szName,MAX_PATH);
	
	return string(szName);
}


HRESULT CSoundManager::LoadSoundFile()
{
	// exe 기준 ../../../content/sound
	filesystem::path SoundFilePath = L"../../Resource/Sound/";
	/* Mobile 구조 : 실행 폴더가 Client/Bin/Debug 이므로 한 단계 더 올라간다 */
	std::error_code ec;
	if (!filesystem::exists(SoundFilePath, ec))
		SoundFilePath = L"../../../Resource/Sound/";
	if (!filesystem::exists(SoundFilePath, ec))
		return E_FAIL;
	
	/* 파일 순회 */
	for ( auto& Path :  filesystem::recursive_directory_iterator(SoundFilePath) )
	{
		path	FilePath = Path;
		wstring	wstrDefaultFullPath = filesystem::absolute(FilePath).wstring();
		wstring wstrFileExt			= FilePath.extension().wstring();
		wstring wstrFileName		= FilePath.filename().stem().wstring();

		if (wstrFileExt != L".wav")
			continue;

		std::string u8 = W2U8(wstrDefaultFullPath);

		FMOD_SOUND* pSound = nullptr;
		FMOD_RESULT hr = FMOD_System_CreateSound(m_pSystem, u8.c_str() , FMOD_DEFAULT, 0, &pSound);

		if (hr != FMOD_OK)
			return E_FAIL;

		m_mapSound.emplace(wstrFileName , pSound);

	}

	FMOD_System_Update(m_pSystem);

	return E_FAIL;

	//std::wstring soundDir = UpW(ExeDirW(), 3) + L"\\content\\sound";

	//// 디버그 출력
	//OutputDebugStringW((L"[Sound] Scan Dir: " + soundDir + L"\n").c_str());

	//WIN32_FIND_DATAW wfd{};
	//HANDLE h = FindFirstFileW((soundDir + L"\\*.*").c_str(), &wfd); // ★ 패턴 중요
	//if (h == INVALID_HANDLE_VALUE) {
	//	OutputDebugStringW(L"[Sound] 폴더가 없거나 비었습니다.\n");
	//	return;
	//}

	//do {
	//	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

	//	std::wstring wfull = soundDir + L"\\" + wfd.cFileName; // 전체 경로(유니코드)
	//	std::string  u8 = W2U8(wfull);                      // FMOD용 UTF-8

	//	FMOD_SOUND* pSound = nullptr;
	//	FMOD_RESULT r = FMOD_System_CreateSound(m_pSystem, u8.c_str(), FMOD_DEFAULT, 0, &pSound);
	//	if (r == FMOD_OK) {
	//		m_mapSound.emplace(wfd.cFileName, pSound);
	//	}
	//	else 
	//	{
	//		MSG_BOX("파일 찾기 실패");
	//	}
	//} while (FindNextFileW(h, &wfd));
	//FindClose(h);
}

#else /* !_WIN32 */

IMPLEMENT_SINGLETON(CSoundManager)

CSoundManager::CSoundManager()
	:m_pSystem(nullptr), m_iTotalChannelCount{0}
	, m_vecChannel{}
{
}

CSoundManager::~CSoundManager()
{
}

void    CSoundManager::Initialize(uint32 iTotalChannelCount) { m_iTotalChannelCount = iTotalChannelCount; }
void    CSoundManager::SoundPlay(const wstring& pSoundKey, uint32 iChannelID, float fVolume, bool loop) {}
void    CSoundManager::Sound_Fade_Out(uint32 iChannelID, float fDuration) {}
void    CSoundManager::PlayBGM(const std::wstring& soundKey, uint32 iBgmChannelID, float fVolume) {}
void    CSoundManager::SoundStop(uint32 iChannelID) {}
void    CSoundManager::StopAll() {}
void    CSoundManager::SetChannelVolume(uint32 iChannelID, float fVolume) {}
bool    CSoundManager::IsPlayingSound(uint32 iChannelID) { return false; }
string  CSoundManager::Get_CurChannel_SoundName(uint32 iChannelID) { return string(); }
HRESULT CSoundManager::LoadSoundFile() { return S_OK; }
void    CSoundManager::Free() {}

#endif
