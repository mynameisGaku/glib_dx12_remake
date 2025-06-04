#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibCommandQueue.h>
#include <GLibDevice.h>

namespace glib
{
    class GLibFence
    {
    public:
        GLibFence() {}
        ~GLibFence();

        bool Initialize(GLibDevice* device, GLibCommandQueue* queue, const D3D12_FENCE_FLAGS& flags);

        ID3D12Fence* Get() const { return m_Fence.Get(); }

        void WaitDrawDone();

        void Close();
    private:
        ComPtr<ID3D12Fence> m_Fence = nullptr;
        HANDLE              m_FenceEvent = nullptr;
        UINT64              m_FenceValue = 0;
        HRESULT             m_Hr = {};
        GLibDevice*         m_pDevice = nullptr;
        GLibCommandQueue*   m_pCommandQueue = nullptr;
    };
}