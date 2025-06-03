#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibDescriptorHeap
    {
    public:
        bool Initialize(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        bool Initialize(ID3D12DescriptorHeap* descriptorHeap)
        {
            if (!descriptorHeap) return false;
            m_DescriptorHeap = descriptorHeap;
            m_Hr = S_OK; // ê¨å˜Ç∆Ç›Ç»Ç∑
            return true;
        }

        ID3D12DescriptorHeap* Get() const { return m_DescriptorHeap.Get(); }

        void Release()
        {
            m_DescriptorHeap = nullptr;
        }

    private:
        ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap = nullptr;

        HRESULT m_Hr = {};
    };
}