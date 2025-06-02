#pragma once
#include <string>

namespace glib
{
    namespace StringUtil
    {
        // wstring to string
        static std::string WStringToString(const std::wstring& wstr)
        {
            return std::string(wstr.begin(), wstr.end());
        }

        // string to wstring
        static std::wstring StringToWString(const std::string& str)
        {
            return std::wstring(str.begin(), str.end());
        }
    }
}