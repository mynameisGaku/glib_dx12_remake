#pragma once
#include <d3d12.h>
#include <GLib.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <unordered_map>
#include <string>

namespace glib
{
    class GLibDescriptorPool
    {
    public:
        GLibDescriptorPool() = default;
        ~GLibDescriptorPool();

        bool Initialize(ID3D12Device* device);

        ID3D12DescriptorHeap* Get(const GLIB_DESCRIPTOR_HEAP_TYPE& type) const;
        ID3D12DescriptorHeap* Allocate(const GLIB_DESCRIPTOR_HEAP_TYPE& type, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        void Free(const GLIB_DESCRIPTOR_HEAP_TYPE& type);
        void Free(ID3D12DescriptorHeap* descriptorHeap);

        void AllFree();

    private:
        ID3D12Device* m_pDevice = nullptr;
        std::unordered_map<GLIB_DESCRIPTOR_HEAP_TYPE, ComPtr<ID3D12DescriptorHeap>> m_DescriptorHeaps;
        HRESULT m_Hr = {};
        int m_DescriptorSize = 0;

    };
}