#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <string>
#include <Windows.h>

#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>
#include <GLibCommandAllocator.h>

namespace glib
{
    class GLibGraphicsCommandList
    {
    public:
        GLibGraphicsCommandList() {}
        ~GLibGraphicsCommandList();

        void Initialize(GLibDevice* device, GLibCommandAllocator* commandAllocator, const D3D12_COMMAND_LIST_TYPE& type);

        ID3D12GraphicsCommandList* Get() const { return m_CommandList.Get(); }
    private:
        ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;

        HRESULT m_Hr = {};
    };
}