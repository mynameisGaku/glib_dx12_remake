#pragma once
#include <string>
#include <sstream>  
#include <iomanip>  

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

        // format
        std::string FormatHex(size_t value)
        {
            std::stringstream stream;
            stream << std::hex << std::uppercase << value;
            return stream.str();
        }
    }
}