#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibCommandAllocator
    {
    private:
        GLibCommandAllocator() {}
        ~GLibCommandAllocator() {}

        static GLibCommandAllocator* m_Instance;
    public:
        static GLibCommandAllocator& GetInstance()
        {
            if (!m_Instance)
            {
                m_Instance = new GLibCommandAllocator();
            }
            return *m_Instance;
        }

        static void Release()
        {
            if (m_Instance)
            {
                delete m_Instance;
            }
            m_Instance = nullptr;

            glib::Logger::DebugLog("GLibCommandAllocator released successfully.");
        }

        bool Initialize(ID3D12Device* device, const D3D12_COMMAND_LIST_TYPE& type);

        ID3D12CommandAllocator* Get() const { return m_CommandAllocator.Get(); }
    private:
        ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;

        HRESULT m_Hr = {};
    };
}