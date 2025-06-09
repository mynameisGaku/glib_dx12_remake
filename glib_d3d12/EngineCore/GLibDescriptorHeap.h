#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <string>
#include <Windows.h>

#include <GLibComPtr.h>

namespace glib
{
    class GLibDevice;
    class GLibDescriptorPool;

    class GLibDescriptorHeap
    {
    public:
        GLibDescriptorHeap();
        ~GLibDescriptorHeap();

        bool Initialize(GLibDevice* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        bool Initialize(GLibDescriptorPool* pPool, ID3D12DescriptorHeap* descriptorHeap);

        bool Initialize(GLibDevice* device, GLibDescriptorPool* pPool, ID3D12DescriptorHeap* descriptorHeap);

        ID3D12DescriptorHeap* Get() const;

        UINT GetIncrementSize() const;
        UINT GetIncrementSizeIndex(UINT idx) const;

        void Release();

        UINT GetIndex() const;
        void AddIndex() { addIdx(); }

    private:

        void addIdx()
        {
            m_Index++;
        }

        GLibDevice*                  m_pDevice = nullptr;
        GLibDescriptorPool*          m_pDescriptorPool = nullptr; // ディスクリプタプールへのポインタ
        ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap = nullptr;
        D3D12_DESCRIPTOR_HEAP_DESC   m_Desc{};
        UINT m_Index;

        HRESULT m_Hr = {};
    };
}