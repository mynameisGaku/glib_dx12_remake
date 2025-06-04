#pragma once
#include <d3d12.h>
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

        ID3D12DescriptorHeap* Get(const std::string& name) const;
        ID3D12DescriptorHeap* Allocate(const std::string& name, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        void Free(const std::string& name);
        void Free(ID3D12DescriptorHeap* descriptorHeap);

        void AllFree();

    private:
        ID3D12Device* m_pDevice = nullptr;
        std::unordered_map<std::string, ComPtr<ID3D12DescriptorHeap>> m_DescriptorHeaps;
        HRESULT m_Hr = {};
        int m_DescriptorSize = 0;

    };
}