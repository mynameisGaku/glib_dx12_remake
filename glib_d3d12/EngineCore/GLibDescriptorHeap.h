#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDescriptorPool.h>

namespace glib
{
    class GLibDescriptorHeap
    {
    public:
        GLibDescriptorHeap();
        ~GLibDescriptorHeap();

        bool Initialize(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        bool Initialize(GLibDescriptorPool* pPool, ID3D12DescriptorHeap* descriptorHeap)
        {
            if (!descriptorHeap) return false;
            m_DescriptorHeap = descriptorHeap;
            m_pDescriptorPool = pPool;
            m_Hr = S_OK; // 成功とみなす
            return true;
        }

        ID3D12DescriptorHeap* Get() const { return m_DescriptorHeap.Get(); }

        void Release()
        {
            m_pDescriptorPool->Free(m_DescriptorHeap.Get());
            m_DescriptorHeap.Reset();
        }

    private:
        GLibDescriptorPool*          m_pDescriptorPool = nullptr; // ディスクリプタプールへのポインタ
        ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap = nullptr;

        HRESULT m_Hr = {};
    };
}