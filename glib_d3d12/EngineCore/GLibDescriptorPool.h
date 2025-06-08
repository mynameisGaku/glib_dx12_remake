#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <string>
#include <Windows.h>

#include <GLib.h>
#include <GLibComPtr.h>
#include <GLibDescriptorHeap.h>

namespace glib
{
    class GLibDevice;

    class GLibDescriptorPool
    {
    public:
        GLibDescriptorPool() = default;
        ~GLibDescriptorPool();

        bool Initialize(GLibDevice* device);

        GLibDescriptorHeap* Get(const GLIB_DESCRIPTOR_HEAP_TYPE& type) const;
        ID3D12DescriptorHeap* Allocate(const GLIB_DESCRIPTOR_HEAP_TYPE& type, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        void Free(const GLIB_DESCRIPTOR_HEAP_TYPE& type);
        void Free(ID3D12DescriptorHeap* descriptorHeap);

        void AllFree();

    private:
        GLibDevice* m_pDevice = nullptr;
        std::unordered_map<GLIB_DESCRIPTOR_HEAP_TYPE, GLibDescriptorHeap*> m_DescriptorHeaps;
        HRESULT m_Hr = {};
        int m_DescriptorSize = 0;

    };
}