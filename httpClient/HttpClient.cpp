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
		Response get(const std::wstring& sUrl);

		DWORD ChooseAuthScheme(DWORD dwSupportedSchemes);

		//URLをホスト名とパスに分解する
		//sUrl --> (sHostName, sPath)
		void ParseUrl(const std::wstring& sUrl, std::wstring& sHostName, std::wstring& sPath);

	public:
		std::shared_ptr<std::remove_pointer<HINTERNET>::type> m_hSession;
		std::wstring m_sHeaders;
	};

	CHttpClient::Impl::Impl() :
		m_hSession(
			WinHttpOpen(UA.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0),
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

	Response CHttpClient::Impl::get(const std::wstring& sUrl)
	{
		std::wstring sHostName;
		std::wstring sPath;
		ParseUrl(sUrl, sHostName, sPath);

		std::shared_ptr<std::remove_pointer<HINTERNET>::type> hConnect(
			WinHttpConnect(m_hSession.get(), sHostName.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0),
			WinHttpCloseHandle
		);
		ThrowLastError(hConnect == nullptr, "WinHttpConnect failed");

		std::shared_ptr<std::remove_pointer<HINTERNET>::type> hRequest(
			WinHttpOpenRequest(hConnect.get(), L"GET", sPath.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE),
			WinHttpCloseHandle
		);
		ThrowLastError(hRequest == nullptr, "WinHttpOpenRequest failed");

		BOOL bDone = FALSE;
		BOOL bRet;
		Response response = { 0 };
		DWORD dwSize = sizeof(DWORD);
		while (!bDone)
		{
			bRet = WinHttpSendRequest(hRequest.get(), m_sHeaders.length() == 0 ? nullptr : m_sHeaders.c_str(), m_sHeaders.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, (DWORD_PTR)nullptr);
			ThrowLastError(bRet == FALSE, "WinHttpSendRequest failed");

			try
			{
				bRet = WinHttpReceiveResponse(hRequest.get(), nullptr);
				ThrowLastError(bRet == FALSE, "WinHttpReceiveResponse failed");
			}
			catch (CWin32Exception& e)
			{
				if (e.GetErrorCode() != ERROR_WINHTTP_RESEND_REQUEST)
				{
					throw;
				}
			}

			bRet = WinHttpQueryHeaders(hRequest.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &response.statusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
			ThrowLastError(bRet == FALSE, "WinHttpQueryHeaders failed");

			switch (response.statusCode)
			{
			case 200:
			{
				bDone = TRUE;
				break;
			}
			case 401:
			{
				DWORD dwSupportedSchemes, dwFirstScheme, dwTarget;
				bRet = WinHttpQueryAuthSchemes(hRequest.get(), &dwSupportedSchemes, &dwFirstScheme, &dwTarget);
				ThrowLastError(bRet == FALSE, "WinHttpQueryAuthSchemes failed");

				DWORD dwSelectedScheme = ChooseAuthScheme(dwSupportedSchemes);
				if (dwSelectedScheme == 0)
				{
					return response;
				}

				bRet = WinHttpSetCredentials(hRequest.get(), dwTarget, dwSelectedScheme, L"user", L"pass", nullptr);
				ThrowLastError(bRet == FALSE, "WinHttpSetCredentials failed");

				break;
			}
			default:
			{
				return response;
			}
			}

		}//while(!bDone)


		//200 OK
		for (;;)
		{
			bRet = WinHttpQueryDataAvailable(hRequest.get(), &dwSize);
			ThrowLastError(bRet == FALSE, "WinHttpQueryDataAvailable");

			if (dwSize == 0)
			{
				break;
			}
			std::vector<CHAR> szBuffer(dwSize + 1);
			bRet = WinHttpReadData(hRequest.get(), (LPVOID)szBuffer.data(), dwSize, nullptr);
			response.text += std::string(szBuffer.data());
		}

		return response;
	}

	DWORD CHttpClient::Impl::ChooseAuthScheme(DWORD dwSupportedSchemes)
	{
		if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
		{
			return WINHTTP_AUTH_SCHEME_NEGOTIATE;
		}
		else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
		{
			return WINHTTP_AUTH_SCHEME_NTLM;
		}
		else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
		{
			return WINHTTP_AUTH_SCHEME_PASSPORT;
		}
		else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
		{
			return WINHTTP_AUTH_SCHEME_DIGEST;
		}
		else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_BASIC)
		{
			return WINHTTP_AUTH_SCHEME_BASIC;
		}
		else
		{
			return 0;
		}
	}
	
	void CHttpClient::Impl::ParseUrl(const std::wstring& sUrl, std::wstring& sHostName, std::wstring& sPath)
	{
		URL_COMPONENTS urlComponents = { 0 };
		urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
		std::vector<WCHAR> szHost((DWORD)-1);
		std::vector<WCHAR> szPath((DWORD)-1);
		urlComponents.lpszHostName = szHost.data();
		urlComponents.dwHostNameLength = szHost.size();
		urlComponents.lpszUrlPath = szPath.data();
		urlComponents.dwUrlPathLength = szPath.size();
		BOOL bRet = WinHttpCrackUrl(sUrl.c_str(), sUrl.length(), 0, &urlComponents);
		ThrowLastError(bRet == FALSE, "WinHttpCrackUrl failed");

		sHostName = std::wstring(szHost.data());
		sPath = std::wstring(szPath.data());
	}

	CHttpClient::CHttpClient() :pimpl(std::make_shared<Impl>())
	{
	}

	void CHttpClient::SetHeader(const std::wstring& sHeader)
	{
		pimpl->SetHeader(sHeader);
	}

	Response CHttpClient::get(const std::wstring& sUrl)
	{
		return pimpl->get(sUrl);
	}

}//namespace HttpUtil
}//namespace Win32Util