#include <iostream>
#include "../httpClient/HttpClient.h"
#include "../httpClient/Win32Exception.h"
#pragma comment(lib, "httpClient.lib")

int main()
{
	using namespace Win32Util;
	HttpUtil::CHttpClient httpClient;

	HttpUtil::Response res;
	try
	{
		res = httpClient.get(L"https://www.microsoft.com");
	}
	catch (CWin32Exception& e)
	{
		std::cerr << "fatal error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "Status Code: " << res.statusCode << std::endl;
	std::cout << "Response Body: " << std::endl << res.text << std::endl;
	return 0;
}
