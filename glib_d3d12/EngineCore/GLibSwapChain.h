#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <Windows.h>
#include <cmath>

#include <GLibComPtr.h>
#include <GLibDescriptorHeap.h>
#include <GLibDescriptorPool.h>
#include <GLibGraphicsCommandList.h>
#include <GLibCommandQueue.h>
#include <GLibCommandAllocator.h>
#include <GLibDevice.h>

namespace glib
{
    class GLibSwapChain
    {
    public:
        GLibSwapChain() = default;
        ~GLibSwapChain();

        bool Initialize(GLibDevice* device, GLibCommandQueue* queue, GLibCommandAllocator* allocator, GLibDescriptorPool* pool, UINT buffIdx);

        IDXGISwapChain4* Get() const { return m_SwapChain.Get(); }
        GLibDescriptorHeap& GetBbvHeap() { return m_BbvHeap; }
        UINT GetBbvHeapSize() const { return m_BbvHeapSize; }
        UINT GetBackBufferIndex() const { return m_BackBufIdx; }
        ID3D12Resource* GetBackBuffer(UINT idx) const
        {
            if (idx < m_BackBuffers.size())
            {
                return m_BackBuffers[idx].Get();
            }
            return nullptr;
        }

        void DrawBegin(glib::GLibGraphicsCommandList* cmdList);
        void DrawEnd(glib::GLibGraphicsCommandList* cmdList);

    private:
        std::vector<ComPtr<ID3D12Resource>>     m_BackBuffers{};
        UINT                                    m_BackBufIdx{};
        ComPtr<IDXGISwapChain4>                 m_SwapChain             = nullptr;
        GLibDevice*                             m_pDevice               = nullptr;
        GLibCommandQueue*                       m_pCommandQueue         = nullptr;
        GLibCommandAllocator*                   m_pCommandAllocator     = nullptr;
        GLibDescriptorPool*                     m_pDescriptorPool       = nullptr;
        GLibDescriptorHeap                      m_BbvHeap{};
        UINT                                    m_BbvHeapSize{};
        HRESULT                                 m_Hr{};
    };
}