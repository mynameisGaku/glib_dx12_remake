#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibGraphicsCommandList
    {
    public:
        GLibGraphicsCommandList() {}
        ~GLibGraphicsCommandList() {}

        void Initialize(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator, const D3D12_COMMAND_LIST_TYPE& type);

        ID3D12GraphicsCommandList* Get() const { return m_CommandList.Get(); }
    private:
        ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;

        HRESULT m_Hr = {};
    };
}