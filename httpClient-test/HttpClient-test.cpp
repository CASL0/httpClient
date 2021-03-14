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

	TEST(HttpClientTest, StatusOk)
	{
		::HttpUtil::CHttpClient httpClient;
		std::wstring sUrl = L"https://www.microsoft.com";
		auto response = httpClient.get(sUrl);
		EXPECT_EQ(response.statusCode, 200);
	}

	TEST(HttpClientTest, StatusNotFound)
	{
		::HttpUtil::CHttpClient httpClient;
		std::wstring sUrl = L"https://www.microsoft.com/hoge";
		auto response = httpClient.get(sUrl);
		EXPECT_EQ(response.statusCode, 404);

	}
}