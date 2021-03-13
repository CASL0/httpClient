#include <gtest/gtest.h>
#include "../httpClient/HttpClient.h"
#include "../httpClient/Win32Exception.h"

using namespace Win32Util;

namespace {
	TEST(HttpClientTest, InvalidUrl)
	{
		::HttpUtil::CHttpClient httpClient;
		std::wstring sUrl = L"http://";
		EXPECT_THROW(httpClient.get(sUrl), ::CWin32Exception);
	}
}