#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibCommandQueue
    {
    public:
        GLibCommandQueue() {}
        ~GLibCommandQueue();

        bool Initialize(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC& desc);

        ID3D12CommandQueue* Get() const { return m_CommandQueue.Get(); }

    private:
        ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

        HRESULT m_Hr = {};
    };
}