#include "Engine_Define.h"
#include "Engine_FileIO.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include <android/log.h>
#endif

namespace
{
	void* g_pAssetManager = nullptr;
}

namespace Engine
{
	void Set_Platform_AssetManager(void* pAssetManager)
	{
		g_pAssetManager = pAssetManager;
	}

	void* Get_Platform_AssetManager()
	{
		return g_pAssetManager;
	}

	std::string To_AssetPath(const std::wstring& wstrPath)
	{
		std::string strPath = WString_To_UTF8(wstrPath);
		std::replace(strPath.begin(), strPath.end(), '\\', '/');

		/* "./" , "../" 를 정리한 뒤 앞쪽에 남은 "../" 는 모두 제거한다 */
		std::vector<std::string> vecParts;
		std::stringstream ss(strPath);
		std::string strPart;
		while (std::getline(ss, strPart, '/'))
		{
			if (strPart.empty() || strPart == ".")
				continue;
			if (strPart == "..")
			{
				if (!vecParts.empty() && vecParts.back() != "..")
					vecParts.pop_back();
				continue;
			}
			vecParts.push_back(strPart);
		}

		std::string strResult;
		for (size_t i = 0; i < vecParts.size(); ++i)
		{
			if (i > 0) strResult += '/';
			strResult += vecParts[i];
		}
		return strResult;
	}

	void Engine_Log(const char* pFormat, ...)
	{
		char szBuffer[4096];
		va_list Args;
		va_start(Args, pFormat);
		std::vsnprintf(szBuffer, sizeof(szBuffer), pFormat, Args);
		va_end(Args);

#if defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_INFO, "MyFrameworkMobile", "%s", szBuffer);
#else
		OutputDebugStringA(szBuffer);
		OutputDebugStringA("\n");

		static std::mutex s_LogMutex;
		static std::ofstream s_LogFile("EngineLog.txt", std::ios::out | std::ios::trunc);
		std::lock_guard<std::mutex> Lock(s_LogMutex);
		if (s_LogFile.is_open())
		{
			s_LogFile << szBuffer << '\n';
			s_LogFile.flush();
		}
#endif
	}

	bool File_ReadAll(const std::wstring& wstrPath, std::vector<uint8_t>& vecOut)
	{
		vecOut.clear();
#if defined(__ANDROID__)
		AAssetManager* pAssetManager = static_cast<AAssetManager*>(g_pAssetManager);
		if (pAssetManager == nullptr)
			return false;

		const std::string strAssetPath = To_AssetPath(wstrPath);
		AAsset* pAsset = AAssetManager_open(pAssetManager, strAssetPath.c_str(), AASSET_MODE_BUFFER);
		if (pAsset == nullptr)
		{
			__android_log_print(ANDROID_LOG_WARN, "MyFrameworkMobile", "Asset not found : %s", strAssetPath.c_str());
			return false;
		}

		const off_t iLength = AAsset_getLength(pAsset);
		vecOut.resize(static_cast<size_t>(iLength));
		const int iRead = AAsset_read(pAsset, vecOut.data(), static_cast<size_t>(iLength));
		AAsset_close(pAsset);
		return iRead == iLength;
#else
		std::ifstream fin(std::filesystem::path(wstrPath), std::ios::binary);
		if (!fin.is_open())
			return false;

		fin.seekg(0, std::ios::end);
		const std::streamoff iLength = fin.tellg();
		fin.seekg(0, std::ios::beg);
		if (iLength < 0)
			return false;

		vecOut.resize(static_cast<size_t>(iLength));
		if (iLength > 0)
			fin.read(reinterpret_cast<char*>(vecOut.data()), iLength);
		return true;
#endif
	}

	bool File_ReadAll(const std::string& strPath, std::vector<uint8_t>& vecOut)
	{
		return File_ReadAll(std::filesystem::path(strPath).wstring(), vecOut);
	}

	bool File_ReadText(const std::wstring& wstrPath, std::string& strOut)
	{
		std::vector<uint8_t> vecData;
		if (!File_ReadAll(wstrPath, vecData))
			return false;

		size_t iStart = 0;
		/* UTF-8 BOM 제거 */
		if (vecData.size() >= 3 && vecData[0] == 0xEF && vecData[1] == 0xBB && vecData[2] == 0xBF)
			iStart = 3;
		strOut.assign(reinterpret_cast<const char*>(vecData.data()) + iStart, vecData.size() - iStart);
		return true;
	}

	bool File_Exists(const std::wstring& wstrPath)
	{
#if defined(__ANDROID__)
		AAssetManager* pAssetManager = static_cast<AAssetManager*>(g_pAssetManager);
		if (pAssetManager == nullptr)
			return false;
		AAsset* pAsset = AAssetManager_open(pAssetManager, To_AssetPath(wstrPath).c_str(), AASSET_MODE_UNKNOWN);
		if (pAsset == nullptr)
			return false;
		AAsset_close(pAsset);
		return true;
#else
		std::error_code ec;
		return std::filesystem::exists(std::filesystem::path(wstrPath), ec);
#endif
	}

	bool CFileStream::Open(const std::wstring& wstrPath)
	{
		m_iPos = 0;
		m_iLastRead = 0;
		m_isFail = false;
		m_isEOF = false;
		m_isOpen = File_ReadAll(wstrPath, m_vecData);
		if (!m_isOpen)
			m_isFail = true;
		return m_isOpen;
	}

	bool CFileStream::Open(const std::string& strPath)
	{
		return Open(std::filesystem::path(strPath).wstring());
	}

	CFileStream& CFileStream::read(void* pDst, size_t iSize)
	{
		const size_t iAvailable = m_iPos < m_vecData.size() ? m_vecData.size() - m_iPos : 0;
		const size_t iCopy = iSize <= iAvailable ? iSize : iAvailable;
		if (iCopy > 0 && pDst != nullptr)
			::memcpy(pDst, m_vecData.data() + m_iPos, iCopy);
		m_iPos += iCopy;
		m_iLastRead = iCopy;
		if (iCopy < iSize)
		{
			m_isFail = true;
			m_isEOF = true;
		}
		return *this;
	}
}

#if defined(__ANDROID__)
/* Engine_Platform.h 의 Windows API 대체 함수 */
int Engine_Android_MessageBoxW(void*, const wchar_t* pText, const wchar_t* pCaption, unsigned int)
{
	const std::string strText = pText != nullptr ? Engine::WString_To_UTF8(pText) : std::string{};
	const std::string strCaption = pCaption != nullptr ? Engine::WString_To_UTF8(pCaption) : std::string{};
	__android_log_print(ANDROID_LOG_ERROR, "MyFrameworkMobile", "[%s] %s", strCaption.c_str(), strText.c_str());
	return 0;
}

void Engine_Android_OutputDebugStringW(const wchar_t* pText)
{
	if (pText == nullptr) return;
	__android_log_print(ANDROID_LOG_INFO, "MyFrameworkMobile", "%s", Engine::WString_To_UTF8(pText).c_str());
}

void Engine_Android_OutputDebugStringA(const char* pText)
{
	if (pText == nullptr) return;
	__android_log_print(ANDROID_LOG_INFO, "MyFrameworkMobile", "%s", pText);
}
#endif
