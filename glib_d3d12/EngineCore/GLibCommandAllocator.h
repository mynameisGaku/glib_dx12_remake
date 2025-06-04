#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibCommandAllocator
    {
    public:
        GLibCommandAllocator() {}
        ~GLibCommandAllocator();

        bool Initialize(ID3D12Device* device, const D3D12_COMMAND_LIST_TYPE& type);

        ID3D12CommandAllocator* Get() const { return m_CommandAllocator.Get(); }
    private:
        ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;

        HRESULT m_Hr = {};
    };
}