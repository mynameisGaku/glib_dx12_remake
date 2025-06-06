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
        bool Initialize(GLibDevice* device, GLibDescriptorPool* pPool, const D3D12_RESOURCE_DESC& desc);

    private:
        void* m_pMappedConstBuf;
        ComPtr<ID3D12Resource> m_ConstBuf;
        ID3D12DescriptorHeap* m_pCbvHeap;
        HRESULT m_Hr;
    };
}