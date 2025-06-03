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

        static void FormatDebugLog(const char* format, ...)
        {
#ifndef _DEBUG
            return;
#else
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            std::string s(buffer);
            s.insert(0, "[DEBUG] ");
            Log(LogLevel::Debug , buffer);
#endif
        }

        static void FormatInfoLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            std::string s(buffer);
            s.insert(0, "[INFO] ");
            Log(LogLevel::Info, buffer);
        }

        static void FormatWarningLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            std::string s(buffer);
            s.insert(0, "[WARNING] ");
            Log(LogLevel::Warning, buffer);
        }

        static void FormatErrorLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            std::string s(buffer);
            s.insert(0, "[Error] ");
            Log(LogLevel::Error, buffer);
        }

        static void FormatCriticalLog(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            std::string s(buffer);
            s.insert(0, "[CRITICAL] ");
            Log(LogLevel::Critical, buffer);
        }
    }
}