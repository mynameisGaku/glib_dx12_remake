#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibConstantBuffer
    {
    public:

        GLibConstantBuffer() = default;
        ~GLibConstantBuffer() = default;
        bool Initialize(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc);

        ID3D12Resource* Get() const { return m_ConstantBuffer.Get(); }

    private:
        ID3D12DescriptorHeap* m_pConstantHeap = nullptr;
        ComPtr<ID3D12Resource> m_ConstantBuffer = nullptr;
        void* m_pConstBufferData = nullptr; // マップされた定数バッファのポインタ
        HRESULT m_Hr = {};
    };
}