#pragma once
#include <memory>
#include <string>

namespace Win32Util {namespace HttpUtil {
	typedef struct tagResponse
	{
		unsigned long statusCode;
		std::string text;
	}Response;

	class CHttpClient
	{
	private:
		class Impl;
		std::shared_ptr<Impl> pimpl;

	public:
		CHttpClient();
		~CHttpClient() = default;

		void SetHeader(const std::wstring& sHeader);
		Response get(const std::wstring& sUrl);
	};
}
}
