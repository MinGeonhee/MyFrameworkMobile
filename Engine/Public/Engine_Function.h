#pragma once

//#include "Engine_Define.h"

namespace Engine
{
	template<class T>
	void	Safe_Delete(T& Instance)
	{
		if (Instance)
		{
			delete Instance;
			Instance = nullptr;
		}
	}

	template<class T>
	void	Safe_Delete_Arr(T& Instance)
	{
		if (Instance)
		{
			delete[] Instance;
			Instance = nullptr;
		}
	}

	template<class T>
	uint32	Safe_AddRef(T& Instance)
	{
		uint32 Cnt = 0;

		if (Instance)
			Cnt = Instance->AddRef();

		return Cnt;
	}

	template<class T>
	uint32	Safe_Release(T& Instance)
	{
		uint32 Cnt = 0;

		if (Instance)
		{
			Cnt = Instance->Release();

			if (Cnt == 0)
				Instance = nullptr;
		}

		return Cnt;
	}


#if defined(_WIN32)
	inline std::string WString_To_String(const std::wstring& wstr)
	{
		int wstringsize = ::WideCharToMultiByte(CP_ACP, 0 , wstr.data() , (int)wstr.size() , nullptr ,0 ,nullptr , nullptr );

		std::string str(wstringsize,0);
		::WideCharToMultiByte(CP_ACP,0, wstr.data(), static_cast<int>(wstr.size()), &str[0] , wstringsize , nullptr , nullptr );

		return str;
	}

	inline std::wstring String_To_WString(const std::string& str)
	{
		int stringsize = ::MultiByteToWideChar(CP_ACP, 0, str.data(), (int)str.size(), nullptr, 0);

		std::wstring wstr(stringsize, 0);
		::MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), &wstr[0], stringsize);

		return wstr;
	}

	inline std::string WString_To_UTF8(const std::wstring& wstr)
	{
		int iSize = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string str(iSize, 0);
		::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &str[0], iSize, nullptr, nullptr);
		return str;
	}
#else
	/* Android : 시스템 코드페이지가 UTF-8 이므로 UTF-8 <-> UTF-32(wchar_t) 변환을 직접 수행한다. */
	inline std::string WString_To_UTF8(const std::wstring& wstr)
	{
		std::string str;
		str.reserve(wstr.size());
		for (wchar_t wc : wstr)
		{
			uint32_t c = static_cast<uint32_t>(wc);
			if (c < 0x80) str.push_back(static_cast<char>(c));
			else if (c < 0x800) { str.push_back(static_cast<char>(0xC0 | (c >> 6))); str.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
			else if (c < 0x10000) { str.push_back(static_cast<char>(0xE0 | (c >> 12))); str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); str.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
			else { str.push_back(static_cast<char>(0xF0 | (c >> 18))); str.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F))); str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); str.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
		}
		return str;
	}

	inline std::wstring String_To_WString(const std::string& str)
	{
		std::wstring wstr;
		wstr.reserve(str.size());
		for (size_t i = 0; i < str.size();)
		{
			const unsigned char c = static_cast<unsigned char>(str[i]);
			uint32_t cp = 0; size_t n = 0;
			if (c < 0x80) { cp = c; n = 1; }
			else if ((c >> 5) == 0x6) { cp = c & 0x1F; n = 2; }
			else if ((c >> 4) == 0xE) { cp = c & 0x0F; n = 3; }
			else if ((c >> 3) == 0x1E) { cp = c & 0x07; n = 4; }
			else { ++i; continue; }
			for (size_t k = 1; k < n && i + k < str.size(); ++k)
				cp = (cp << 6) | (static_cast<unsigned char>(str[i + k]) & 0x3F);
			wstr.push_back(static_cast<wchar_t>(cp));
			i += n;
		}
		return wstr;
	}

	inline std::string WString_To_String(const std::wstring& wstr)
	{
		return WString_To_UTF8(wstr);
	}
#endif

	inline std::string NormalizePath(const std::filesystem::path& path)
	{
		std::string s = path.lexically_normal().string();  // ../, ./ 정리 + 문자열 변환
		std::replace(s.begin(), s.end(), '\\', '/');       // 윈도우 백슬래시 → 슬래시
		return s;
	}

	inline std::wstring NormalizePath_WString(const std::filesystem::path& path)
	{
		std::wstring s = path.lexically_normal().wstring();  // ../, ./ 정리 + 문자열 변환
		std::replace(s.begin(), s.end(), '\\', '/');       // 윈도우 백슬래시 → 슬래시
		return s;
	}

	inline XMFLOAT3 QuaternionToEulerDeg_Simple(const XMVECTOR& Quaternion)
	{
		XMFLOAT4 f;
		XMStoreFloat4(&f, Quaternion);

		// 참고: f = (x, y, z, w);

		// Pitch (X)
		float pitch = std::asin(2.f * (f.w * f.x - f.y * f.z));

		// Yaw (Y)
		float yaw = std::atan2(2.f * (f.w * f.y + f.z * f.x),
			1.f - 2.f * (f.x * f.x + f.y * f.y));

		// Roll (Z)
		float roll = std::atan2(2.f * (f.w * f.z + f.x * f.y),
			1.f - 2.f * (f.y * f.y + f.z * f.z));

		return {
			XMConvertToDegrees(pitch),
			XMConvertToDegrees(yaw),
			XMConvertToDegrees(roll)
		};
	}

}