#include "HttpClient.h"
#include "Win32Exception.h"
#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib,"winhttp.lib")

namespace Win32Util {namespace HttpUtil {
	class CHttpClient::Impl
	{
	public:
		const std::wstring UA = L"WinHttp Client";

	public:
		Impl();
		void SetHeader(const std::wstring& sHeader);
		void get(const std::wstring& sUrl);

	public:
		std::shared_ptr<std::remove_pointer<HINTERNET>::type> m_hSession;
		std::wstring m_sHeaders;
	};

	CHttpClient::Impl::Impl() :
		m_hSession(
			WinHttpOpen(UA.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC),
			WinHttpCloseHandle
		),
		m_sHeaders(std::wstring())
	{
		ThrowLastError(m_hSession == nullptr, "WinHttpOpen failed");
	}

	void CHttpClient::Impl::SetHeader(const std::wstring& sHeader)
	{
		m_sHeaders += sHeader;
		m_sHeaders += L"\r\n";
	}

	void CHttpClient::Impl::get(const std::wstring& sUrl)
	{
	}

	CHttpClient::CHttpClient() :pimpl(std::make_shared<Impl>())
	{
	}

	void CHttpClient::SetHeader(const std::wstring& sHeader)
	{
		pimpl->SetHeader(sHeader);
	}

	void CHttpClient::get(const std::wstring& sUrl)
	{
		pimpl->get(sUrl);
	}

}//namespace HttpUtil
}//namespace Win32Util