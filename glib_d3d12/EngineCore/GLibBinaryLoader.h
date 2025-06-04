#pragma once
#include <fstream>
#include <GLibLogger.h>

namespace glib
{
    class GLibBinaryLoader
    {
    public:
        GLibBinaryLoader(const char* filename) :m_Succeeded(false)
        {
            std::ifstream ifs(filename, std::ios::binary);
            if (ifs.fail())
            {
                glib::Logger::FormatErrorLog("Failed to open Binary file: %s", filename);
                return;
            }
            glib::Logger::FormatDebugLog("Loading Binary file: %s", filename);
            m_Succeeded = true;
            std::istreambuf_iterator<char> first(ifs);
            std::istreambuf_iterator<char> last;
            m_Buffer.assign(first, last);
            ifs.close();
            if (m_Buffer.empty())
            {
                glib::Logger::FormatErrorLog("Binary file is empty: %s", filename);
                m_Succeeded = false;
            }
            else
            {
                glib::Logger::FormatDebugLog("Loaded Binary file size: %zu bytes", m_Buffer.size());
            }
        }
        bool Succeeded() const
        {
            return m_Succeeded;
        }
        unsigned char* Code() const
        {
            char* p = const_cast<char*>(m_Buffer.data());
            return reinterpret_cast<unsigned char*>(p);
        }
        size_t Size() const
        {
            return m_Buffer.size();
        }
    private:
        std::string m_Buffer;
        bool m_Succeeded;
    };
}