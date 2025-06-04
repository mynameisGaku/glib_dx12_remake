#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>
#include <GLibDescriptorPool.h>

namespace glib
{
    class GLibConstantBuffer
    {
    public:

        GLibConstantBuffer() = default;
        ~GLibConstantBuffer();
        bool Initialize(GLibDevice* device, const D3D12_RESOURCE_DESC& desc);

        ID3D12Resource* Get() const { return m_ConstantBuffer.Get(); }

    private:
        GLibDevice*             m_pDevice = nullptr; // デバイスへのポインタ
        GLibDescriptorPool*     m_pDescriptorPool = nullptr; // ディスクリプタプールへのポインタ
        ID3D12DescriptorHeap*   m_pConstantHeap = nullptr;
        ComPtr<ID3D12Resource>  m_ConstantBuffer = nullptr;
        void*                   m_pConstBufferData = nullptr; // マップされた定数バッファのポインタ
        HRESULT                 m_Hr = {};
    };
}