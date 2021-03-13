#pragma once
#pragma once
#include <Windows.h>
#include <vector>
#include <stdexcept>
#include <string>

namespace Win32Util {

    inline std::string FormatErrorMessage(DWORD error, const std::string& msg);
    inline void ThrowLastError(bool expression, const std::string& msg);


    class CWin32Exception : public std::runtime_error
    {
    private:
        DWORD m_dwError;
    public:
        CWin32Exception(DWORD error, const std::string& msg)
            : runtime_error(FormatErrorMessage(error, msg)), m_dwError(error) { }

        DWORD GetErrorCode() const { return m_dwError; }
    };

    std::string FormatErrorMessage(DWORD error, const std::string& msg)
    {
        constexpr size_t BUFFERLENGTH = 1024;
        std::vector<CHAR> buf(BUFFERLENGTH);
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, buf.data(), BUFFERLENGTH - 1, 0);
        return std::string(buf.data()) + "   (" + msg + ")";
    }

    void ThrowLastError(bool expression, const std::string& msg)
    {
        if (expression)
        {
            throw CWin32Exception(GetLastError(), msg);
        }
    }

}