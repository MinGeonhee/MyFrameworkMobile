#pragma once

/* ------------------------------------------------------------------------------------------- */
/* 파일 읽기 공용 함수                                                                           */
/* Windows : 실행 폴더 기준 상대경로 그대로 디스크에서 읽는다.                                     */
/* Android : APK assets 에서 읽는다. "../../../Resource/A.png" 처럼 앞쪽의 "../" 를 제거한        */
/*           "Resource/A.png" 를 asset 경로로 사용한다. (Gradle 이 저장소 루트 기준으로 복사한다)   */
/* ------------------------------------------------------------------------------------------- */

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <ios>
#include <nlohmann/json.hpp>

namespace Engine
{
	ENGINE_DLL bool			File_ReadAll(const std::wstring& wstrPath, std::vector<uint8_t>& vecOut);
	ENGINE_DLL bool			File_ReadAll(const std::string& strPath, std::vector<uint8_t>& vecOut);
	ENGINE_DLL bool			File_ReadText(const std::wstring& wstrPath, std::string& strOut);
	ENGINE_DLL bool			File_Exists(const std::wstring& wstrPath);

	/* 원본 코드의 std::ifstream(Path, binary) 를 대체하기 위한 스트림 (Android asset 도 읽을 수 있다) */
	class ENGINE_DLL CFileStream final
	{
	public:
		CFileStream() = default;
		explicit CFileStream(const std::wstring& wstrPath) { Open(wstrPath); }
		explicit CFileStream(const std::string& strPath) { Open(strPath); }
		/* std::ifstream(Path , std::ios::binary) 형태 그대로 쓸 수 있게 한다 (모드는 무시 : 항상 바이너리) */
		CFileStream(const std::wstring& wstrPath, std::ios_base::openmode) { Open(wstrPath); }
		CFileStream(const std::string& strPath, std::ios_base::openmode) { Open(strPath); }
		CFileStream(const wchar_t* pPath, std::ios_base::openmode = std::ios_base::in) { Open(std::wstring(pPath)); }
		CFileStream(const char* pPath, std::ios_base::openmode = std::ios_base::in) { Open(std::string(pPath)); }

		bool				Open(const std::wstring& wstrPath);
		bool				Open(const std::string& strPath);
		bool				is_open() const { return m_isOpen; }
		void				close() { m_isOpen = false; m_vecData.clear(); m_iPos = 0; }
		/* std::istream 과 같이 끝을 넘어서 읽으려 했을 때 true */
		bool				eof() const { return m_isEOF; }
		bool				good() const { return m_isOpen && !m_isFail; }
		bool				fail() const { return m_isFail; }
		explicit			operator bool() const { return m_isOpen && !m_isFail; }

		CFileStream&		read(void* pDst, size_t iSize);
		CFileStream&		read(char* pDst, size_t iSize) { return read(static_cast<void*>(pDst), iSize); }
		size_t				gcount() const { return m_iLastRead; }
		size_t				tellg() const { return m_iPos; }
		CFileStream&		seekg(size_t iPos) { m_iPos = iPos <= m_vecData.size() ? iPos : m_vecData.size(); return *this; }
		size_t				size() const { return m_vecData.size(); }
		const uint8_t*		data() const { return m_vecData.data(); }
		std::string			str() const { return std::string(reinterpret_cast<const char*>(m_vecData.data()), m_vecData.size()); }

	private:
		std::vector<uint8_t>	m_vecData;
		size_t					m_iPos = 0;
		size_t					m_iLastRead = 0;
		bool					m_isOpen = false;
		bool					m_isFail = false;
		bool					m_isEOF = false;
	};

	/* std::ifstream >> nlohmann::json 대체 */
	inline CFileStream& operator>>(CFileStream& tStream, nlohmann::json& tJson)
	{
		tJson = nlohmann::json::parse(tStream.str(), nullptr, false);
		return tStream;
	}
	inline CFileStream& operator>>(CFileStream& tStream, nlohmann::ordered_json& tJson)
	{
		tJson = nlohmann::ordered_json::parse(tStream.str(), nullptr, false);
		return tStream;
	}

	/* Android : NativeActivity 의 AssetManager 를 엔진에 넘겨준다 (AAssetManager*) */
	ENGINE_DLL void			Set_Platform_AssetManager(void* pAssetManager);
	ENGINE_DLL void*		Get_Platform_AssetManager();

	/* 엔진 로그 : Windows = OutputDebugString + EngineLog.txt , Android = logcat */
	ENGINE_DLL void			Engine_Log(const char* pFormat, ...);

	/* "../../../Resource/A.png" → "Resource/A.png" (Android asset 경로) */
	ENGINE_DLL std::string	To_AssetPath(const std::wstring& wstrPath);
}
