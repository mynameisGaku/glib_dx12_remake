#pragma once

#include <windows.h>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <GLib.h>

namespace glib
{

    inline std::wstring ConvertAnsiToWide(const std::string& ansiStr)
    {
        if (ansiStr.empty()) return L"";

        int size_needed = MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, nullptr, 0);
        if (size_needed == 0) return L"[•ÏŠ·Ž¸”s]";

        std::wstring wideStr(size_needed, 0);
        MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, &wideStr[0], size_needed);
        return wideStr;
    }

    inline std::string GetCurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_c);

        std::ostringstream oss;
        oss << "[" << std::put_time(&local_tm, "%H:%M:%S") << "]";
        return oss.str();
    }

    namespace Logger
    {
        enum class LogLevel
        {
            Debug,
            Info,
            Warning,
            Error,
            Critical
        };

        static void Log(LogLevel level, const char* message)
        {
            std::string logMessage;

            switch (level)
            {
            case LogLevel::Debug:
                logMessage = "[DEBUG] [GLIB] ";
                break;
            case LogLevel::Info:
                logMessage = "[INFO] [GLIB] ";
                break;
            case LogLevel::Warning:
                logMessage = "[WARNING] [GLIB] ";
                break;
            case LogLevel::Error:
                logMessage = "[ERROR] [GLIB] ";
                break;
            case LogLevel::Critical:
                logMessage = "[CRITICAL] [GLIB] ";
                break;
            }

            logMessage += glib::GetCurrentTimeString();
            logMessage += " ";
            logMessage += message;
            logMessage += "\n";

#ifdef UNICODE
            std::wstring wLogMessage = glib::ConvertAnsiToWide(logMessage);
            OutputDebugStringW(wLogMessage.c_str());
#else
            OutputDebugStringA(logMessage.c_str());
#endif
        }

        inline void LogFormattedArgs(LogLevel level, const char* format, va_list args)
        {
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Log(level, buffer);
        }

        template<typename... Args>
        static void FormatLog(LogLevel level, const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            LogFormattedArgs(level, format, args);
            va_end(args);
        }

        static void DebugLog(const std::string& message)
        {
#ifndef _DEBUG
            return;
#else
            Log(LogLevel::Debug, message.c_str());
#endif
        }

        static void InfoLog(const std::string& message)
        {
            Log(LogLevel::Info, message.c_str());
        }

        static void WarningLog(const std::string& message)
        {
            Log(LogLevel::Warning, message.c_str());
        }

        static void ErrorLog(const std::string& message)
        {
            Log(LogLevel::Error, message.c_str());
        }

        static void CriticalLog(const std::string& message)
        {
            Log(LogLevel::Critical, message.c_str());
        }

        static void FormatDebugLog(const char* format, ...)
        {
#ifndef _DEBUG
            return;
#else
            va_list args;
            va_start(args, format);
            LogFormattedArgs(LogLevel::Debug, format, args);
            va_end(args);
#endif
        }

        static void FormatInfoLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            LogFormattedArgs(LogLevel::Info, format, args);
            va_end(args);
        }

        static void FormatWarningLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            LogFormattedArgs(LogLevel::Warning, format, args);
            va_end(args);
        }

        static void FormatErrorLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            LogFormattedArgs(LogLevel::Error, format, args);
            va_end(args);
        }

        static void FormatCriticalLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            LogFormattedArgs(LogLevel::Critical, format, args);
            va_end(args);
        }
    }
}