#pragma once
#include <memory>
#include <string>

namespace Win32Util {namespace HttpUtil {
	class CHttpClient
	{
	private:
		class Impl;
		std::shared_ptr<Impl> pimpl;

	public:
		CHttpClient();
		~CHttpClient() = default;

		void SetHeader(const std::wstring& sHeader);
		void get(const std::wstring& sUrl);
	};
}
}
