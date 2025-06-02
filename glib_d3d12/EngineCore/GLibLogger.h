#pragma once
#include <stdio.h>
#include <Windows.h>
#include <string>

namespace glib
{
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
            std::string logMessage = "[NULL MESSAGE]";

            switch (level)
            {
            case LogLevel::Debug:
                // Debugレベルのログは通常、開発中にのみ使用されるため、特に目立たせない
                logMessage = "[DEBUG] ";
                break;
            case LogLevel::Info:
                // Infoレベルのログは通常、アプリケーションの状態や進行状況を示すために使用される
                logMessage = "[INFO] ";
                break;
            case LogLevel::Warning:
                // Warningレベルのログは、潜在的な問題を示すために使用される
                logMessage = "[WARNING] ";
                break;
            case LogLevel::Error:
                // Errorレベルのログは、アプリケーションの動作に影響を与える問題を示すために使用される
                logMessage = "[ERROR] ";
                break;
            case LogLevel::Critical:
                // Criticalレベルのログは、アプリケーションのクラッシュや重大な問題を示すために使用される
                logMessage = "[CRITICAL] ";
                break;
            }

            logMessage += message;
            logMessage += "\n";

#ifdef UNICODE
            std::wstring wLogMessage(logMessage.begin(), logMessage.end());
            OutputDebugStringW(wLogMessage.c_str());
#else
            OutputDebugStringA(logMessage.c_str());
#endif
        }

        // Optional: Overload for formatted strings
        template<typename... Args>
        static void FormatLog(LogLevel level, const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            Log(level, buffer);
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

        static void FormatDebugLog(const std::string& message, ...)
        {
#ifndef _DEBUG
            return;
#else
            FormatLog(LogLevel::Debug, "[DEBUG] %s", message.c_str());
#endif
        }

        static void FormatInfoLog(const std::string& message, ...)
        {
            FormatLog(LogLevel::Info, "[INFO] %s", message.c_str());
        }

        static void FormatWarningLog(const std::string& message, ...)
        {
            FormatLog(LogLevel::Warning, "[WARNING] %s", message.c_str());
        }

        static void FormatErrorLog(const std::string& message, ...)
        {
            FormatLog(LogLevel::Error, "[ERROR] %s", message.c_str());
        }

        static void FormatCriticalLog(const std::string& message, ...)
        {
            FormatLog(LogLevel::Critical, "[CRITICAL] %s", message.c_str());
        }
    }
}