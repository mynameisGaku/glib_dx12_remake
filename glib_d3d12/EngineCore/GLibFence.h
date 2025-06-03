#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibFence
    {
    private:
        GLibFence() {}
        ~GLibFence() {}

        static GLibFence* m_Instance;
    public:
        static GLibFence& GetInstance() 
        {
            if (!m_Instance)
            {
                m_Instance = new GLibFence();
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

            glib::Logger::DebugLog("GLibFence released successfully.");
        }

        bool Initialize(ID3D12Device* device, const D3D12_FENCE_FLAGS& flags);

        ID3D12Fence* Get() const { return m_Fence.Get(); }

        void WaitDrawDone();

        void Close();
    private:
        ComPtr<ID3D12Fence> m_Fence = nullptr;
        HANDLE m_FenceEvent = nullptr;
        UINT64 m_FenceValue = 0;

        HRESULT m_Hr = {};
    };
}