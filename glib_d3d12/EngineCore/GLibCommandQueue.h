#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibCommandQueue
    {
    private:
        GLibCommandQueue() {}
        ~GLibCommandQueue() {}

        static GLibCommandQueue* m_Instance;
    public:
        static GLibCommandQueue& GetInstance()
        {
            if (!m_Instance)
            {
                m_Instance = new GLibCommandQueue();
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
        }

        bool Initialize(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC& desc);

        ID3D12CommandQueue* Get() const { return m_CommandQueue.Get(); }

    private:
        ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

        HRESULT m_Hr = {};
    };
}