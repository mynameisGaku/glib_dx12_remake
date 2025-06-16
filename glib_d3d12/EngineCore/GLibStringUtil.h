#pragma once
#include <string>
#include <sstream>  
#include <iomanip>  
#include <Windows.h>

namespace glib
{
    namespace StringUtil
    {
        // wstring to string
        static std::string WStringToString(const std::wstring& wstr)
        {
            if (wstr.empty()) return {};

            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
            return strTo;
        }

        // string to wstring
        static std::wstring StringToWString(const std::string& str)
        {
            if (str.empty()) return {};

            int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
            std::wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
            return wstrTo;
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
