#pragma once
#include <dxgi1_6.h>
#include <cmath>
#include <GLibComPtr.h>
#include <GLibDescriptorHeap.h>
#include <d3d12.h>
#include <vector>
#include <Windows.h>
#include <GLibDescriptorPool.h>

namespace glib
{
    class GLibSwapChain
    {
    private:
        GLibSwapChain() = default;
        ~GLibSwapChain()
        {
            m_BackBuffers.clear();
            glib::GLibDescriptorPool::GetInstance().Free("SwapChainRTVHeap");
            glib::Logger::DebugLog("BackBuffers cleared successfully.");
            glib::Logger::DebugLog("SwapChain released successfully.");
            glib::Logger::DebugLog("SwapChain RTV Heap freed successfully.");
        }
        static GLibSwapChain* m_Instance;
    public:

        static GLibSwapChain& GetInstance()
        {
            if (!m_Instance)
            {
                m_Instance = new GLibSwapChain();
            }
            return *m_Instance;
        }

        static void Release()
        {
            if (m_Instance)
            {
                delete m_Instance;
                m_Instance = nullptr;
            }
            glib::Logger::DebugLog("GLibSwapChain released successfully.");
        }

        bool Initialize(ID3D12Device* device, UINT buffIdx);

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

        void DrawBegin();
        void DrawEnd();

    private:
        ComPtr<IDXGISwapChain4> m_SwapChain;
        std::vector<ComPtr<ID3D12Resource>> m_BackBuffers;
        UINT m_BackBufIdx{};

        GLibDescriptorHeap m_BbvHeap;
        UINT m_BbvHeapSize{};

        HRESULT m_Hr{};
    };
}