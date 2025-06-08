#pragma once
#include <d3d12.h>
#include <Windows.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>

namespace glib
{
    class GLibCommandQueue
    {
    public:
        GLibCommandQueue() {}
        ~GLibCommandQueue();

        bool Initialize(GLibDevice* device, const D3D12_COMMAND_QUEUE_DESC& desc);

        ID3D12CommandQueue* Get() const { return m_CommandQueue.Get(); }

    private:
        ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

        HRESULT m_Hr = {};
    };
}