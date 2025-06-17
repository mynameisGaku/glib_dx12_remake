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
                glib::Logger::FormatErrorLog("�o�C�i���t�@�C�����J���܂���ł���: %s", filename);
                return;
            }
            glib::Logger::FormatInfoLog("�o�C�i���t�@�C����ǂݍ���ł��܂�: %s", filename);
            m_Succeeded = true;
            std::istreambuf_iterator<char> first(ifs);
            std::istreambuf_iterator<char> last;
            m_Buffer.assign(first, last);
            ifs.close();
            if (m_Buffer.empty())
            {
                glib::Logger::FormatErrorLog("�o�C�i���t�@�C������ł���: %s", filename);
                m_Succeeded = false;
            }
            else
            {
                glib::Logger::FormatInfoLog("�o�C�i���t�@�C���̓ǂݍ��݊����i�T�C�Y: %zu �o�C�g�j", m_Buffer.size());
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